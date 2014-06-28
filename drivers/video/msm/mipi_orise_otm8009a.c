/* Copyright (c) 2008-2011, Code Aurora Forum. All rights reserved.
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
//#define DEBUG

#include "mach/gpio.h"
#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_orise_otm8009a.h"
#include <mach/pmic.h>
#include <linux/leds.h>// add by hanxiaohui
#define GPIO_ORISE_LCD_RESET 85

//*lilonghui add it for the backlightness 2012-3-14*/
#define BACKLIGHT_BRIGHTNESS_LEVEL 32


static int gpio_backlight_en = 0xff;
/*lilonghui add it for the keyboard led 2012-3-13*/
static int gpio_keybaord_en = 0xff;
static struct msm_panel_common_pdata *mipi_orise_pdata;
static struct dsi_buf orise_tx_buf;
static struct dsi_buf orise_rx_buf;
static int prev_bl = BACKLIGHT_BRIGHTNESS_LEVEL + 1 ;
static int lcd_state = 0;

#define ORISE_CMD_DELAY 0
#define ORISE_SLEEP_DELAY 150
#define ORISE_DISPLAY_DELAY 120

static char exit_sleep[2] = {0x11, 0x00};
static char display_on[2] = {0x29, 0x00};
static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};
static char write_ram[2] = {0x2C, 0x00};


static char ORISE_CUSTOMER_0XFF1[] = {0xFF,0x80,0x09,0x01};                                 
static char ORISE_CUSTOMER_0X001[] = {0x00,0x80};
static char ORISE_CUSTOMER_0XFF2[] = {0xFF,0x80,0x09};

static char ORISE_CUSTOMER_0X360[] = {0x35,0x01};         //0xc0

static char ORISE_CUSTOMER_0X002[] = {0x00,0x03};
static char ORISE_CUSTOMER_0XFF3[] = {0xFF,0x01};

static char ORISE_CUSTOMER_0X003[] = {0x00,0xB4};
//static char ORISE_CUSTOMER_0XC00[] = {0xC0,0x10};
static char ORISE_CUSTOMER_0XC00[] = {0xC0,0x50};     //0x10, 0x50 colomn 

static char ORISE_CUSTOMER_0X004[] = {0x00,0x89};
static char ORISE_CUSTOMER_0XC40[] = {0xC4,0x08};

static char ORISE_CUSTOMER_0X005[] = {0x00,0xA3};
static char ORISE_CUSTOMER_0XC01[] = {0xc0,0x00};

static char ORISE_CUSTOMER_0X006[] = {0X00,0x82};
static char ORISE_CUSTOMER_0XC50[] = {0xC5,0xA3};

static char ORISE_CUSTOMER_0X007[] = {0x00,0x90};
//static char ORISE_CUSTOMER_0XC51[] = {0xC5,0xD6,0X87};
static char ORISE_CUSTOMER_0XC51[] = {0xC5,0xA6,0X87};  // 0xd6 , 0xA6  2*VDD 

static char ORISE_CUSTOMER_0X008[] = {0X00,0x00};
static char ORISE_CUSTOMER_0XD80[] = {0xD8,0x75,0X73};

static char ORISE_CUSTOMER_0X009[] = {0x00,0x00};
static char ORISE_CUSTOMER_0XD90[] = {0xD9,0X4E};

static char ORISE_CUSTOMER_0X0010[] = {0x00,0x00};
static char ORISE_CUSTOMER_0XE10[] = {0xE1,0x09,0x0B,0x11,0x0F,0x09,0x1A,0x0B,0x0B,0x01,0x05,0x03,0x08,0x0D,0x26,0x23,0x18};

static char ORISE_CUSTOMER_0X0011[] = {0x00,0x00};
static char ORISE_CUSTOMER_0XE20[] = {0xE2,0x09,0x0B,0x11,0x0F,0x09,0x1A,0x0B,0x0B,0x01,0x05,0x03,0x08,0x0D,0x26,0x23,0x18};

static char ORISE_CUSTOMER_0X0012[] = {0x00,0x81};
static char ORISE_CUSTOMER_0XC10[] = {0xC1,0X66};

static char ORISE_CUSTOMER_0X0013[] = {0x00,0xA1};
static char ORISE_CUSTOMER_0XC11[] = {0xC1,0X08};

static char ORISE_CUSTOMER_0X0014[] = {0x00,0x89};
static char ORISE_CUSTOMER_0XC41[] = {0xC4,0X08};

static char ORISE_CUSTOMER_0X0015[] = {0x00,0x81};
static char ORISE_CUSTOMER_0XC42[] = {0xC4,0X83};

