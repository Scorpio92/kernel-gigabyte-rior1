/* Copyright (c) 2009-2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/delay.h>
#include <linux/module.h>
#include <linux/leds.h>
#include <mach/gpio.h>
#include <mach/pmic.h>
#include "msm_fb.h"

#define CONFIG_FB_MSM_LCDC_AUTO_DETECT

static int prev_bl = 32;

static int spi_cs;
static int spi_sclk;
static int spi_mosi;
static int gpio_backlight_en;
static int gpio_display_reset;

struct lcdc_state_type{
	boolean initialized;
};

static struct lcdc_state_type lcdc_state = { 0 };
static struct msm_panel_common_pdata *lcdc_pdata;

static char init_item_0xB9[] = {0xFF, 0x83, 0x69};
static char init_item_0xB3[] = {0x07};
//static char init_item_0xB1[] = {0x01, 0x00, 0x34, 0x07, 0x00, 0x0E, 0x0E, 0x21, 0x26/*0x29*/, 0x3F, 0x3F, 0x01, 0x23, 0x01, 0xE6, 0xE6, 0xE6, 0xE6, 0xE6};
static char init_item_0xB1[] = {0x01, 0x00, 0x34, 0x07, 0x00, 0x0E, 0x0E, 0x21, 0x29, 0x3F, 0x3F, 0x01, 0x23, 0x01, 0xE6, 0xE6, 0xE6, 0xE6, 0xE6};
static char init_item_0xB2[] = {0x00, 0x23, 0x07, 0x07, 0x70, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x01};
static char init_item_0xB4[] = {0x0A, 0x0C, 0x84, 0x0C, 0x01};
static char init_item_0xB6[] = {0x1F, 0x1F};
static char init_item_0xCC[] = {0x00};
static char init_item_0xD5[] = {0x00, 0x03, 0x00, 0x00, 0x01, 0x07, 0x10, 0x80, 0x33, 0x37, 0x23, 0x01, 0xB9, 0x75, 0xA8, 0x64, 0x00, 0x00, 0x41, 0x06, 0x50, 0x07, 0x07, 0x0F, 0x07, 0x00};
//static char init_item_0xE0[] = {0x00, 0x03, 0x00, 0x09, 0x09, 0x21, 0x1B, 0x2D, 0x06, 0x0C, 0x10, 0x15, 0x16, 0x14, 0x16, 0x12, 0x18, 0x00, 0x03, 0x00, 0x09, 0x09, 0x21, 0x1B, 0x2D, 0x06, 0x0C, 0x10, 0x15, 0x16, 0x14, 0x16, 0x12, 0x18};
static char init_item_0xE0[] = {0x00, 0x03, 0x00, 0x08, 0x08, 0x21, 0x15, 0x2b, 0x06, 0x0C, 0x10, 0x13, 0x16, 0x14, 0x14, 0x11, 0x17, 0x00, 0x03, 0x00, 0x08, 0x08, 0x21, 0x15, 0x2b, 0x06, 0x0C, 0x10, 0x13, 0x16, 0x14, 0x14, 0x11, 0x17};
static char init_item_0x3A[] = {0x77};

static char init_item_0x53[] = {0x24};
static char init_item_0x55[] = {0x00};

static __inline__ void lcdc_spi_write_byte(char dc, uint8 data)
{
	uint32 bit;
	int bnum;

	gpio_set_value_cansleep(spi_sclk, 0); /* clk low */
	/* dc: 0 for command, 1 for parameter */
	gpio_set_value_cansleep(spi_mosi, dc);
	udelay(1);	/* at least 20 ns */
	gpio_set_value_cansleep(spi_sclk, 1); /* clk high */
	udelay(1);	/* at least 20 ns */
	bnum = 8;	/* 8 data bits */
	bit = 0x80;
	while (bnum) {
		gpio_set_value_cansleep(spi_sclk, 0); /* clk low */
		if (data & bit)
			gpio_set_value_cansleep(spi_mosi, 1);
		else
			gpio_set_value_cansleep(spi_mosi, 0);
		udelay(1);
		gpio_set_value_cansleep(spi_sclk, 1); /* clk high */
		udelay(1);
		bit >>= 1;
		bnum--;
	}
}

static __inline__ int lcdc_spi_write(char cmd, char *data, int num)
{
	int i;

	gpio_set_value_cansleep(spi_cs, 0);	/* cs low */
	/* command byte first */
	lcdc_spi_write_byte(0, cmd);
	/* followed by parameter bytes */
	for(i = 0; i < num; i++)
	{
		if(data)
			lcdc_spi_write_byte(1, data[i]);
	}
	gpio_set_value_cansleep(spi_mosi, 1);	/* mosi high */
	gpio_set_value_cansleep(spi_cs, 1);	/* cs high */
	udelay(10);
	return 0;
}

