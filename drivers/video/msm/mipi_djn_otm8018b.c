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
 *
 */
#include "mach/gpio.h"
#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_djn_otm8018b.h"
#include <linux/leds.h>  // add by hanxiaohui


//#define DRVIE_BACKLIGHT_BY_HOST
#define GPIO_TCL_LCD_RESET 85
/* common setting */
#define DJN_CMD_DELAY 0
#define DJN_SLEEP_OFF_DELAY 150
#define DJN_DISPLAY_ON_DELAY 0
#define BACKLIGHT_BRIGHTNESS_LEVEL 32

static struct msm_panel_common_pdata *mipi_djn8018b_pdata;
static struct dsi_buf djn8018b_tx_buf;
static struct dsi_buf djn8018b_rx_buf;

static int prev_bl = BACKLIGHT_BRIGHTNESS_LEVEL + 1 ;
static int gpio_backlight_en = 96;
//static int gpio_keybaord_en = 0xff;

static void set_backlight_by_host(struct msm_fb_data_type *mfd);

/* common setting */
//static char extend_cmd_enable[4] = {0xB9, 0xFF, 0x83, 0x69};
static char exit_sleep[2] = {0x11, 0x00};
static char display_on[2] = {0x29, 0x00};
#if 0
static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};
#endif
static char write_memory_start[2] = {0x2C, 0x00};

//set bl by lcd device
static char bl_0x51[2] = {0x51, 0xFF};
static char bl_0x53[2] = {0x53, 0x24};
static char bl_0x55[2] = {0x55, 0x00};

/***********************************************/

static char DJN_OTM8018B_0XFF1[] = {0xFF,0x80,0x09,0x01};                                 
static char DJN_OTM8018B_0X001[] = {0x00,0x80};
static char DJN_OTM8018B_0XFF2[] = {0xFF,0x80,0x09};

static char DJN_OTM8018B_0X002[] = {0x00,0x03};
static char DJN_OTM8018B_0XFF3[] = {0xFF,0x01};

static char DJN_OTM8018B_0X003[] = {0x00,0xB4};
static char DJN_OTM8018B_0XC00[] = {0xC0,0x10};     //0x10, 0x50 colomn 

static char DJN_OTM8018B_0X004[] = {0x00,0x89};
static char DJN_OTM8018B_0XC40[] = {0xC4,0x08};

static char DJN_OTM8018B_0X005[] = {0x00,0xA3};
static char DJN_OTM8018B_0XC01[] = {0xc0,0x00};

static char DJN_OTM8018B_0X006[] = {0X00,0x82};
static char DJN_OTM8018B_0XC50[] = {0xC5,0xA3};

static char DJN_OTM8018B_0X007[] = {0x00,0x90};
static char DJN_OTM8018B_0XC51[] = {0xC5,0x96,0x87};  // 0xd6 , 0xA6  2*VDD 

static char DJN_OTM8018B_0X008[] = {0X00,0x00};
static char DJN_OTM8018B_0XD80[] = {0xD8,0x72,0x72};

static char DJN_OTM8018B_0X009[] = {0x00,0x00};
static char DJN_OTM8018B_0XD90[] = {0xD9,0x61};

static char DJN_OTM8018B_0X0010[] = {0x00,0x00};
static char DJN_OTM8018B_0XE10[] = {0xE1,0x09,0x0a,0x0e,0x0d,0x07,0x18,0x0d,0x0d,0x01,0x04,0x05,0x06,0x0e,0x25,0x22,0x05};

static char DJN_OTM8018B_0X0011[] = {0x00,0x00};
static char DJN_OTM8018B_0XE20[] = {0xE2,0x09,0x0a,0x0e,0x0d,0x07,0x18,0x0d,0x0d,0x01,0x04,0x05,0x06,0x0e,0x25,0x22,0x05};

static char DJN_OTM8018B_0X0012[] = {0x00,0x81};
static char DJN_OTM8018B_0XC10[] = {0xC1,0x55};  //0x66