static char ORISE_CUSTOMER_0X0016[] = {0x00,0x92};
static char ORISE_CUSTOMER_0XC52[] = {0xC5,0X01};

static char ORISE_CUSTOMER_0X0017[] = {0x00,0xB1};
static char ORISE_CUSTOMER_0XC53[] = {0xC5,0XA9};

static char ORISE_CUSTOMER_0X0018[] = {0x00,0x80};
static char ORISE_CUSTOMER_0XCE0[] = {0xCE,0x85,0x03,0x00,0x84,0x03,0x00,0x83,0x03,0x00,0x82,0x03,0x00};

static char ORISE_CUSTOMER_0X0019[] = {0x00,0xA0};
static char ORISE_CUSTOMER_0XCE1[] = {0xCE,0x38,0x02,0x03,0x21,0x00,0x00,0x00,0x38,0x01,0x03,0x22,0x00,0x00,0x00};

static char ORISE_CUSTOMER_0X0020[] = {0x00,0xB0};
static char ORISE_CUSTOMER_0XCE2[] = {0xCE,0x38,0x00,0x03,0x23,0x00,0x00,0x00,0x30,0x00,0x03,0x24,0x00,0x00,0x00};

static char ORISE_CUSTOMER_0X0021[] = {0x00,0xC0};
static char ORISE_CUSTOMER_0XCE3[] = {0xCE,0x30,0x01,0x03,0x25,0x00,0x00,0x00,0x30,0x02,0x03,0x26,0x00,0x00,0x00};

static char ORISE_CUSTOMER_0X0022[] = {0x00,0xD0};
static char ORISE_CUSTOMER_0XCE4[] = {0xCE,0x30,0x03,0x03,0x27,0x00,0x00,0x00,0x30,0x04,0x03,0x28,0x00,0x00,0x00};

