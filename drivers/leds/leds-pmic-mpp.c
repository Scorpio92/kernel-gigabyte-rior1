/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <mach/pmic.h>
//start===hanxiaohui
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
//end===hanxiaohui
#define LED_MPP(x)		((x) & 0xFF)
#define LED_CURR(x)		((x) >> 16)

struct pmic_mpp_led_data {
	struct led_classdev cdev;
	int which;
	int type;
	int max;
};
//start===Allen
struct delayed_work     dwork;
atomic_t keyboard_led_on;
static int keyboard_led_state = 0;
static int need_work = 1;
#if 0  //masked by hanxiaohui
static void keyboard_work_handler(struct work_struct *work)
{
        printk("%s\n",__func__);
        pmic_secure_mpp_config_i_sink(PM_MPP_7, PM_MPP__I_SINK__LEVEL_5mA,PM_MPP__I_SINK__SWITCH_DIS);
        keyboard_led_state = 0;
}
#endif
void keyboard_led_light(int user, unsigned int on)
{
       return ;
	if(!!on){
		if((atomic_read(&keyboard_led_on) == 0)||(user ==KEY_LED_USER_BUTTON)||(user == KEY_LED_USER_CIT))
		{
			if(user == KEY_LED_USER_LCD){
				atomic_set(&keyboard_led_on, 1);
				need_work = 1;
			}

			if(need_work){
				cancel_delayed_work(&dwork);
				schedule_delayed_work(&dwork, msecs_to_jiffies(3000));
			}
			printk("%s: on\n", __func__);

			if(keyboard_led_state == 0){
				pmic_secure_mpp_config_i_sink(PM_MPP_7, PM_MPP__I_SINK__LEVEL_5mA,
								PM_MPP__I_SINK__SWITCH_ENA_IF_MPP_LOW);
				keyboard_led_state=1;
			}
		}
	}else{
		if(need_work)
			cancel_delayed_work(&dwork);
		printk("%s: off\n", __func__);

		if(keyboard_led_state == 1){
			pmic_secure_mpp_config_i_sink(PM_MPP_7,PM_MPP__I_SINK__LEVEL_5mA, 
							PM_MPP__I_SINK__SWITCH_DIS);
			keyboard_led_state = 0;
		}

		if(user == KEY_LED_USER_LCD)
			atomic_set(&keyboard_led_on, 0);
	}
}
EXPORT_SYMBOL_GPL(keyboard_led_light);

void keyboard_led_set_brightness(struct led_classdev *led_cdev, enum led_brightness brightness)
{
        need_work =0;
        keyboard_led_light(KEY_LED_USER_BUTTON, (unsigned int) brightness);
}

//================For keyboard led CIT test ================
static int keyboard_led_read_proc(char *page, char **start, off_t offset,
                                        int count, int *eof, void *data)
{
        *eof = 1;
        printk("%s \n", __func__);
        return sprintf(page, "keyboard_led_state:%u\n", keyboard_led_state);
}

static int keyboard_led_write_proc(struct file *file, const char *buffer,
                                        unsigned long count, void *data)
{
        char *buf;

        printk("%s \n", __func__);
        if (count < 1)
                return -EINVAL;

        buf = kmalloc(count, GFP_KERNEL);
        if (!buf)
                return -ENOMEM;

	if (copy_from_user(buf, buffer, count)) {
                kfree(buf);
                return -EFAULT;
        }

        if (buf[0] == '0') {
                need_work = 1;
                keyboard_led_light(KEY_LED_USER_CIT, 0);
        } else {
                need_work = 0;
                keyboard_led_light(KEY_LED_USER_CIT, 1);
        }

        kfree(buf);
        return count;
}
void keyboard_led_cit_init(void)
{
        struct proc_dir_entry *pe;
        pe = create_proc_entry("kpsetting", 0777, NULL);
        if (0 == pe)
                return;
        pe->write_proc = keyboard_led_write_proc;
        pe->read_proc = keyboard_led_read_proc;
}
//End===Allen

static void pm_mpp_led_set(struct led_classdev *led_cdev,
	enum led_brightness value)
{
	struct pmic_mpp_led_data *led;
	int ret;