static char DJN_OTM8018B_0X0013[] = {0x00,0xA0};
static char DJN_OTM8018B_0XC11[] = {0xC1,0xea};

static char DJN_OTM8018B_0X00A2[] = {0x00,0xA2};
static char DJN_OTM8018B_0XC0[] = {0xC0,0x04,0x00,0x02};

static char DJN_OTM8018B_0X0015[] = {0x00,0x80};
static char DJN_OTM8018B_0XC42[] = {0xC4,0x30};

static char DJN_OTM8018B_0X0016[] = {0x00,0xc0};
static char DJN_OTM8018B_0XC52[] = {0xC5,0x00};

static char DJN_OTM8018B_0X000B_0[] = {0x00,0x8B};
static char DJN_OTM8018B_0XB040[] = {0xB0,0x40};

static char DJN_OTM8018B_0X000B_1[] = {0x00,0xb2};
static char DJN_OTM8018B_0XF515[] = {0xF5,0x15,0x00,0x15,0x00};

static char DJN_OTM8018B_0X0081[] = {0x00,0x81};
static char DJN_OTM8018B_0XC481[] = {0xC4,0x83};

static char DJN_OTM8018B_0X0092[] = {0x00,0x92};
static char DJN_OTM8018B_0XC501[] = {0xC5,0x01,0x03};

static char DJN_OTM8018B_0X0017[] = {0x00,0xB1};
static char DJN_OTM8018B_0XC53[] = {0xC5,0xA9};

static char DJN_OTM8018B_0X0090[] = {0x00,0x90};
static char DJN_OTM8018B_0XC000[] = {0xC0,0x00,0x44,0x00,0x00,0x00,0x03};

static char DJN_OTM8018B_0X00a6[] = {0x00,0xa6};
static char DJN_OTM8018B_0XC100[] = {0xC1,0x01,0x00,0x00};
#if 1
static char DJN_OTM8018B_0X0018[] = {0x00,0x80};
static char DJN_OTM8018B_0XCE0[] = {0xCE,0x87,0x03,0x00,0x85,0x03,0x00,0x86,0x03,0x00,0x84,0x03,0x00};

static char DJN_OTM8018B_0X0019[] = {0x00,0xA0};
static char DJN_OTM8018B_0XCE1[] = {0xCE,0x38,0x03,0x03,0x20,0x00,0x00,0x00,0x38,0x02,0x03,0x21,0x00,0x00,0x00};

static char DJN_OTM8018B_0X0020[] = {0x00,0xB0};
static char DJN_OTM8018B_0XCE2[] = {0xCE,0x38,0x01,0x03,0x22,0x00,0x00,0x00,0x38,0x00,0x03,0x23,0x00,0x00,0x00};

static char DJN_OTM8018B_0X0021[] = {0x00,0xC0};
static char DJN_OTM8018B_0XCE3[] = {0xCE,0x30,0x00,0x03,0x24,0x00,0x00,0x00,0x30,0x01,0x03,0x25,0x00,0x00,0x00};

static char DJN_OTM8018B_0X0022[] = {0x00,0xD0};
static char DJN_OTM8018B_0XCE4[] = {0xCE,0x30,0x02,0x03,0x26,0x00,0x00,0x00,0x30,0x03,0x03,0x27,0x00,0x00,0x00};
#endif

#if 0
//GB modify start
static char DJN_OTM8018B_0X0018[] = {0x00,0x80};
static char DJN_OTM8018B_0XCE0[] = {0xCE,0x86,0x03,0x00,0x84,0x03,0x00,0x85,0x03,0x00,0x83,0x03,0x00};

static char DJN_OTM8018B_0X0019[] = {0x00,0xA0};
static char DJN_OTM8018B_0XCE1[] = {0xCE,0x38,0x02,0x03,0x21,0x00,0x00,0x00,0x38,0x01,0x03,0x22,0x00,0x00,0x00};

