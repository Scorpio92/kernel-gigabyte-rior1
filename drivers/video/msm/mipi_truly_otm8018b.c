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
#include "mipi_truly_otm8018b.h"
#include <linux/leds.h>  // add by hanxiaohui


#define DRVIE_BACKLIGHT_BY_HOST 1
#define GPIO_TRULY_LCD_RESET 85
/* common setting */
#define TRULY_CMD_DELAY 0
#define TRULY_SLEEP_OFF_DELAY 150
#define TRULY_DISPLAY_ON_DELAY 0
#define BACKLIGHT_BRIGHTNESS_LEVEL 32

static struct msm_panel_common_pdata *mipi_truly8018b_pdata;
static struct dsi_buf truly8018b_tx_buf;
static struct dsi_buf truly8018b_rx_buf;

static int gpio_backlight_en = 96;
//static int gpio_keybaord_en = 0xff;

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

static char TRULY_OTM8018B_0XFF1[] = {0xFF,0x80,0x09,0x01};                                 
static char TRULY_OTM8018B_0X001[] = {0x00,0x80};
static char TRULY_OTM8018B_0XFF2[] = {0xFF,0x80,0x09};

static char TRULY_OTM8018B_0X002[] = {0x00,0x03};
static char TRULY_OTM8018B_0XFF3[] = {0xFF,0x01};

static char TRULY_OTM8018B_0X003[] = {0x00,0xB4};
static char TRULY_OTM8018B_0XC00[] = {0xC0,0x10};     //0x10, 0x50 colomn 

static char TRULY_OTM8018B_0X004[] = {0x00,0x89};
static char TRULY_OTM8018B_0XC40[] = {0xC4,0x08};

static char TRULY_OTM8018B_0X005[] = {0x00,0xA3};
static char TRULY_OTM8018B_0XC01[] = {0xc0,0x00};

static char TRULY_OTM8018B_0X006[] = {0X00,0x82};
static char TRULY_OTM8018B_0XC50[] = {0xC5,0xA3};

static char TRULY_OTM8018B_0X007[] = {0x00,0x90};
static char TRULY_OTM8018B_0XC51[] = {0xC5,0x96,0x87};  // 0xd6 , 0xA6  2*VDD 

static char TRULY_OTM8018B_0X008[] = {0X00,0x00};
static char TRULY_OTM8018B_0XD80[] = {0xD8,0x72,0x72};

static char TRULY_OTM8018B_0X009[] = {0x00,0x00};
static char TRULY_OTM8018B_0XD90[] = {0xD9,0x61};

static char TRULY_OTM8018B_0X0010[] = {0x00,0x00};
static char TRULY_OTM8018B_0XE10[] = {0xE1,0x09,0x0a,0x0e,0x0d,0x07,0x18,0x0d,0x0d,0x01,0x04,0x05,0x06,0x0e,0x25,0x22,0x05};

static char TRULY_OTM8018B_0X0011[] = {0x00,0x00};
static char TRULY_OTM8018B_0XE20[] = {0xE2,0x09,0x0a,0x0e,0x0d,0x07,0x18,0x0d,0x0d,0x01,0x04,0x05,0x06,0x0e,0x25,0x22,0x05};

static char TRULY_OTM8018B_0X0012[] = {0x00,0x81};
static char TRULY_OTM8018B_0XC10[] = {0xC1,0x55};  //0x66

static char TRULY_OTM8018B_0X0013[] = {0x00,0xA0};
static char TRULY_OTM8018B_0XC11[] = {0xC1,0xea};

static char TRULY_OTM8018B_0X00A2[] = {0x00,0xA2};
static char TRULY_OTM8018B_0XC0[] = {0xC0,0x04,0x00,0x02};

static char TRULY_OTM8018B_0X0015[] = {0x00,0x80};
static char TRULY_OTM8018B_0XC42[] = {0xC4,0x30};

static char TRULY_OTM8018B_0X0016[] = {0x00,0xc0};
static char TRULY_OTM8018B_0XC52[] = {0xC5,0x00};

static char TRULY_OTM8018B_0X000B_0[] = {0x00,0x8B};
static char TRULY_OTM8018B_0XB040[] = {0xB0,0x40};

static char TRULY_OTM8018B_0X000B_1[] = {0x00,0xb2};
static char TRULY_OTM8018B_0XF515[] = {0xF5,0x15,0x00,0x15,0x00};

static char TRULY_OTM8018B_0X0081[] = {0x00,0x81};
static char TRULY_OTM8018B_0XC481[] = {0xC4,0x83};

static char TRULY_OTM8018B_0X0092[] = {0x00,0x92};
static char TRULY_OTM8018B_0XC501[] = {0xC5,0x01,0x03};

static char TRULY_OTM8018B_0X0017[] = {0x00,0xB1};
static char TRULY_OTM8018B_0XC53[] = {0xC5,0xA9};

