/*  Copyright (C) 2012 Bosch Sensortec GmbH All Rights Reserved
 *  Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *  Date: 2012/11/13 11:00:00
 *  Revision: 2.5
 *
 *
 *
 * This software program is licensed subject to the GNU General Public License
 * (GPL).Version 2,June 1991, available at http://www.fsf.org/copyleft/gpl.html
 *
 * file RPI1040.c
 * brief This file contains all function implementations for the RPI1040 in linux
 *
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <mach/gpio.h>
#include <linux/platform_device.h>

#define GRAVITY_EARTH                   9806550
#define ABSMIN_2G                       (-GRAVITY_EARTH * 2)
#define ABSMAX_2G                       (GRAVITY_EARTH * 2)
#define RPI1040_MAX_DELAY		50


struct rpi1040acc{
	s16	x,
		y,
		z;
} ;

struct rpi1040_data {
	atomic_t delay;
	atomic_t enable;
	unsigned char mode;
	struct input_dev *input;
	struct rpi1040acc value;
	struct mutex value_mutex;
	struct mutex enable_mutex;
	struct mutex mode_mutex;
	struct delayed_work work;
	struct work_struct irq_work;
};

struct rpi1040_data *data;


static void rpi1040_read_accel_xyz(struct rpi1040acc *acc)
{
	acc->x = gpio_get_value(109);
	acc->y = gpio_get_value(28);;
	acc->z = 0;
	if(acc->x == 0 && acc->y == 0)
	{
		acc->x = 0x108;
		acc->y = 0x08;
		acc->z = -0x08;
	}else if(acc->x == 1 && acc->y == 0)
	{
		acc->y = 0x04;
		acc->y = 0xf8;
		acc->z = 0x08;
	}else if(acc->x == 0 && acc->y == 1)
	{
		acc->x = 0x08;
		acc->y = -0x10d;
		acc->z = -0x08;
	}else if(acc->x == 1 && acc->y == 1)
	{
		acc->x = -0xf8;
		acc->y = 0x08;
		acc->z = 0x04;
	}
}

static void rpi1040_work_func(struct work_struct *work)
{
	struct rpi1040_data *rpi1040 = container_of((struct delayed_work *)work,
			struct rpi1040_data, work);
	static struct rpi1040acc acc;
	unsigned long delay = msecs_to_jiffies(atomic_read(&rpi1040->delay));

	rpi1040_read_accel_xyz(&acc);
	

		//printk("###%s,x=0x%x,y=0x%x,z=0x%x\n",__func__,acc.x,acc.y,acc.z);

		input_report_rel(rpi1040->input, REL_RX, acc.x * (4));
		input_report_rel(rpi1040->input, REL_RY, acc.y * (4));	
		input_report_rel(rpi1040->input, REL_RZ, acc.z * (4));

		input_sync(rpi1040->input);

	mutex_lock(&rpi1040->value_mutex);
	rpi1040->value = acc;
	mutex_unlock(&rpi1040->value_mutex);
	schedule_delayed_work(&rpi1040->work, delay);
}


static ssize_t rpi1040_value_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct rpi1040_data *rpi1040 = input_get_drvdata(input);
	struct rpi1040acc acc_value;

	mutex_lock(&rpi1040->value_mutex);
	acc_value = rpi1040->value;
	mutex_unlock(&rpi1040->value_mutex);

	return sprintf(buf, "%d %d %d\n", acc_value.x, acc_value.y,
			acc_value.z);
}


static ssize_t rpi1040_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{

	int i = gpio_get_value(27);
	return sprintf(buf, "%d\n", i);
}

static void rpi1040_set_enable(struct device *dev, int enable)
{
	struct rpi1040_data *rpi1040 = data;
	int pre_enable = atomic_read(&rpi1040->enable);

	mutex_lock(&rpi1040->enable_mutex);
	if (enable) {
		if (pre_enable ==0) {
			gpio_set_value(27, 1);
			schedule_delayed_work(&rpi1040->work,
				msecs_to_jiffies(atomic_read(&rpi1040->delay)));
			atomic_set(&rpi1040->enable, 1);
		}
		
	} else {
		if (pre_enable ==1) {
			gpio_set_value(27, 0);
			cancel_delayed_work_sync(&rpi1040->work);
			atomic_set(&rpi1040->enable, 0);
		} 
	}
	mutex_unlock(&rpi1040->enable_mutex);
	
}

static ssize_t rpi1040_enable_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if (data == 0){
		rpi1040_set_enable(dev,0);
		gpio_set_value(27, 0);
	}
	else if(data == 1){
		rpi1040_set_enable(dev,1);
	}
	return count;
}


static DEVICE_ATTR(value, 0664,
		rpi1040_value_show, NULL);
static DEVICE_ATTR(enable, 0664,
		rpi1040_enable_show, rpi1040_enable_store);


static struct attribute *rpi1040_attributes[] = {
	&dev_attr_value.attr,
	&dev_attr_enable.attr,
	NULL
};

static struct attribute_group rpi1040_attribute_group = {
	.attrs = rpi1040_attributes
};

static int rpi1040_input_init(struct rpi1040_data *rpi1040)
{
	struct input_dev *dev;
	int err;

	dev = input_allocate_device();

	//printk("###rpi1040_input_init\n");
	if (!dev)
		return -ENOMEM;
	dev->name = "acc"; //SENSOR_NAME; changed for sku1 compatible
	dev->id.bustype = BUS_I2C;

	/* X */
	input_set_capability(dev, EV_REL, REL_RX);
	input_set_abs_params(dev, REL_RX, ABSMIN_2G, ABSMAX_2G, 0, 0);
	/* Y */
	input_set_capability(dev, EV_REL, REL_RY);
	input_set_abs_params(dev, REL_RY, ABSMIN_2G, ABSMAX_2G, 0, 0);
	/* Z */
	input_set_capability(dev, EV_REL, REL_RZ);
	input_set_abs_params(dev, REL_RZ, ABSMIN_2G, ABSMAX_2G, 0, 0);
	input_set_drvdata(dev, rpi1040);

	err = input_register_device(dev);
	if (err < 0) {
		input_free_device(dev);
		return err;
	}
	rpi1040->input = dev;

	return 0;
}