static char DJN_OTM8018B_0X0020[] = {0x00,0xB0};
static char DJN_OTM8018B_0XCE2[] = {0xCE,0x38,0x00,0x03,0x23,0x00,0x00,0x00,0x30,0x00,0x03,0x24,0x00,0x00,0x00};

static char DJN_OTM8018B_0X0021[] = {0x00,0xC0};
static char DJN_OTM8018B_0XCE3[] = {0xCE,0x30,0x01,0x03,0x25,0x00,0x00,0x00,0x30,0x02,0x03,0x26,0x00,0x00,0x00};

static char DJN_OTM8018B_0X0022[] = {0x00,0xD0};
static char DJN_OTM8018B_0XCE4[] = {0xCE,0x30,0x03,0x03,0x27,0x00,0x00,0x00,0x30,0x04,0x03,0x27,0x00,0x00,0x00};

//GB modify End
#endif
static char DJN_OTM8018B_0X00c7[] = {0x00,0xc7};
static char DJN_OTM8018B_0XCFc7[] = {0xCF,0x00};

static char DJN_OTM8018B_0X00c9[] = {0x00,0xc9};
static char DJN_OTM8018B_0XCFc9[] = {0xCF,0X00};

static char DJN_OTM8018B_0X0024[] = {0x00,0xD0};
static char DJN_OTM8018B_0XCF1[] = {0xCF,0x00};

static char DJN_OTM8018B_0X0025[] = {0x00,0xC0};
static char DJN_OTM8018B_0XCB0[] = {0xCB,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00,0x00,0x00};

static char DJN_OTM8018B_0X0026[] = {0x00,0xD0};
static char DJN_OTM8018B_0XCB1[] =  {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x04};

static char DJN_OTM8018B_0X0027[] = {0x00,0x80};
static char DJN_OTM8018B_0XCC0[] =  {0xCC,0x00,0x00,0x00,0x00,0x0C,0x0A,0x10,0x0E,0x03,0x04};

static char DJN_OTM8018B_0X0028[] = {0x00,0x90};
static char DJN_OTM8018B_0XCC1[] =  {0xCC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0B};