static char TRULY_OTM8018B_0X0090[] = {0x00,0x90};
static char TRULY_OTM8018B_0XC000[] = {0xC0,0x00,0x44,0x00,0x00,0x00,0x03};

static char TRULY_OTM8018B_0X00a6[] = {0x00,0xa6};
static char TRULY_OTM8018B_0XC100[] = {0xC1,0x01,0x00,0x00};

static char TRULY_OTM8018B_0X0018[] = {0x00,0x80};
static char TRULY_OTM8018B_0XCE0[] = {0xCE,0x87,0x03,0x00,0x85,0x03,0x00,0x86,0x03,0x00,0x84,0x03,0x00};

static char TRULY_OTM8018B_0X0019[] = {0x00,0xA0};
static char TRULY_OTM8018B_0XCE1[] = {0xCE,0x38,0x03,0x03,0x20,0x00,0x00,0x00,0x38,0x02,0x03,0x21,0x00,0x00,0x00};

static char TRULY_OTM8018B_0X0020[] = {0x00,0xB0};
static char TRULY_OTM8018B_0XCE2[] = {0xCE,0x38,0x01,0x03,0x22,0x00,0x00,0x00,0x38,0x00,0x03,0x23,0x00,0x00,0x00};

static char TRULY_OTM8018B_0X0021[] = {0x00,0xC0};
static char TRULY_OTM8018B_0XCE3[] = {0xCE,0x30,0x00,0x03,0x24,0x00,0x00,0x00,0x30,0x01,0x03,0x25,0x00,0x00,0x00};

static char TRULY_OTM8018B_0X0022[] = {0x00,0xD0};
static char TRULY_OTM8018B_0XCE4[] = {0xCE,0x30,0x02,0x03,0x26,0x00,0x00,0x00,0x30,0x03,0x03,0x27,0x00,0x00,0x00};

static char TRULY_OTM8018B_0X00c7[] = {0x00,0xc7};
static char TRULY_OTM8018B_0XCFc7[] = {0xCF,0x00};

static char TRULY_OTM8018B_0X00c9[] = {0x00,0xc9};
static char TRULY_OTM8018B_0XCFc9[] = {0xCF,0X00};

static char TRULY_OTM8018B_0X0024[] = {0x00,0xD0};
static char TRULY_OTM8018B_0XCF1[] = {0xCF,0x00};

static char TRULY_OTM8018B_0X0025[] = {0x00,0xC0};
static char TRULY_OTM8018B_0XCB0[] = {0xCB,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00,0x00,0x00};

static char TRULY_OTM8018B_0X0026[] = {0x00,0xD0};
static char TRULY_OTM8018B_0XCB1[] =  {0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x04};

static char TRULY_OTM8018B_0X0027[] = {0x00,0x80};
static char TRULY_OTM8018B_0XCC0[] =  {0xCC,0x00,0x00,0x00,0x00,0x0C,0x0A,0x10,0x0E,0x03,0x04};

static char TRULY_OTM8018B_0X0028[] = {0x00,0x90};
static char TRULY_OTM8018B_0XCC1[] =  {0xCC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0B};