	led = container_of(led_cdev, struct pmic_mpp_led_data, cdev);

	if (value < LED_OFF || value > led->cdev.max_brightness) {
		dev_err(led->cdev.dev, "Invalid brightness value");
		return;
	}

	if(value > led->max) {
		value = led->max;
	}

	if(PMIC8029_DRV_TYPE_CUR == led->type) {
		ret = pmic_secure_mpp_config_i_sink(led->which, 1,
				value ? PM_MPP__I_SINK__SWITCH_ENA :
					PM_MPP__I_SINK__SWITCH_DIS);
	} else {
		ret = pmic_secure_mpp_control_digital_output(led->which,
			value,
			value ? PM_MPP__DLOGIC_OUT__CTRL_HIGH : PM_MPP__DLOGIC_OUT__CTRL_LOW);
	}
	if (ret)
		dev_err(led_cdev->dev, "can't set mpp led\n");
}

static int pmic_mpp_led_probe(struct platform_device *pdev)
{
	const struct pmic8029_leds_platform_data *pdata = pdev->dev.platform_data;
	struct pmic_mpp_led_data *led, *tmp_led;
	int i, rc;
	if (!pdata) {
		dev_err(&pdev->dev, "platform data not supplied\n");
		return -EINVAL;
	}
       
	led = kcalloc(pdata->num_leds, sizeof(*led), GFP_KERNEL);
	if (!led) {
		dev_err(&pdev->dev, "failed to alloc memory\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, led);

	for (i = 0; i < pdata->num_leds; i++) {
		tmp_led	= &led[i];
		tmp_led->cdev.name = pdata->leds[i].name;
       		#if 1
		tmp_led->cdev.brightness_set = pm_mpp_led_set;
		#else
		tmp_led->cdev.brightness_set = keyboard_led_set_brightness;
		#endif
		tmp_led->cdev.brightness = LED_OFF;
		tmp_led->cdev.max_brightness = LED_FULL;
		tmp_led->which = pdata->leds[i].which;
		tmp_led->type = pdata->leds[i].type;
		if(PMIC8029_DRV_TYPE_CUR == tmp_led->type) {
			tmp_led->max = pdata->leds[i].max.cur;
		} else {
			tmp_led->max = pdata->leds[i].max.vol;
		}

		if (PMIC8029_DRV_TYPE_CUR == tmp_led->type &&
			(tmp_led->max < PM_MPP__I_SINK__LEVEL_5mA ||
			tmp_led->max > PM_MPP__I_SINK__LEVEL_40mA)) {
			dev_err(&pdev->dev, "invalid current\n");
			goto unreg_led_cdev;
		}

		rc = led_classdev_register(&pdev->dev, &tmp_led->cdev);
		if (rc) {
			dev_err(&pdev->dev, "failed to register led\n");
			goto unreg_led_cdev;
		}
	}

      	keyboard_led_cit_init();// add by Allen
	return 0;

unreg_led_cdev:
	while (i)
		led_classdev_unregister(&led[--i].cdev);

	kfree(led);
	return rc;

}

static int __devexit pmic_mpp_led_remove(struct platform_device *pdev)
{
	const struct pmic8029_leds_platform_data *pdata = pdev->dev.platform_data;
	struct pmic_mpp_led_data *led = platform_get_drvdata(pdev);
	int i;

	for (i = 0; i < pdata->num_leds; i++)
		led_classdev_unregister(&led[i].cdev);

	kfree(led);

	return 0;
}

static struct platform_driver pmic_mpp_led_driver = {
	.probe		= pmic_mpp_led_probe,
	.remove		= __devexit_p(pmic_mpp_led_remove),
	.driver		= {
		.name	= "pmic-mpp-leds",
		.owner	= THIS_MODULE,
	},
};

static int __init pmic_mpp_led_init(void)
{
	return platform_driver_register(&pmic_mpp_led_driver);
}
module_init(pmic_mpp_led_init);

static void __exit pmic_mpp_led_exit(void)
{
	platform_driver_unregister(&pmic_mpp_led_driver);
}
module_exit(pmic_mpp_led_exit);

MODULE_DESCRIPTION("PMIC MPP LEDs driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:pmic-mpp-leds");