static char DJN_OTM8018B_0X0029[] = {0x00,0xA0};
static char DJN_OTM8018B_0XCC2[] =  {0xCC,0x09,0x0F,0x0D,0x01,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static char DJN_OTM8018B_0X0030[] = {0x00,0xB0};
static char DJN_OTM8018B_0XCC3[] =  {0xCC,0x00,0x00,0x00,0x00,0x0D,0x0F,0x09,0x0B,0x02,0x01};

static char DJN_OTM8018B_0X0031[] = {0x00,0xC0};
static char DJN_OTM8018B_0XCC4[] =  {0xCC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0E};

static char DJN_OTM8018B_0X0032[] = {0x00,0xD0};
static char DJN_OTM8018B_0XCC5[] =  {0xCC,0x10,0x0A,0x0C,0x04,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static char DJN_OTM8018B_0XCC6[] =  {0x3A,0X77};



static struct dsi_cmd_desc djn8018b_video_display_on_cmds[] = {

	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XFF1), DJN_OTM8018B_0XFF1},
	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X001), DJN_OTM8018B_0X001},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XFF2), DJN_OTM8018B_0XFF2},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X002), DJN_OTM8018B_0X002},
	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XFF3), DJN_OTM8018B_0XFF3},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X003), DJN_OTM8018B_0X003},
	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XC00), DJN_OTM8018B_0XC00},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X004), DJN_OTM8018B_0X004},
	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XC40), DJN_OTM8018B_0XC40},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X005), DJN_OTM8018B_0X005},
	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XC01), DJN_OTM8018B_0XC01},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X006), DJN_OTM8018B_0X006},
	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XC50), DJN_OTM8018B_0XC50},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X007), DJN_OTM8018B_0X007},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XC51), DJN_OTM8018B_0XC51},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X008), DJN_OTM8018B_0X008},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XD80), DJN_OTM8018B_0XD80},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X009), DJN_OTM8018B_0X009},
	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XD90), DJN_OTM8018B_0XD90},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0010), DJN_OTM8018B_0X0010},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XE10), DJN_OTM8018B_0XE10},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0011), DJN_OTM8018B_0X0011},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XE20), DJN_OTM8018B_0XE20},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0012), DJN_OTM8018B_0X0012},
	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XC10), DJN_OTM8018B_0XC10},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0013), DJN_OTM8018B_0X0013},
	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XC11), DJN_OTM8018B_0XC11},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X00A2), DJN_OTM8018B_0X00A2},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XC0), DJN_OTM8018B_0XC0},// 1

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0015), DJN_OTM8018B_0X0015},
	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XC42), DJN_OTM8018B_0XC42},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0016), DJN_OTM8018B_0X0016},
	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XC52), DJN_OTM8018B_0XC52},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X000B_0), DJN_OTM8018B_0X000B_0},
	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XB040), DJN_OTM8018B_0XB040},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X000B_1), DJN_OTM8018B_0X000B_1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XF515), DJN_OTM8018B_0XF515},//3

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0081), DJN_OTM8018B_0X0081},
	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XC481), DJN_OTM8018B_0XC481},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0092), DJN_OTM8018B_0X0092},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XC501), DJN_OTM8018B_0XC501},//4

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0017), DJN_OTM8018B_0X0017},
	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XC53), DJN_OTM8018B_0XC53},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0090), DJN_OTM8018B_0X0090},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XC000), DJN_OTM8018B_0XC000},//5

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X00a6), DJN_OTM8018B_0X00a6},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XC100), DJN_OTM8018B_0XC100},//6

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0018), DJN_OTM8018B_0X0018},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XCE0), DJN_OTM8018B_0XCE0},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0019), DJN_OTM8018B_0X0019},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XCE1), DJN_OTM8018B_0XCE1},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0020), DJN_OTM8018B_0X0020},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XCE2), DJN_OTM8018B_0XCE2},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0021), DJN_OTM8018B_0X0021},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XCE3), DJN_OTM8018B_0XCE3},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0022), DJN_OTM8018B_0X0022},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XCE4), DJN_OTM8018B_0XCE4},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X00c7), DJN_OTM8018B_0X00c7},
	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XCFc7), DJN_OTM8018B_0XCFc7},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X00c9), DJN_OTM8018B_0X00c9},
	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XCFc9), DJN_OTM8018B_0XCFc9},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0024), DJN_OTM8018B_0X0024},
	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XCF1), DJN_OTM8018B_0XCF1},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0025), DJN_OTM8018B_0X0025},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XCB0), DJN_OTM8018B_0XCB0},

        {DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0026), DJN_OTM8018B_0X0026},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XCB1), DJN_OTM8018B_0XCB1},
 
	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0027), DJN_OTM8018B_0X0027},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XCC0), DJN_OTM8018B_0XCC0},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0028), DJN_OTM8018B_0X0028},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XCC1), DJN_OTM8018B_0XCC1},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0029), DJN_OTM8018B_0X0029},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XCC2), DJN_OTM8018B_0XCC2},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0030), DJN_OTM8018B_0X0030},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XCC3), DJN_OTM8018B_0XCC3},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0031), DJN_OTM8018B_0X0031},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XCC4), DJN_OTM8018B_0XCC4},

	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0X0032), DJN_OTM8018B_0X0032},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XCC5), DJN_OTM8018B_0XCC5},
	{DTYPE_GEN_WRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(DJN_OTM8018B_0XCC6), DJN_OTM8018B_0XCC6},

	

	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_SLEEP_OFF_DELAY, sizeof(exit_sleep), exit_sleep},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(display_on), display_on},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(write_memory_start), write_memory_start},

};