static char TRULY_OTM8018B_0X0029[] = {0x00,0xA0};
static char TRULY_OTM8018B_0XCC2[] =  {0xCC,0x09,0x0F,0x0D,0x01,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static char TRULY_OTM8018B_0X0030[] = {0x00,0xB0};
static char TRULY_OTM8018B_0XCC3[] =  {0xCC,0x00,0x00,0x00,0x00,0x0D,0x0F,0x09,0x0B,0x02,0x01};

static char TRULY_OTM8018B_0X0031[] = {0x00,0xC0};
static char TRULY_OTM8018B_0XCC4[] =  {0xCC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0E};

static char TRULY_OTM8018B_0X0032[] = {0x00,0xD0};
static char TRULY_OTM8018B_0XCC5[] =  {0xCC,0x10,0x0A,0x0C,0x04,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static char TRULY_OTM8018B_0XCC6[] =  {0x3A,0X77};



static struct dsi_cmd_desc truly8018b_video_display_on_cmds[] = {

	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XFF1), TRULY_OTM8018B_0XFF1},
	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X001), TRULY_OTM8018B_0X001},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XFF2), TRULY_OTM8018B_0XFF2},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X002), TRULY_OTM8018B_0X002},
	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XFF3), TRULY_OTM8018B_0XFF3},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X003), TRULY_OTM8018B_0X003},
	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XC00), TRULY_OTM8018B_0XC00},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X004), TRULY_OTM8018B_0X004},
	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XC40), TRULY_OTM8018B_0XC40},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X005), TRULY_OTM8018B_0X005},
	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XC01), TRULY_OTM8018B_0XC01},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X006), TRULY_OTM8018B_0X006},
	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XC50), TRULY_OTM8018B_0XC50},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X007), TRULY_OTM8018B_0X007},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XC51), TRULY_OTM8018B_0XC51},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X008), TRULY_OTM8018B_0X008},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XD80), TRULY_OTM8018B_0XD80},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X009), TRULY_OTM8018B_0X009},
	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XD90), TRULY_OTM8018B_0XD90},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0010), TRULY_OTM8018B_0X0010},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XE10), TRULY_OTM8018B_0XE10},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0011), TRULY_OTM8018B_0X0011},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XE20), TRULY_OTM8018B_0XE20},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0012), TRULY_OTM8018B_0X0012},
	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XC10), TRULY_OTM8018B_0XC10},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0013), TRULY_OTM8018B_0X0013},
	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XC11), TRULY_OTM8018B_0XC11},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X00A2), TRULY_OTM8018B_0X00A2},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XC0), TRULY_OTM8018B_0XC0},// 1

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0015), TRULY_OTM8018B_0X0015},
	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XC42), TRULY_OTM8018B_0XC42},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0016), TRULY_OTM8018B_0X0016},
	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XC52), TRULY_OTM8018B_0XC52},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X000B_0), TRULY_OTM8018B_0X000B_0},
	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XB040), TRULY_OTM8018B_0XB040},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X000B_1), TRULY_OTM8018B_0X000B_1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XF515), TRULY_OTM8018B_0XF515},//3

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0081), TRULY_OTM8018B_0X0081},
	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XC481), TRULY_OTM8018B_0XC481},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0092), TRULY_OTM8018B_0X0092},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XC501), TRULY_OTM8018B_0XC501},//4

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0017), TRULY_OTM8018B_0X0017},
	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XC53), TRULY_OTM8018B_0XC53},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0090), TRULY_OTM8018B_0X0090},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XC000), TRULY_OTM8018B_0XC000},//5

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X00a6), TRULY_OTM8018B_0X00a6},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XC100), TRULY_OTM8018B_0XC100},//6

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0018), TRULY_OTM8018B_0X0018},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XCE0), TRULY_OTM8018B_0XCE0},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0019), TRULY_OTM8018B_0X0019},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XCE1), TRULY_OTM8018B_0XCE1},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0020), TRULY_OTM8018B_0X0020},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XCE2), TRULY_OTM8018B_0XCE2},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0021), TRULY_OTM8018B_0X0021},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XCE3), TRULY_OTM8018B_0XCE3},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0022), TRULY_OTM8018B_0X0022},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XCE4), TRULY_OTM8018B_0XCE4},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X00c7), TRULY_OTM8018B_0X00c7},
	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XCFc7), TRULY_OTM8018B_0XCFc7},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X00c9), TRULY_OTM8018B_0X00c9},
	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XCFc9), TRULY_OTM8018B_0XCFc9},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0024), TRULY_OTM8018B_0X0024},
	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XCF1), TRULY_OTM8018B_0XCF1},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0025), TRULY_OTM8018B_0X0025},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XCB0), TRULY_OTM8018B_0XCB0},

        {DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0026), TRULY_OTM8018B_0X0026},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XCB1), TRULY_OTM8018B_0XCB1},
 
	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0027), TRULY_OTM8018B_0X0027},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XCC0), TRULY_OTM8018B_0XCC0},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0028), TRULY_OTM8018B_0X0028},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XCC1), TRULY_OTM8018B_0XCC1},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0029), TRULY_OTM8018B_0X0029},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XCC2), TRULY_OTM8018B_0XCC2},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0030), TRULY_OTM8018B_0X0030},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XCC3), TRULY_OTM8018B_0XCC3},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0031), TRULY_OTM8018B_0X0031},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XCC4), TRULY_OTM8018B_0XCC4},

	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0X0032), TRULY_OTM8018B_0X0032},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XCC5), TRULY_OTM8018B_0XCC5},
	{DTYPE_GEN_WRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(TRULY_OTM8018B_0XCC6), TRULY_OTM8018B_0XCC6},

	

	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_SLEEP_OFF_DELAY, sizeof(exit_sleep), exit_sleep},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(display_on), display_on},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(write_memory_start), write_memory_start},

};

static struct dsi_cmd_desc tcl_cmd_drive_backlight_cmds[] = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(bl_0x51), bl_0x51},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(bl_0x53), bl_0x53},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(bl_0x55), bl_0x55},
}; 
#if 0
static struct dsi_cmd_desc truly8018b_display_off_cmds[] = {
	{DTYPE_GEN_WRITE, 1, 0, 0, 100, sizeof(enter_sleep), enter_sleep},
	{DTYPE_GEN_WRITE, 1, 0, 0, 0, sizeof(display_off), display_off},
};
#endif
static struct dsi_cmd_desc truly8018b_cmd_display_on_cmds[] = {

	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_SLEEP_OFF_DELAY, sizeof(exit_sleep), exit_sleep},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(display_on), display_on},
	{DTYPE_GEN_LWRITE, 1, 0, 0, TRULY_CMD_DELAY, sizeof(write_memory_start), write_memory_start},
};