static char ORISE_CUSTOMER_0X0023[] = {0x00,0xC0};
static char ORISE_CUSTOMER_0XCF0[] =  {0xCF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static char ORISE_CUSTOMER_0X0024[] = {0x00,0xD0};
static char ORISE_CUSTOMER_0XCF1[] = {0xCF,0X00};

static char ORISE_CUSTOMER_0X0025[] = {0x00,0xC0};
static char ORISE_CUSTOMER_0XCB0[] = {0xCB,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00,0x00,0x00};

static char ORISE_CUSTOMER_0X0026[] = {0x00,0xD0};
static char ORISE_CUSTOMER_0XCB1[] =  {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x04};
//
static char ORISE_CUSTOMER_0X0026X[] = {0x00,0xE0};
static char ORISE_CUSTOMER_0XCB3[] =  {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
//
static char ORISE_CUSTOMER_0X0027[] = {0x00,0x80};
static char ORISE_CUSTOMER_0XCC0[] =  {0xCC,0x00,0x00,0x00,0x00,0x0C,0x0A,0x10,0x0E,0x03,0x04};

static char ORISE_CUSTOMER_0X0028[] = {0x00,0x90};
static char ORISE_CUSTOMER_0XCC1[] =  {0xCC,0x00,0x00,0x00,0x00,0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0B};

static char ORISE_CUSTOMER_0X0029[] = {0x00,0xA0};
static char ORISE_CUSTOMER_0XCC2[] =  {0xCC,0x09,0x0F,0x0D,0x01,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static char ORISE_CUSTOMER_0X0030[] = {0x00,0xB0};
static char ORISE_CUSTOMER_0XCC3[] =  {0xCC,0x00,0x00,0x00,0x00,0x0D,0x0F,0x09,0x0B,0x02,0x01};

static char ORISE_CUSTOMER_0X0031[] = {0x00,0xC0};
static char ORISE_CUSTOMER_0XCC4[] =  {0xCC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0E};

static char ORISE_CUSTOMER_0X0032[] = {0x00,0xD0};
static char ORISE_CUSTOMER_0XCC5[] =  {0xCC,0x10,0x0A,0x0C,0x04,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x55};

static char ORISE_CUSTOMER_0X0033[] = {0x00,0x00};
static char ORISE_CUSTOMER_0XCC6[] =  {0x3A,0X77};

static struct dsi_cmd_desc orise_cmd_display_on_cmds[] = {                                    

	{DTYPE_GEN_LWRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XFF1), ORISE_CUSTOMER_0XFF1},
	{DTYPE_GEN_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X001), ORISE_CUSTOMER_0X001},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XFF2), ORISE_CUSTOMER_0XFF2},

        {DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X360), ORISE_CUSTOMER_0X360},

	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X002), ORISE_CUSTOMER_0X002},
	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XFF3), ORISE_CUSTOMER_0XFF3},

	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X003), ORISE_CUSTOMER_0X003},
	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XC00), ORISE_CUSTOMER_0XC00},

	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X004), ORISE_CUSTOMER_0X004},
	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XC40), ORISE_CUSTOMER_0XC40},

	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X005), ORISE_CUSTOMER_0X005},
	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XC01), ORISE_CUSTOMER_0XC01},

	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X006), ORISE_CUSTOMER_0X006},
	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XC50), ORISE_CUSTOMER_0XC50},

	{DTYPE_GEN_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X007), ORISE_CUSTOMER_0X007},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XC51), ORISE_CUSTOMER_0XC51},

	{DTYPE_GEN_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X008), ORISE_CUSTOMER_0X008},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XD80), ORISE_CUSTOMER_0XD80},

	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X009), ORISE_CUSTOMER_0X009},
	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XD90), ORISE_CUSTOMER_0XD90},

	{DTYPE_GEN_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0010), ORISE_CUSTOMER_0X0010},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XE10), ORISE_CUSTOMER_0XE10},

	{DTYPE_GEN_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0011), ORISE_CUSTOMER_0X0011},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XE20), ORISE_CUSTOMER_0XE20},

	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0012), ORISE_CUSTOMER_0X0012},
	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XC10), ORISE_CUSTOMER_0XC10},

	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0013), ORISE_CUSTOMER_0X0013},
	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XC11), ORISE_CUSTOMER_0XC11},

	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0014), ORISE_CUSTOMER_0X0014},
	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XC41), ORISE_CUSTOMER_0XC41},

	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0015), ORISE_CUSTOMER_0X0015},
	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XC42), ORISE_CUSTOMER_0XC42},

	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0016), ORISE_CUSTOMER_0X0016},
	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XC52), ORISE_CUSTOMER_0XC52},

	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0017), ORISE_CUSTOMER_0X0017},
	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XC53), ORISE_CUSTOMER_0XC53},

	{DTYPE_GEN_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0018), ORISE_CUSTOMER_0X0018},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XCE0), ORISE_CUSTOMER_0XCE0},

	{DTYPE_GEN_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0019), ORISE_CUSTOMER_0X0019},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XCE1), ORISE_CUSTOMER_0XCE1},

	{DTYPE_GEN_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0020), ORISE_CUSTOMER_0X0020},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XCE2), ORISE_CUSTOMER_0XCE2},

	{DTYPE_GEN_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0021), ORISE_CUSTOMER_0X0021},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XCE3), ORISE_CUSTOMER_0XCE3},

	{DTYPE_GEN_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0022), ORISE_CUSTOMER_0X0022},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XCE4), ORISE_CUSTOMER_0XCE4},

	{DTYPE_GEN_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0023), ORISE_CUSTOMER_0X0023},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XCF0), ORISE_CUSTOMER_0XCF0},

	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0024), ORISE_CUSTOMER_0X0024},
	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XCF1), ORISE_CUSTOMER_0XCF1},

	{DTYPE_GEN_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0025), ORISE_CUSTOMER_0X0025},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XCB0), ORISE_CUSTOMER_0XCB0},

        {DTYPE_GEN_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0026), ORISE_CUSTOMER_0X0026},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XCB1), ORISE_CUSTOMER_0XCB1},
//     
        {DTYPE_GEN_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0026X), ORISE_CUSTOMER_0X0026X},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XCB3), ORISE_CUSTOMER_0XCB3},
//   
	{DTYPE_GEN_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0027), ORISE_CUSTOMER_0X0027},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XCC0), ORISE_CUSTOMER_0XCC0},

	{DTYPE_GEN_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0028), ORISE_CUSTOMER_0X0028},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XCC1), ORISE_CUSTOMER_0XCC1},

	{DTYPE_GEN_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0029), ORISE_CUSTOMER_0X0029},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XCC2), ORISE_CUSTOMER_0XCC2},

	{DTYPE_GEN_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0030), ORISE_CUSTOMER_0X0030},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XCC3), ORISE_CUSTOMER_0XCC3},

	{DTYPE_GEN_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0031), ORISE_CUSTOMER_0X0031},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XCC4), ORISE_CUSTOMER_0XCC4},

	{DTYPE_GEN_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0032), ORISE_CUSTOMER_0X0032},
	{DTYPE_GEN_LWRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XCC5), ORISE_CUSTOMER_0XCC5},

	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X0033), ORISE_CUSTOMER_0X0033},
	{DTYPE_DCS_WRITE, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0XCC6), ORISE_CUSTOMER_0XCC6},
        {DTYPE_DCS_WRITE, 1, 0, 0, 150, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(display_on), display_on},
        {DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(write_ram), write_ram},

};