static void spi_pin_assign(void)
{
	/* Setting the Default GPIO's */
	spi_mosi	= *(lcdc_pdata->gpio_num);
	spi_sclk	= *(lcdc_pdata->gpio_num + 1);
	spi_cs		= *(lcdc_pdata->gpio_num + 2);
	gpio_backlight_en = *(lcdc_pdata->gpio_num + 3);
	gpio_display_reset = *(lcdc_pdata->gpio_num + 4);
}

#if 0
static void lcdc_disp_powerup(void)
{
	if (!lcdc_state.disp_powered_up && !lcdc_state.display_on) {
		/* Reset the hardware first */
		/* Include DAC power up implementation here */
	      lcdc_state.disp_powered_up = TRUE;
	}
}
#endif 

static void lcdc_truly_disp_reginit(void)
{
		gpio_set_value_cansleep(spi_cs, 1);	/* cs high */
		//mdelay(10);
		gpio_set_value_cansleep(gpio_display_reset, 1);
		msleep(10);
		gpio_set_value_cansleep(gpio_display_reset, 0);
		msleep(15);
		gpio_set_value_cansleep(gpio_display_reset, 1);
		msleep(100);

		lcdc_spi_write(0xB9, init_item_0xB9, sizeof(init_item_0xB9));
		lcdc_spi_write(0xB1, init_item_0xB1, sizeof(init_item_0xB1));
		lcdc_spi_write(0xB2, init_item_0xB2, sizeof(init_item_0xB2));
		lcdc_spi_write(0xB3, init_item_0xB3, sizeof(init_item_0xB3));
		lcdc_spi_write(0xB4, init_item_0xB4, sizeof(init_item_0xB4));
		lcdc_spi_write(0xB6, init_item_0xB6, sizeof(init_item_0xB6));
		lcdc_spi_write(0xCC, init_item_0xCC, sizeof(init_item_0xCC));
                lcdc_spi_write(0xD5, init_item_0xD5, sizeof(init_item_0xD5));
		lcdc_spi_write(0xE0, init_item_0xE0, sizeof(init_item_0xE0));
		lcdc_spi_write(0x3A, init_item_0x3A, sizeof(init_item_0x3A));
		lcdc_spi_write(0x11, NULL, 0);//sleep out
		msleep(150);
		lcdc_spi_write(0x29, NULL, 0);//display on
		msleep(150);
		//lcdc_spi_write(0x2c, NULL, 0);//display on
}

static int lcdc_truly_panel_on(struct platform_device *pdev)
{
//luke: add for continue
	struct msm_fb_data_type *mfd = platform_get_drvdata(pdev);
	if((lcdc_pdata->cont_splash_enabled)&&(!mfd->cont_splash_done)){
		printk("luke: %s cont_splash_enabled! %d \n",__func__,__LINE__);
        	mfd->cont_splash_done = 1;
        	//return 0;
	}
//luke: add end
	if (lcdc_pdata->panel_config_gpio)
		lcdc_pdata->panel_config_gpio(1);
		printk("luke: %s: %d\n",__func__,__LINE__);
	if (! lcdc_state.initialized){
		lcdc_truly_disp_reginit();
		lcdc_state.initialized = TRUE;
	}
	return 0;
}

static int lcdc_truly_panel_off(struct platform_device *pdev)
{
	if ( lcdc_state.initialized) {
		printk("luke: %s: %d\n",__func__,__LINE__);
		/* Main panel power off (Pull down reset) */
		lcdc_spi_write(0x28, NULL, 0);//display off
		lcdc_spi_write(0x10, NULL, 0);//enter sleep
		msleep(50);
		lcdc_state.initialized = FALSE;
	}
	return 0;
}