static void rpi1040_input_delete(struct rpi1040_data *rpi1040)
{
	struct input_dev *dev = rpi1040->input;

	input_unregister_device(dev);
	input_free_device(dev);
}


static int rpi1040_probe(struct platform_device *pdev)
{
	int err = 0;

	data = kzalloc(sizeof(struct rpi1040_data), GFP_KERNEL);
	//printk("###rpi1040_probe\n");
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}
	mutex_init(&data->value_mutex);
	mutex_init(&data->mode_mutex);
	mutex_init(&data->enable_mutex);

	INIT_DELAYED_WORK(&data->work, rpi1040_work_func);
	atomic_set(&data->delay, RPI1040_MAX_DELAY);
	atomic_set(&data->enable, 0);
	err = rpi1040_input_init(data);
	if (err < 0)
		goto kfree_exit;

	err = sysfs_create_group(&data->input->dev.kobj,
						 &rpi1040_attribute_group);
	if (err < 0)
		goto error_sysfs;
	rpi1040_set_enable(&data->input->dev, 3);

	return 0;

error_sysfs:
	rpi1040_input_delete(data);

kfree_exit:
	kfree(data);
exit:
	return err;
}

static struct platform_driver msm_driver_rpi1040_sensor = {
	.probe  = rpi1040_probe,
	.driver = {
		.name   = "rpi1040", 
	},
};


static int rpi1040_remove(void)
{       
	
	platform_driver_unregister(&msm_driver_rpi1040_sensor);
	rpi1040_set_enable(&data->input->dev, 0);
	sysfs_remove_group(&data->input->dev.kobj, &rpi1040_attribute_group);
	rpi1040_input_delete(data);
	kfree(data);
	return 0;
}


static int __init RPI1040_init(void)
{	
	int ret=0;
	ret = platform_driver_register(&msm_driver_rpi1040_sensor);
	return ret;
}

static void __exit RPI1040_exit(void)
{
	rpi1040_remove();
}

MODULE_AUTHOR("wm <meng.wang@ragentek.com>");
MODULE_DESCRIPTION("RPI1040 driver");
MODULE_LICENSE("GPL");

module_init(RPI1040_init);
module_exit(RPI1040_exit);