static struct dsi_cmd_desc orise_display_off_cmds[] = {
        {DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 150, sizeof(enter_sleep), enter_sleep},	
};


static char ORISE_CUSTOMER_0X51[] = {0x51,0xFF};  //write brightness
static char ORISE_CUSTOMER_0X53[] = {0x53,0x24};  //write ctrl display
static char ORISE_CUSTOMER_0X55[] = {0x55,0x00};  //write content adaptive brightness ctrl

static struct dsi_cmd_desc orise_cmd_drive_backlight_cmds[] = {
	{DTYPE_GEN_WRITE1, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X51), ORISE_CUSTOMER_0X51},
	{DTYPE_GEN_WRITE1, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X53), ORISE_CUSTOMER_0X53},
	{DTYPE_GEN_WRITE1, 1, 0, 0, ORISE_CMD_DELAY, sizeof(ORISE_CUSTOMER_0X55), ORISE_CUSTOMER_0X55},
};

void lcd_backlight_on(struct msm_fb_data_type *mfd)
{
	mipi_dsi_cmds_tx(mfd, &orise_tx_buf, orise_cmd_drive_backlight_cmds,
			ARRAY_SIZE(orise_cmd_drive_backlight_cmds));
}

static int mipi_orise_lcd_reset(void)
{
    int rc = 0;

    gpio_set_value_cansleep(GPIO_ORISE_LCD_RESET, 1);
    mdelay(5);
    gpio_set_value_cansleep(GPIO_ORISE_LCD_RESET, 0);
    msleep(10);
    gpio_set_value_cansleep(GPIO_ORISE_LCD_RESET, 1);
    msleep(120);

    return rc;
}

static int mipi_orise_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mipi_panel_info *mipi;


	mfd = platform_get_drvdata(pdev);
	if (!mfd) {
		return -ENODEV;
	}

	if (mfd->key != MFD_KEY) {
		return -EINVAL;
	}

	mipi  = &mfd->panel_info.mipi;
	if (mipi_orise_lcd_reset() < 0) {
		pr_err("mipi_orise_lcd_reset error\n");
		return -EINVAL;
	}
	if (mipi->mode == DSI_CMD_MODE) {
		mipi_dsi_cmds_tx(mfd, &orise_tx_buf, orise_cmd_display_on_cmds,
			ARRAY_SIZE(orise_cmd_display_on_cmds));  
               // mdelay(10);//add by wang tao
	}
        lcd_state = 1;
printk("luke:  %s  %d ----------------------------------end \n",__func__, __LINE__);
	return 0;
}

static int mipi_orise_lcd_off(struct platform_device *pdev)
{


	struct msm_fb_data_type *mfd;


	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	mipi_dsi_cmds_tx(mfd, &orise_tx_buf, orise_display_off_cmds,
			ARRAY_SIZE(orise_display_off_cmds));
printk("luke:  %s  %d ----------------------------------end\n",__func__, __LINE__);
	gpio_set_value_cansleep(GPIO_ORISE_LCD_RESET, 0);

	return 0;
}

 