static struct dsi_cmd_desc tcl_cmd_drive_backlight_cmds[] = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(bl_0x51), bl_0x51},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(bl_0x53), bl_0x53},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(bl_0x55), bl_0x55},
}; 
#if 0
static struct dsi_cmd_desc djn8018b_display_off_cmds[] = {
	{DTYPE_GEN_WRITE, 1, 0, 0, 100, sizeof(enter_sleep), enter_sleep},
	{DTYPE_GEN_WRITE, 1, 0, 0, 0, sizeof(display_off), display_off},
};
#endif
static struct dsi_cmd_desc djn8018b_cmd_display_on_cmds[] = {

	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_SLEEP_OFF_DELAY, sizeof(exit_sleep), exit_sleep},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(display_on), display_on},
	{DTYPE_GEN_LWRITE, 1, 0, 0, DJN_CMD_DELAY, sizeof(write_memory_start), write_memory_start},
};

/************************************************************************************************/
static char  ic_status_0x0a[] = {0x0a};
static char  ic_status_0x3a[] = {0x3a};

static struct dsi_cmd_desc get_djn8018b_0x0a_value_cmd[] = {{DTYPE_GEN_READ, 1, 0, 0, 0, sizeof(ic_status_0x0a), ic_status_0x0a}};
static struct dsi_cmd_desc get_djn8018b_0x3a_value_cmd[] = {{DTYPE_GEN_READ, 1, 0, 0, 0, sizeof(ic_status_0x3a), ic_status_0x3a}};
static int djn8018b_read_panel_reg(struct msm_fb_data_type *mfd, char reg) 
{ 
        struct dsi_buf *rp, *tp; 
        struct dsi_cmd_desc *cmd; 
        unsigned char  *lp; 

        tp = &djn8018b_tx_buf; 
        rp = &djn8018b_tx_buf; 
        mipi_dsi_buf_init(tp); 
        mipi_dsi_buf_init(rp); 

        switch(reg) 
        { 
                case 0x0a: 
                        cmd = get_djn8018b_0x0a_value_cmd; 
                break; 

		case 0x3a:
			cmd =  get_djn8018b_0x3a_value_cmd;

		break;
                default: 
			cmd = get_djn8018b_0x0a_value_cmd;
                       // cmd = djn8018b_manufacture_id;//default read manufacture id 
                break; 
        } 
	printk("switch(reg)\n");
        mipi_dsi_cmds_rx(mfd, tp, rp, cmd, 1); 
	printk(" mipi_dsi_cmds_rx(mfd, tp, rp, cmd, 1); \n");
        lp = (unsigned char  *)rp->data; 
        return *lp; 
} 

unsigned int djn8018b_monitor_reg_status(struct msm_fb_data_type *mfd) 
{ 
        uint32 data=0; 
        uint16 cnt = 0,ret=0; 
        uint8 err=0;     //ok 
      
       // if(mipi_dsi_wait_stop_state()) return 1; 
        printk("djn8018b_monitor_reg_status\n");
        mipi_set_tx_power_mode(1);                      //entry low power mode         
	printk("mipi_set_tx_power_mode(1)\n");       
	 MIPI_OUTP(MIPI_DSI_BASE + 0x094, 0x01);        //sw bta trigger 
	printk("MIPI_OUTP(MIPI_DSI_BASE + 0x094, 0x01); \n");
        wmb(); 
	printk("wmb(); \n");
        cnt = 0; 
        while (cnt++ < 10000) { 
                data = MIPI_INP(MIPI_DSI_BASE + 0x0004);/* DSI1_STATUS */ 
                if ((data & 0x0010) == 0){              //out of bta busy 
                        break; 
                } 
        } 
	printk(" while (cnt++ < 10000)  \n");
        mipi_dsi_ack_err_status(); 
	printk("mipi_dsi_ack_err_status();   \n");
        if(cnt >= 10000){err = 2; goto exit_stop_state; } 
        printk("cnt = %d\n", cnt);
        ret = djn8018b_read_panel_reg(mfd,0x0a); 
	printk("djn8018b_read_panel_reg  \n");
        printk("hxh:  the returned value of 0x0a is  0x%x\n",ret);
	 ret = djn8018b_read_panel_reg(mfd,0x3a); 
	 printk("hxh:  the returned value of 0x3a is  0x%x\n",ret);
exit_stop_state: 
	printk("exit_stop_state\n");
        mipi_set_tx_power_mode(0); //entry high speed mode
	printk("mipi_set_tx_power_mode(0\n");
        //if(mipi_dsi_wait_stop_state()) {err = 7;} 
         return 1; 
}