#define LCD_DRIVER_BL 1
//static int lcd_backlight = 0;
static void lcdc_truly_set_backlight(struct msm_fb_data_type *mfd)
{

if (lcdc_state.initialized) {
    if(!LCD_DRIVER_BL)
    {
	int step = 0, i = 0;
	int bl_level = mfd->bl_level;

	/* real backlight level, 1 - max, 16 - min, 32 - off */
	bl_level = 32 - bl_level;

	if (bl_level > prev_bl) {
		step = bl_level - prev_bl;
		if (bl_level == 32) {
			step--;
		}
	} else if (bl_level < prev_bl) {
		step = bl_level + 31 - prev_bl;
	} else {
		return;
	}
	if (bl_level == 32) {
		/* turn off backlight */
		gpio_set_value_cansleep(gpio_backlight_en, 0);
	} else {
		if (prev_bl == 32) {
			/* turn on backlight */
			gpio_set_value_cansleep(gpio_backlight_en, 1);
			udelay(30);
		}

		/* adjust backlight level */
		for (i = 0; i < step; i++) {
			gpio_set_value_cansleep(gpio_backlight_en, 0);
			udelay(1);
			gpio_set_value_cansleep(gpio_backlight_en, 1);
			udelay(1);
		}
	}
	mdelay(1);
	prev_bl = bl_level;
    }
    else {
	int bl_level = mfd->bl_level;
       // printk("luke: %s -----------------------------  bl_level=%d \n",__func__, bl_level);
	gpio_set_value_cansleep(spi_cs, 0);
	//if(!lcd_backlight)
	//msleep(80);

	lcdc_spi_write_byte(0, 0x51);
	lcdc_spi_write_byte(1, bl_level);
		
	gpio_set_value_cansleep(spi_mosi, 1);
	gpio_set_value_cansleep(spi_cs, 1);

	lcdc_spi_write(0x53, init_item_0x53, sizeof(init_item_0x53));
	lcdc_spi_write(0x55, init_item_0x55, sizeof(init_item_0x55));

	if(0 == bl_level){
 printk("luke: %s -----------------------------  bl_level=%d \n",__func__, bl_level);
		//Start===Allen
		keyboard_led_light(KEY_LED_USER_LCD, 0);
		//lcd_backlight = 0;
		//End===Allen
	}else {
		//Start===Allen
		keyboard_led_light(KEY_LED_USER_LCD, 1);
		//End===Allen
		//lcd_backlight = 1;
	}  
    }
}
    return;
}

static int __devinit lcdc_truly_probe(struct platform_device *pdev)
{

printk("luke: %s: %d\n",__func__,__LINE__);
	if (pdev->id == 0) {

		lcdc_pdata = pdev->dev.platform_data;
		spi_pin_assign();
		return 0;
	}
	msm_fb_add_device(pdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = lcdc_truly_probe,
	.driver = {
		.name   = "lcdc_truly_wvga_hx8369a_pt",
	},	          
};

static struct msm_fb_panel_data lcdc_panel_data = {

	.on = lcdc_truly_panel_on,
	.off = lcdc_truly_panel_off,
	.set_backlight = lcdc_truly_set_backlight,
};

static struct platform_device this_device = {
	.name   = "lcdc_truly_wvga_hx8369a_pt",
	.id	= 1,
	.dev	= {
		.platform_data = &lcdc_panel_data,
	}
};




static int __init lcdc_truly_panel_init(void)
{
	int ret;
	struct msm_panel_info *pinfo;

#ifdef CONFIG_FB_MSM_LCDC_AUTO_DETECT
	if (msm_fb_detect_client("lcdc_truly_wvga_hx8369a_pt")) {
		return 0;
	}
#endif	

	ret = platform_driver_register(&this_driver);
	if (ret)
		return ret;

	pinfo = &lcdc_panel_data.panel_info;
	pinfo->xres = 480;
	pinfo->yres = 800;
	MSM_FB_SINGLE_MODE_PANEL(pinfo);
	pinfo->type = LCDC_PANEL;
	pinfo->pdest = DISPLAY_1;
	pinfo->wait_cycle = 0;
	pinfo->bpp = 24;
	pinfo->fb_num = 2;
	/* 10Mhz mdp_lcdc_pclk and mdp_lcdc_pad_pcl */
	pinfo->clk_rate = 24500000;
	pinfo->bl_max = 255;
	pinfo->bl_min = 1;
#if 0
	pinfo->lcdc.h_back_porch = 10;		/* hsw = 8 + hbp=16 */
	pinfo->lcdc.h_front_porch = 10;
	pinfo->lcdc.h_pulse_width = 10;
	pinfo->lcdc.v_back_porch = 6;		/* vsw=1 + vbp = 7 */
	pinfo->lcdc.v_front_porch = 6;
	pinfo->lcdc.v_pulse_width = 4;
#endif
	pinfo->lcdc.h_back_porch = 30;		
	pinfo->lcdc.h_front_porch = 16;
	pinfo->lcdc.h_pulse_width = 7;
	pinfo->lcdc.v_back_porch = 6;		
	pinfo->lcdc.v_front_porch = 6;
	pinfo->lcdc.v_pulse_width = 4;

	pinfo->lcdc.border_clr = 0;			/* blk */
	pinfo->lcdc.underflow_clr = 0xff;	/* blue */
	pinfo->lcdc.hsync_skew = 0;
	ret = platform_device_register(&this_device);
printk("luke: %s: lcdc_panel device registered!\n",__func__);
	if (ret) {
		printk(KERN_ERR "%s not able to register the device\n",
			 __func__);
		goto fail_driver;
	}
	
	return 0;

fail_driver:
	platform_driver_unregister(&this_driver);
      
	return ret;
}

device_initcall(lcdc_truly_panel_init);