static int __devinit mipi_orise_lcd_probe(struct platform_device *pdev)
{
	int rc = 0;
	if (pdev->id == 0) {
		mipi_orise_pdata = pdev->dev.platform_data;
	    if (mipi_orise_pdata) {
			gpio_backlight_en = mipi_orise_pdata->gpio;

	    }

		return rc;
	}
printk("luke : %s    %d\n",__func__, __LINE__);
	rc = gpio_request(GPIO_ORISE_LCD_RESET, "gpio_disp_reset");
	if(rc < 0) {
		pr_err("Error request gpio %d\n", GPIO_ORISE_LCD_RESET);
	}

	rc = gpio_tlmm_config(GPIO_CFG(GPIO_ORISE_LCD_RESET, 0,
				GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	
	if (rc < 0) {
		pr_err("Error config lcdc gpio:%d\n", GPIO_ORISE_LCD_RESET);
	}

	rc = gpio_direction_output(GPIO_ORISE_LCD_RESET, 0);
	if (rc < 0) {
		pr_err("Error direct lcdc gpio:%d\n", GPIO_ORISE_LCD_RESET);
	}	
	msm_fb_add_device(pdev);
	return rc;
}

static struct platform_driver this_driver = {
	.probe  = mipi_orise_lcd_probe,
	.driver = {
		.name   = "mipi_orise_otm8009a_wvga_pt",
	},
};


static void set_backlight_by_driver(struct msm_fb_data_type *mfd)
{
	int bl_level = mfd->bl_level;
	ORISE_CUSTOMER_0X51[1] = bl_level;

        mipi_dsi_mdp_busy_wait(mfd);	
	mipi_dsi_cmds_tx(mfd, &orise_tx_buf, orise_cmd_drive_backlight_cmds,
		    ARRAY_SIZE(orise_cmd_drive_backlight_cmds));
	
	//add by medea to support keyboard light [2012-5-4].
	if(0 == bl_level) {
		//Start===Allen
		keyboard_led_light(KEY_LED_USER_LCD, 0);
		//End===Allen
	}else {
		//Start===Allen
		keyboard_led_light(KEY_LED_USER_LCD, 1);
		//End===Allen
	}	
	//end add
}


static void set_backlight_by_host(struct msm_fb_data_type *mfd)
{
	int step = 0, i = 0;
    	unsigned long flags;
	int bl_level = mfd->bl_level;
	/*lilonghui modify it for the lcd baklight 2012-3-14*/
	/* real backlight level, 32 - max, 1 - min, 0 - off */
	bl_level = BACKLIGHT_BRIGHTNESS_LEVEL + 1 - bl_level;

	if (bl_level > prev_bl) {
		step = bl_level - prev_bl;
          
		if (bl_level == BACKLIGHT_BRIGHTNESS_LEVEL + 1) {
			step--;
		}
	} else if (bl_level < prev_bl) {
		step = bl_level + BACKLIGHT_BRIGHTNESS_LEVEL - prev_bl;
	} else {
		return;
	}
	if (bl_level == (BACKLIGHT_BRIGHTNESS_LEVEL + 1)) {
		/* turn off backlight */
		pr_debug("%s: turn off backlight\n", __func__);
		gpio_set_value_cansleep(gpio_backlight_en, 0);
		/*lilonghui add it for the keybaord led 2012-3-13*/
			mdelay(4);
		gpio_set_value_cansleep(gpio_keybaord_en, 0);
			udelay(30);
	} else {
        local_irq_save(flags);	
         
		if (prev_bl == (BACKLIGHT_BRIGHTNESS_LEVEL + 1)) {
			/* turn on backlight */
			pr_debug("%s: turn on backlight\n", __func__);
			gpio_set_value_cansleep(gpio_backlight_en, 1);
			udelay(400);
			/*lilonghui add it for the keybaord led 2012-3-13*/
		        gpio_set_value_cansleep(gpio_keybaord_en, 1);
		   	udelay(30);
		}
		
         pr_debug("%s:lilonghui step 2= %d\n",__func__,step); 
		/* adjust backlight level */
		for (i = 0; i < step; i++) {
			gpio_set_value(gpio_backlight_en, 0);
			udelay(1);
			gpio_set_value(gpio_backlight_en, 1);
			udelay(1);
		}

		local_irq_restore(flags);
                   
	}
	prev_bl = bl_level;
      /*end */
	return;
}

#ifdef DRVIE_BACKLIGHT_BY_HOST
	static int driver_by_host = 1;
#else
	static int driver_by_host = 0;
#endif

static void mipi_orise_set_backlight(struct msm_fb_data_type *mfd)
{
	if(driver_by_host) {
		set_backlight_by_host(mfd);
	}
	else {
		set_backlight_by_driver(mfd);
	}
}

static struct msm_fb_panel_data orise_panel_data = {
	.on	= mipi_orise_lcd_on,
	.off = mipi_orise_lcd_off,
	.set_backlight = mipi_orise_set_backlight,
};

static int ch_used[3];

int mipi_orise_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

	pr_debug("mipi_orise_device_register\n");

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_orise_otm8009a_wvga_pt", (panel << 8)|channel);

	if (!pdev)
		return -ENOMEM;

	orise_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &orise_panel_data,
		sizeof(orise_panel_data));

	if (ret) {
		pr_debug("%s: platform_device_add_data failed!\n", __func__);
		goto err_device_put;
	}

	ret = platform_device_add(pdev);
                   
	if (ret) {
		pr_debug("%s: platform_device_register failed!\n", __func__);
		goto err_device_put;
	}

	return 0;

err_device_put:
	platform_device_put(pdev);
	return ret;
}

static int __init mipi_orise_lcd_init(void)
{
	pr_debug("mipi_orise_lcd_ini\n");
	
	mipi_dsi_buf_alloc(&orise_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&orise_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

module_init(mipi_orise_lcd_init);