/************************************************************************************************/
static int mipi_tcl_lcd_reset(void)
{
    int rc = 0;
    gpio_set_value_cansleep(GPIO_TCL_LCD_RESET, 1);
    mdelay(5);
    gpio_set_value_cansleep(GPIO_TCL_LCD_RESET, 0);
    msleep(10);
    gpio_set_value_cansleep(GPIO_TCL_LCD_RESET, 1);
    msleep(150);
	printk("mipi_tcl_lcd_reset  rc = %d\n", rc);
    return rc;
}

static bool  mipi_djn8018b_resume = false;
static int mipi_djn8018b_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);
printk("zhou : enter %s %d\n ", __func__,__LINE__);
	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	if(!mfd->cont_splash_done){
        		mfd->cont_splash_done = 1;
		}//add for continue_display_splash.hxh

	if (mipi_tcl_lcd_reset() < 0) {
		pr_err("mipi_tcl_lcd_reset error\n");
		return -EINVAL;
	}
	
	
		if (mfd->panel_info.mipi.mode ==  DSI_VIDEO_MODE ){
	
			mipi_dsi_cmds_tx(mfd, &djn8018b_tx_buf, djn8018b_video_display_on_cmds,
					ARRAY_SIZE(djn8018b_video_display_on_cmds));
	
		}else  if (mfd->panel_info.mipi.mode == DSI_CMD_MODE) {
				mipi_dsi_cmds_tx(mfd, &djn8018b_tx_buf,djn8018b_cmd_display_on_cmds,
					ARRAY_SIZE(djn8018b_cmd_display_on_cmds));
	
			}

	     mipi_djn8018b_resume = true;
	//add by dong teng for flicker 2013.06.28 (start)
	mdelay(240);
        //add by dong teng for flicker 2013.06.28 (end )
printk("zhou : exit %s %d\n ", __func__,__LINE__);
	return 0;
}

static int mipi_djn8018b_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
printk("zhou : enter %s %d\n ", __func__,__LINE__);
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	
#if 0
		mipi_dsi_cmds_tx(mfd, &djn8018b_tx_buf, djn8018b_display_off_cmds,
				ARRAY_SIZE(djn8018b_display_off_cmds));
#endif
	gpio_set_value_cansleep(GPIO_TCL_LCD_RESET, 0);
	msleep(100);

	printk("zhou : exit %s %d\n ", __func__,__LINE__);
	return 0;
}