static int mipi_truly_otm8018b_lcd_reset(void)
{
    int rc = 0;
    gpio_set_value_cansleep(GPIO_TRULY_LCD_RESET, 1);
    mdelay(5);
    gpio_set_value_cansleep(GPIO_TRULY_LCD_RESET, 0);
    msleep(10);
    gpio_set_value_cansleep(GPIO_TRULY_LCD_RESET, 1);
    msleep(150);
    return rc;
}

static bool  mipi_truly8018b_resume = false;
static int mipi_truly8018b_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	if(!mfd->cont_splash_done){
        		mfd->cont_splash_done = 1;
		}//add for continue_display_splash.hxh

	if (mipi_truly_otm8018b_lcd_reset() < 0) {
		pr_err("mipi_truly_otm8018b_lcd_reset error\n");
		return -EINVAL;
	}
	
	
		if (mfd->panel_info.mipi.mode ==  DSI_VIDEO_MODE ){
	
			mipi_dsi_cmds_tx(mfd, &truly8018b_tx_buf, truly8018b_video_display_on_cmds,
					ARRAY_SIZE(truly8018b_video_display_on_cmds));
	
		}else  if (mfd->panel_info.mipi.mode == DSI_CMD_MODE) {
				mipi_dsi_cmds_tx(mfd, &truly8018b_tx_buf,truly8018b_cmd_display_on_cmds,
					ARRAY_SIZE(truly8018b_cmd_display_on_cmds));
	
			}

	     mipi_truly8018b_resume = true;
	

	return 0;
}

static int mipi_truly8018b_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	
#if 0
		mipi_dsi_cmds_tx(mfd, &truly8018b_tx_buf, truly8018b_display_off_cmds,
				ARRAY_SIZE(truly8018b_display_off_cmds));
#endif
	gpio_set_value_cansleep(GPIO_TRULY_LCD_RESET, 0);
	msleep(100);

	return 0;
}

static int __devinit mipi_truly8018b_lcd_probe(struct platform_device *pdev)
{
	int rc = 0;
	pr_debug("%s\n", __func__);
	if (pdev->id == 0) {
		mipi_truly8018b_pdata = pdev->dev.platform_data;
	    	if (mipi_truly8018b_pdata) {
			gpio_backlight_en = mipi_truly8018b_pdata->gpio;		
	    	}
		return 0;
	}

	rc = gpio_request(GPIO_TRULY_LCD_RESET, "gpio_disp_reset");
	if(rc < 0) {
		pr_err("Error request gpio %d\n", GPIO_TRULY_LCD_RESET);
	}

	rc = gpio_tlmm_config(GPIO_CFG(GPIO_TRULY_LCD_RESET, 0,
				GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	
	if (rc < 0) {
		pr_err("Error config lcdc gpio:%d\n", GPIO_TRULY_LCD_RESET);
	}

	rc = gpio_direction_output(GPIO_TRULY_LCD_RESET, 1);
	if (rc < 0) {
		pr_err("Error direct lcdc gpio:%d\n", GPIO_TRULY_LCD_RESET);
	}

	msm_fb_add_device(pdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_truly8018b_lcd_probe,
	.driver = {
		.name   = "mipi_truly_otm8018b",
	},
};

static void mipi_truly8018b_tcl_set_backlight(struct msm_fb_data_type *mfd)
{
	int bl_level = mfd->bl_level;

	bl_0x51[1] = bl_level;
	
     	if (mipi_truly8018b_resume){
		msleep(70);
		mipi_truly8018b_resume= false;
	}
	
	mipi_dsi_cmds_tx(mfd, &truly8018b_tx_buf, tcl_cmd_drive_backlight_cmds,
		ARRAY_SIZE(tcl_cmd_drive_backlight_cmds));

	 
}


static struct msm_fb_panel_data truly8018b_panel_data = {
	.on  = mipi_truly8018b_lcd_on,
	.off = mipi_truly8018b_lcd_off,
	.set_backlight = mipi_truly8018b_tcl_set_backlight,
};

static int ch_used[3];

int mipi_truly_otm8018b_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_truly_otm8018b", (panel << 8)|channel);

	if (!pdev)
		return -ENOMEM;

	truly8018b_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &truly8018b_panel_data,
		sizeof(truly8018b_panel_data));

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


static int __init mipi_truly8018b_lcd_init(void)
{
	mipi_dsi_buf_alloc(&truly8018b_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&truly8018b_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

module_init(mipi_truly8018b_lcd_init);