static int __devinit mipi_djn8018b_lcd_probe(struct platform_device *pdev)
{
	int rc = 0;
	pr_debug("%s\n", __func__);
printk("hxh: %s  %d \n", __func__, __LINE__	);
	if (pdev->id == 0) {
		mipi_djn8018b_pdata = pdev->dev.platform_data;
	    	if (mipi_djn8018b_pdata) {
			//gpio_backlight_en = mipi_djn8018b_pdata->gpio;	
			gpio_backlight_en = 96;
			printk("mipi_djn8018b_lcd_probe gpio_backlight_en %d\n", gpio_backlight_en);	
	    	}
		return 0;
	}

	rc = gpio_request(GPIO_TCL_LCD_RESET, "gpio_disp_reset");
	if(rc < 0) {
		pr_err("Error request gpio %d\n", GPIO_TCL_LCD_RESET);
	}

	rc = gpio_tlmm_config(GPIO_CFG(GPIO_TCL_LCD_RESET, 0,
				GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	
	if (rc < 0) {
		pr_err("Error config lcdc gpio:%d\n", GPIO_TCL_LCD_RESET);
	}

	rc = gpio_direction_output(GPIO_TCL_LCD_RESET, 1);
	if (rc < 0) {
		pr_err("Error direct lcdc gpio:%d\n", GPIO_TCL_LCD_RESET);
	}

	msm_fb_add_device(pdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_djn8018b_lcd_probe,
	.driver = {
		.name   = "mipi_djn_otm8018b",
	},
};

static void set_backlight_by_driver(struct msm_fb_data_type *mfd)
{
	int bl_level = mfd->bl_level;

	bl_0x51[1] = bl_level;
	
     	if (mipi_djn8018b_resume){
		msleep(70);
		mipi_djn8018b_resume= false;
	}
	
	mipi_dsi_cmds_tx(mfd, &djn8018b_tx_buf, tcl_cmd_drive_backlight_cmds,
		ARRAY_SIZE(tcl_cmd_drive_backlight_cmds));
		
/*	
		if(0 == bl_level){
			//Start===Allen
			keyboard_led_light(KEY_LED_USER_LCD, 0);
			//End===Allen
		}else {
			//Start===Allen
			keyboard_led_light(KEY_LED_USER_LCD,1);
			//End===Allen
		} 
*/

	 
}

static void set_backlight_by_host(struct msm_fb_data_type *mfd)
{
	int step = 0, i = 0;
    	unsigned long flags;
	int bl_level = (mfd->bl_level+1)/8;
	/* real backlight level, 32 - max, 1 - min, 0 - off */
	bl_0x51[1] = mfd->bl_level;
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
	//	gpio_set_value_cansleep(gpio_keybaord_en, 0);
	//		udelay(30);
	} else {
       local_irq_save(flags);	
         
		if (prev_bl == (BACKLIGHT_BRIGHTNESS_LEVEL + 1)) {
			/* turn on backlight */
			gpio_set_value_cansleep(gpio_backlight_en, 1);
			udelay(400);
			/*lilonghui add it for the keybaord led 2012-3-13*/
		//        gpio_set_value_cansleep(gpio_keybaord_en, 1);
		 //  	udelay(30);
		}		
		/* adjust backlight level */
		for (i = 0; i < step; i++) {
			gpio_set_value(gpio_backlight_en, 0);
			udelay(1);
			gpio_set_value(gpio_backlight_en, 1);
			udelay(1);
		}

		local_irq_restore(flags);                 
	}
	msleep(1);
	prev_bl = bl_level;
      /*end */

	mipi_dsi_cmds_tx(mfd, &djn8018b_tx_buf, tcl_cmd_drive_backlight_cmds,
			    ARRAY_SIZE(tcl_cmd_drive_backlight_cmds));
	
	return;
}

#ifdef DRVIE_BACKLIGHT_BY_HOST
	static int driver_by_host = 1;
#else
	static int driver_by_host = 0;
#endif

static void  mipi_djn8018b_tcl_set_backlight(struct msm_fb_data_type *mfd)
{

	if(driver_by_host) {
		set_backlight_by_host(mfd);
	}
	else {
		set_backlight_by_driver(mfd);
	}
}

static struct msm_fb_panel_data djn8018b_panel_data = {
	.on  = mipi_djn8018b_lcd_on,
	.off = mipi_djn8018b_lcd_off,
	.set_backlight = mipi_djn8018b_tcl_set_backlight,
};

static int ch_used[3];

int mipi_djn8018b_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

printk("hxh: %s  %d \n", __func__, __LINE__	);

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_djn_otm8018b", (panel << 8)|channel);

	if (!pdev)
		return -ENOMEM;

	djn8018b_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &djn8018b_panel_data,
		sizeof(djn8018b_panel_data));

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


static int __init mipi_djn8018b_lcd_init(void)
{
	mipi_dsi_buf_alloc(&djn8018b_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&djn8018b_rx_buf, DSI_BUF_SIZE);

	printk("__init mipi_djn8018b_lcd_init\n");
	return platform_driver_register(&this_driver);
}

module_init(mipi_djn8018b_lcd_init);

