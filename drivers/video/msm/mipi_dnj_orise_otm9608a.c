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
 */

#include "mach/gpio.h"
#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_dnj_orise_otm9608a.h"
#include <mach/pmic.h>
#include <linux/leds.h>  // add by hanxiaohui
#define GPIO_OTM_LCD_RESET 85
#define BACKLIGHT_BRIGHTNESS_LEVEL 32
#define OTM_CMD_DELAY 0

//#define DRVIE_BACKLIGHT_BY_HOST
#ifdef DRVIE_BACKLIGHT_BY_HOST
	static int driver_by_host = 1;
#else
	static int driver_by_host = 0;
#endif

static int prev_bl = 17;
static int gpio_backlight_en = 0xff;

static struct msm_panel_common_pdata *mipi_otm_pdata;
static struct dsi_buf otm_tx_buf;
static struct dsi_buf otm_rx_buf;

static char exit_sleep[] = {0x11, 0x00};
static char display_on[] = {0x29, 0x00};
static char display_off[] = {0x28, 0x00};
static char enter_sleep[] = {0x10, 0x00};

static struct dsi_cmd_desc otm_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(enter_sleep), enter_sleep}
};

static char OTM_CUSTOMER_0X51[] = {0x51,0xFF};  
static char OTM_CUSTOMER_0X53[] = {0x53,0x24};  
static char OTM_CUSTOMER_0X55[] = {0x55,0x00};  

static struct dsi_cmd_desc otm_cmd_drive_backlight_cmds[] = {
	{DTYPE_GEN_WRITE1, 1, 0, 0, OTM_CMD_DELAY, sizeof(OTM_CUSTOMER_0X51), OTM_CUSTOMER_0X51},
	{DTYPE_GEN_WRITE1, 1, 0, 0, OTM_CMD_DELAY, sizeof(OTM_CUSTOMER_0X53), OTM_CUSTOMER_0X53},
	{DTYPE_GEN_WRITE1, 1, 0, 0, OTM_CMD_DELAY, sizeof(OTM_CUSTOMER_0X55), OTM_CUSTOMER_0X55},
};

//static char cmd_pixel_format_setting[2] = {0x3A, 0x77};
//static char cmd_2A_setting[5] = {0x2A, 0x00, 0x00,0x02,0x1B};
//static char cmd_2B_setting[5] = {0x2B, 0x00, 0x00,0x03,0xBF};

static char otm9608a_0x00_x[] = {0x00, 0x00}; // 0
static char otm9608a_0xFF_0[] = {0xFF, 0x96, 0x08, 0x01};

static char otm9608a_0x00_0[] = {0x00, 0x80};//1    
static char otm9608a_0xFF_1[] = {0xFF, 0x96, 0x08};

//static char otm9608a_0x00_xx[] = {0x00, 0x00};//add   
//static char otm9608a_0xFF_xxx[] = {0x21, 0x00};

#if 0
/*FOR ONE LANE*/
static char otm9608a_0x00_X1[2] = {0x00, 0x92};
static char otm9608a_0xFF_X2[2] = {0xFF, 0x00};

static char otm9608a_0x00_X3[2] = {0x00, 0x93};
static char otm9608a_0xFF_X4[2] = {0xFF, 0x02};
//=============
#endif

static char otm9608a_0x00_1[] = {0x00, 0x00};//2
static char otm9608a_0xA0_0[] = {0xA0, 0x00};

static char otm9608a_0x00_2[] = {0x00, 0x80};//3
static char otm9608a_0xB3_0[] = {0xB3,0x00,0x00,0x00,0x21,0x00};

static char otm9608a_0x00_3[] = {0x00,0x92};//4
static char otm9608a_0xB3_1[] = {0xB3,0x01};

static char otm9608a_0x00_4[] = {0x00,0xC0};//5
static char otm9608a_0xC0_0[] = {0xB3,0x19};

static char otm9608a_0x00_5[] = {0x00,0x80};//6
static char otm9608a_0xC0_1[] = {0xC0,0x00,0x48,0x00,0x10,0x10,0x00,0x48,0x10,0x10};

static char otm9608a_0x00_6[] = {0x00,0x92};//7
static char otm9608a_0xC0_2[] = {0xC0,0x00,0x0E,0x00,0x11};

static char otm9608a_0x00_7[] = {0x00,0xA2};//8
static char otm9608a_0xC0_3[] = {0xC0,0x01,0x10,0x00};

static char otm9608a_0x00_8[] = {0x00,0xB3};//9
static char otm9608a_0xC1[]   = {0xC0,0x00,0x50};

static char otm9608a_0x00_9[] = {0x00,0x81};//10
static char otm9608a_0xC4_0[] = {0xC1,0x57};// normal: 70HZ

static char otm9608a_0x00_10[] = {0x00,0x80};//11
static char otm9608a_0xC4_1[]  = {0xC4,0x00,0x84,0xFA};

static char otm9608a_0x00_11[] = {0x00,0xA0};//12
static char otm9608a_0xC5_0[]  = {0xC4,0x33,0x09,0x90,0x2B,0x33,0x09,0x90,0x54};

static char otm9608a_0x00_12[] = {0x00,0x80};//13
static char otm9608a_0xC5_1[]  = {0xC5,0x08,0x00,0x90,0x11};
				   
static char otm9608a_0x00_13[] = {0x00,0x90};//14
static char otm9608a_0xC5_2[]  = {0xC5,0x84,0x77,0x00,0x77,0x33,0x33,0x34};

static char otm9608a_0x00_14[] = {0x00,0xA0};//15
static char otm9608a_0xC5_3[]  = {0xC5,0x96,0x77,0x06,0x77,0x33,0x33,0x34};

static char otm9608a_0x00_15[] = {0x00,0xB0};//16
static char otm9608a_0xD8[]    = {0xC5,0x04,0xF8};

static char otm9608a_0x00_16[] = {0x00,0x80};//17
static char otm9608a_0xD9[]    = {0xC6,0x64};

static char otm9608a_0x00_17[] = {0x00,0xB0};//18
static char otm9608a_0xC6_0[]  = {0xC6,0x03,0x10,0x00,0x1F,0x12};

static char otm9608a_0x00_18[] = {0x00,0xE1};//19
static char otm9608a_0xC6_1[]  = {0xC0,0x9F};

static char otm9608a_0x00_19[] = {0x00,0x00};//20
static char otm9608a_0xD0[]    = {0xD0,0x01};

static char otm9608a_0x00_20[] = {0x00,0x00};//21
static char otm9608a_0xD1[]    = {0xD1,0x01,0x01};

static char otm9608a_0x00_21[] = {0x00,0xB7};//22
static char otm9608a_0xB0_0[]  = {0xB0,0x10};

static char otm9608a_0x00_22[] = {0x00,0xC0};//23
static char otm9608a_0xB0_1[]  = {0xB0,0x55};

static char otm9608a_0x00_23[] = {0x00,0xB1};//24
static char otm9608a_0xB0_2[]  = {0xB0,0x03,0x06};

//static char otm9608a_0x35_1[2] =  {0X35,0x00};     

//static char otm9608a_0x00_x[2] = {0x00, 0xA1};
//static char otm9608a_0xB0_x[2] = {0xB0, 0x02};

static char otm9608a_0x00_24[] = {0x00,0x80};//25
static char otm9608a_0xE1[] = {
0xCB,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x00,0x00
};

static char otm9608a_0x00_25[] = {0x00,0x90};//26
static char otm9608a_0xE2[] = {
0xCB,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00
};

static char otm9608a_0x00_26[] = {0x00,0xA0};//27
static char otm9608a_0xCB_0[] = {
0xCB,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00
};

static char otm9608a_0x00_27[] = {0x00,0xB0};//28
static char otm9608a_0xCB_1[] = {
0xCB,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x00,0x00
};

static char otm9608a_0x00_28[] = {0x00,0xC0};//29
static char otm9608a_0xCB_2[] = {
0xCB,0x00,0x00,0x04,
0x04,0x04,0x04,0x04,
0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00
};

static char otm9608a_0x00_29[] = {0x00,0xD0};//30
static char otm9608a_0xCB_3[] = {
0xCB,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x04,0x04,0x04,0x04,
0x04,0x00,0x00,0x00
};

static char otm9608a_0x00_30[] = {0x00,0xE0};//31
static char otm9608a_0xCB_4[] = {
0xCB,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x00,0x00
};

static char otm9608a_0x00_31[] = {0x00,0xF0};//32
static char otm9608a_0xCB_5[] = {
0xCB,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF
};

static char otm9608a_0x00_32[] = {0x00,0x80};//33
static char otm9608a_0xCB_6[] = {
0xCC,0x00,0x00,0x0B,
0x09,0x01,0x25,0x26,
0x00,0x00,0x00
};
static char otm9608a_0x00_33[] = {0x00,0x90};//34
static char otm9608a_0xCB_7[] = {
0xCC,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x0C,0x0A,0x02
};

static char otm9608a_0x00_34[] = {0x00,0xA0};//35
static char otm9608a_0xCC_0[] = {
0xCC,0x25,0x26,0x00,
0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00
};

static char otm9608a_0x00_35[] = {0x00,0xB0};//36
static char otm9608a_0xCC_1[] = {
0xCC,0x00,0x00,0x0A,
0x0C,0x02,0x26,0x25,
0x00,0x00,0x00
};

static char otm9608a_0x00_36[] = {0x00,0xC0};//37
static char otm9608a_0xCC_2[] = {
0xCC,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x09,0x0B,0x01
};

static char otm9608a_0x00_37[] = {0x00,0xD0};//38
static char otm9608a_0xCC_3[] = {
0xCC,0x26,0x25,0x00,
0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00
};

static char otm9608a_0x00_38[] = {0x00,0x80};//39
static char otm9608a_0xCC_4[] = {
0xCE,0x88,0x01,0x06,
0x87,0x01,0x06,0x0F,
0x00,0x00,0x0F,0x00,
0x00
};

static char otm9608a_0x00_39[] = {0x00,0x90};//40
static char otm9608a_0xCC_5[] = {
0xCE,0x13,0xbf,0x28,
0x13,0xC0,0x28,0xF0,
0x00,0x00,0xF0,0x00,
0x00,0x00,0x00
};

static char otm9608a_0x00_40[] = {0x00,0xA0};//41
static char otm9608a_0xCE_0[] = {
0xCE,0x18,0x06,0x03,
0xC1,0x00,0x06,0x00,
0x18,0x05,0x03,0xC2,
0x00,0x06,0x00
};

static char otm9608a_0x00_41[] = {0x00,0xB0};//42
static char otm9608a_0xCE_1[] = {
0xCE,0x18,0x04,0x03,
0xC3,0x00,0x06,0x00,
0x18,0x03,0x03,0xC4,
0x00,0x06,0x00
};

static char otm9608a_0x00_42[] = {0x00,0xC0};//43
static char otm9608a_0xCE_2[] = {
0xCE,0xF0,0x00,0x00,
0x10,0x00,0x00,0x00,
0xF0,0x00,0x00,0x10,
0x00,0x00,0x00
};

static char otm9608a_0x00_43[] = {0x00,0xD0};//44
static char otm9608a_0xCE_3[] = {
0xCE,0xF0,0x00,0x00,
0x10,0x00,0x00,0x00,
0xF0,0x00,0x00,0x10,
0x00,0x00,0x00
};

static char otm9608a_0x00_44[] = {0x00,0x80};//45
static char otm9608a_0xCE_4[] = {
0xCF,0xF0,0x00,0x00,
0x10,0x00,0x00,0x00,
0xF0,0x00,0x00,0x10,
0x00,0x00,0x00
};

static char otm9608a_0x00_45[] = {0x00,0x90};//46
static char otm9608a_0xCE_5[] = {
0xCF,0xF0,0x00,0x00,
0x10,0x00,0x00,0x00,
0xF0,0x00,0x00,0x10,
0x00,0x00,0x00
};

static char otm9608a_0x00_46[] = {0x00,0xA0};//47
static char otm9608a_0xCF_0[] = {
0xCF,0xF0,0x00,0x00,
0x10,0x00,0x00,0x00,
0xF0,0x00,0x00,0x10,
0x00,0x00,0x00
};

static char otm9608a_0x00_47[] = {0x00,0xB0};//48
static char otm9608a_0xCF_1[] = {
0xCF,0xF0,0x00,0x00,
0x10,0x00,0x00,0x00,
0xF0,0x00,0x00,0x10,
0x00,0x00,0x00
};

static char otm9608a_0x00_48[] = {0x00,0xC0};//49
static char otm9608a_0xCF_2[] = {
0xCF,0x02,0x02,0x00,
0x00,0x00,0x00,0x01,
0x01,0x10,0x00
};

static char otm9608a_0x00_49[] = {0x00,0x80};//50
static char otm9608a_0xCF_3[] = {
0xD6,0x00
};

static char otm9608a_0x00_50[] = {0x00,0x00};//51
static char otm9608a_0xCF_4[] = {
0xD7,0x00
};

static char otm9608a_0x00_51[] = {0x00,0x00};//52
static char otm9608a_0xFF_2[] = {0xD8,0x6F,0x6F};  //6f , 6f ->  1f

static char otm9608a_offset_01[] = {0x00,0x00};//53
static char otm9608a_adpara_01[] = {0xD9,0x45};

static char otm9608a_offset_02[] = {0x00,0x00};//54
static char otm9608a_adpara_02[] = {
0xE1,0x07,0x0a,0x10,
0x0f,0x08,0x0d,0x0a,
0x09,0x04,0x07,0x0f,
0x08,0x0e,0x0c,0x07,
0x00
};

static char otm9608a_offset_03[] = {0x00,0x00};//55
static char otm9608a_adpara_03[] = {
0xE2,0x07,0x0a,0x10,
0x0f,0x08,0x0d,0x0a,
0x09,0x04,0x07,0x0f,
0x08,0x0e,0x0c,0x07,
0x00
};

static char otm9608a_offset_04[] = {0x00,0x00};//56
static char otm9608a_adpara_04[] = {
0xEC,0x40,0x44,0x44,
0x44,0x44,0x44,0x44,
0x44,0x44,0x43,0x44,
0x44,0x44,0x43,0x44,
0x44,0x44,0x44,0x43,
0x44,0x44,0x33,0x44,
0x44,0x33,0x33,0x44,
0x44,0x44,0x44,0x44,
0x44,0x43,0x00
};

static char otm9608a_offset_05[] = {0x00,0x00};//57
static char otm9608a_adpara_05[] = {
0xED,0x40,0x44,0x44,
0x44,0x44,0x44,0x44,
0x44,0x44,0x44,0x44,
0x44,0x44,0x44,0x44,
0x44,0x44,0x44,0x44,
0x44,0x44,0x44,0x44,
0x44,0x44,0x44,0x44,
0x44,0x44,0x44,0x44,
0x44,0x44,0x00
};

static char otm9608a_offset_06[] = {0x00,0x00};//58
static char otm9608a_adpara_06[] = {
0xEE,0x40,0x04,0x44,
0x44,0x44,0x44,0x33,
0x44,0x44,0x43,0x44,
0x44,0x44,0x43,0x44,
0x44,0x44,0x43,0x33,
0x44,0x44,0x33,0x44,
0x44,0x43,0x33,0x44,
0x44,0x43,0x44,0x44,
0x44,0x42,0x00
};

static char otm9608a_offset_07[] = {0x00,0x00};//59
static char otm9608a_adpara_07[] = {0xFF,0xFF,0xFF,0xFF};

static struct dsi_cmd_desc otm_cmd_display_on_cmds[] = {

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_x), otm9608a_0x00_x},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xFF_0), otm9608a_0xFF_0},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_0), otm9608a_0x00_0},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xFF_1), otm9608a_0xFF_1},

	//{DTYPE_DCS_WRITE, 1, 0, 0, OTM_CMD_DELAY,  sizeof(otm9608a_0xFF_xxx), otm9608a_0xFF_xxx},//add

//=====
	//{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_X1), otm9608a_0x00_X1},
	//{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xFF_X2), otm9608a_0xFF_X2},
	//{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_X3), otm9608a_0x00_X3},
	//{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xFF_X4), otm9608a_0xFF_X4},
//======
	
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_1), otm9608a_0x00_1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xA0_0), otm9608a_0xA0_0},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_2), otm9608a_0x00_2},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xB3_0), otm9608a_0xB3_0},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_3), otm9608a_0x00_3},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xB3_1), otm9608a_0xB3_1},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_4), otm9608a_0x00_4},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xC0_0), otm9608a_0xC0_0},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_5), otm9608a_0x00_5},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xC0_1), otm9608a_0xC0_1},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_6), otm9608a_0x00_6},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xC0_2), otm9608a_0xC0_2},	
 
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_7), otm9608a_0x00_7},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xC0_3), otm9608a_0xC0_3},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_8), otm9608a_0x00_8},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xC1), otm9608a_0xC1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_9), otm9608a_0x00_9},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xC4_0), otm9608a_0xC4_0},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_10), otm9608a_0x00_10},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xC4_1), otm9608a_0xC4_1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_11), otm9608a_0x00_11},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xC5_0), otm9608a_0xC5_0},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_12), otm9608a_0x00_12},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xC5_1), otm9608a_0xC5_1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_13), otm9608a_0x00_13},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xC5_2), otm9608a_0xC5_2},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_14), otm9608a_0x00_14},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xC5_3), otm9608a_0xC5_3},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_15), otm9608a_0x00_15},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xD8), otm9608a_0xD8},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_16), otm9608a_0x00_16},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xD9), otm9608a_0xD9},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_17), otm9608a_0x00_17},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xC6_0), otm9608a_0xC6_0},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_18), otm9608a_0x00_18},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xC6_1), otm9608a_0xC6_1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_19), otm9608a_0x00_19},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xD0), otm9608a_0xD0},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_20), otm9608a_0x00_20},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xD1), otm9608a_0xD1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_21), otm9608a_0x00_21},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xB0_0), otm9608a_0xB0_0},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_22), otm9608a_0x00_22},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xB0_1), otm9608a_0xB0_1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_23), otm9608a_0x00_23},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xB0_2), otm9608a_0xB0_2},

	//{DTYPE_DCS_WRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x35_1), otm9608a_0x35_1},    
	//{DTYPE_DCS_WRITE1, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x36_1), otm9608a_0x36_1},   

	//{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_x), otm9608a_0x00_x},
	//{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xB0_x), otm9608a_0xB0_x},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_24), otm9608a_0x00_24},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 10,            sizeof(otm9608a_0xE1), otm9608a_0xE1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_25), otm9608a_0x00_25},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 10,            sizeof(otm9608a_0xE2), otm9608a_0xE2},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_26), otm9608a_0x00_26},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCB_0), otm9608a_0xCB_0},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_27), otm9608a_0x00_27},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCB_1), otm9608a_0xCB_1},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_28), otm9608a_0x00_28},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCB_2), otm9608a_0xCB_2},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_29), otm9608a_0x00_29},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCB_3), otm9608a_0xCB_3},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_30), otm9608a_0x00_30},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCB_4), otm9608a_0xCB_4},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_31), otm9608a_0x00_31},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCB_5), otm9608a_0xCB_5},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_32), otm9608a_0x00_32},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCB_6), otm9608a_0xCB_6},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_33), otm9608a_0x00_33},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCB_7), otm9608a_0xCB_7},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_34), otm9608a_0x00_34},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCC_0), otm9608a_0xCC_0},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_35), otm9608a_0x00_35},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCC_1), otm9608a_0xCC_1},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_36), otm9608a_0x00_36}, ///
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCC_2), otm9608a_0xCC_2},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_37), otm9608a_0x00_37}, ///
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCC_3), otm9608a_0xCC_3},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_38), otm9608a_0x00_38},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCC_4), otm9608a_0xCC_4},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_39), otm9608a_0x00_39},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCC_5), otm9608a_0xCC_5},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_40), otm9608a_0x00_40},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCE_0), otm9608a_0xCE_0},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_41), otm9608a_0x00_41},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCE_1), otm9608a_0xCE_1},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_42), otm9608a_0x00_42},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCE_2), otm9608a_0xCE_2},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_43), otm9608a_0x00_43},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCE_3), otm9608a_0xCE_3},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_44), otm9608a_0x00_44},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCE_4), otm9608a_0xCE_4},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_45), otm9608a_0x00_45},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCE_5), otm9608a_0xCE_5},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_46), otm9608a_0x00_46},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCF_0), otm9608a_0xCF_0},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_47), otm9608a_0x00_47},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCF_1), otm9608a_0xCF_1},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_48), otm9608a_0x00_48},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCF_2), otm9608a_0xCF_2},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_49), otm9608a_0x00_49},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCF_3), otm9608a_0xCF_3},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_50), otm9608a_0x00_50},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xCF_4), otm9608a_0xCF_4},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_51), otm9608a_0x00_51},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xFF_2), otm9608a_0xFF_2},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_offset_01), otm9608a_offset_01},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_adpara_01), otm9608a_adpara_01},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_offset_02), otm9608a_offset_02},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_adpara_02), otm9608a_adpara_02},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_offset_03), otm9608a_offset_03},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_adpara_03), otm9608a_adpara_03},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_offset_04), otm9608a_offset_04},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_adpara_04), otm9608a_adpara_04},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_offset_05), otm9608a_offset_05},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_adpara_05), otm9608a_adpara_05},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_offset_06), otm9608a_offset_06},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_adpara_06), otm9608a_adpara_06},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_offset_07), otm9608a_offset_07},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_adpara_07), otm9608a_adpara_07},


	//{DTYPE_DCS_LWRITE, 1, 0, 0, 50, sizeof(cmd_2A_setting), cmd_2A_setting},
	//{DTYPE_DCS_LWRITE, 1, 0, 0, 50, sizeof(cmd_2B_setting), cmd_2B_setting},

	//{DTYPE_DCS_WRITE1, 1, 0, 0, OTM_CMD_DELAY, sizeof(cmd_pixel_format_setting), cmd_pixel_format_setting},

	{DTYPE_DCS_WRITE, 1, 0, 0, 120,            sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,            sizeof(display_on), display_on},
	//{DTYPE_DCS_WRITE1, 1, 0, 0, 50,            sizeof(write_memory_start), write_memory_start},
};

/*FOR READ IC ID*/
#if 0
static char manufacture_id[2] = {/*0x04*/0x04,0x00}; /* DTYPE_DCS_READ */ 
/*0xc8 can read out 4 parameters:50 78 00 01,but 0x04 can not be read out for unknow reason.*/ 

static struct dsi_cmd_desc novatek_manufacture_id_cmd = { 
DTYPE_DCS_READ/*DTYPE_DCS_READ*/, 1, 0, 1, 5, sizeof(manufacture_id), manufacture_id}; 


static char manufacture_id1[2] = {0xda, 0x00}; /* DTYPE_DCS_READ */ 
static struct dsi_cmd_desc novatek_manufacture_id_cmd1 = { 
DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(manufacture_id1), manufacture_id1}; 

static char manufacture_id2[2] = {0xdb, 0x00}; /* DTYPE_DCS_READ */ 
static struct dsi_cmd_desc novatek_manufacture_id_cmd2 = { 
DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(manufacture_id2), manufacture_id2}; 

static char manufacture_id3[2] = {0xdc, 0x00}; /* DTYPE_DCS_READ */ 
static struct dsi_cmd_desc novatek_manufacture_id_cmd3 = { 
DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(manufacture_id3), manufacture_id3}; 


static char manufacture_id4[2] = {0xd2, 0x00}; /* DTYPE_DCS_READ */ 
static struct dsi_cmd_desc novatek_manufacture_id_cmd4 = { 
DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(manufacture_id4), manufacture_id4}; 

static char manufacture_id5[2] = {0xd2, 0x02}; /* DTYPE_DCS_READ */ 
static struct dsi_cmd_desc novatek_manufacture_id_cmd5 = { 
DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(manufacture_id5), manufacture_id5}; 

static char manufacture_id6[2] = {0xd2, 0x03}; /* DTYPE_DCS_READ */ 
static struct dsi_cmd_desc novatek_manufacture_id_cmd6 = { 
DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(manufacture_id6), manufacture_id6}; 

static uint32 mipi_novatek_manufacture_id(struct msm_fb_data_type *mfd) 
{ 
	struct dsi_buf *rp, *tp; 
	struct dsi_cmd_desc *cmd; 
	uint32 *lp; 

	tp = &otm_tx_buf; 
	rp = &otm_rx_buf; 

	cmd = &novatek_manufacture_id_cmd; 
	mipi_dsi_cmds_rx(mfd, tp, rp, cmd, 3); 
	lp = (uint32 *)rp->data; 
	printk("%s: manufacture_id =%x \n", __func__, *lp); 


	cmd = &novatek_manufacture_id_cmd1; 
	mipi_dsi_cmds_rx(mfd, tp, rp, cmd, 1); 
	lp = (uint32 *)rp->data; 
	printk("%s: manufacture_id1=%x \n", __func__, 0xff&(*lp)); 

	cmd = &novatek_manufacture_id_cmd2; 
	mipi_dsi_cmds_rx(mfd, tp, rp, cmd, 1); 
	lp = (uint32 *)rp->data; 
	printk("%s: manufacture_id2=%x \n", __func__, 0xff&(*lp)); 

	cmd = &novatek_manufacture_id_cmd3; 
	mipi_dsi_cmds_rx(mfd, tp, rp, cmd, 1); 
	lp = (uint32 *)rp->data; 
	printk("%s: manufacture_id3=%x \n", __func__, 0xff&(*lp)); 
	
	//mipi_dsi_cmds_tx(mfd, &otm_tx_buf, txcmd,ARRAY_SIZE(txcmd));

	cmd = &novatek_manufacture_id_cmd4; 
	mipi_dsi_cmds_rx(mfd, tp, rp, cmd, 1); 
	lp = (uint32 *)rp->data; 
	printk("%s: manufacture_id_d200=%x \n", __func__, 0xff&(*lp)); 

	cmd = &novatek_manufacture_id_cmd5; 
	mipi_dsi_cmds_rx(mfd, tp, rp, cmd, 1); 
	lp = (uint32 *)rp->data; 
	printk("%s: manufacture_id_d202=%x \n", __func__, 0xff&(*lp)); 

	cmd = &novatek_manufacture_id_cmd6; 
	mipi_dsi_cmds_rx(mfd, tp, rp, cmd, 1); 
	lp = (uint32 *)rp->data; 
	printk("%s: manufacture_id_d203=%x \n", __func__, 0xff&(*lp)); 

	return *lp; 
} 
#endif


static int mipi_dnj_otm_lcd_reset(void)
{
    int rc = 0;

    gpio_set_value_cansleep(GPIO_OTM_LCD_RESET, 1);
    mdelay(5);
    gpio_set_value_cansleep(GPIO_OTM_LCD_RESET, 0);
    msleep(10);
    gpio_set_value_cansleep(GPIO_OTM_LCD_RESET, 1);
    msleep(120);

    return rc;
}


static int mipi_dnj_otm_lcd_on(struct platform_device *pdev)
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
	if (mipi_dnj_otm_lcd_reset() < 0) {
		pr_err("mipi_dnj_otm_lcd_reset error\n");
		return -EINVAL;
	} 	
	mipi_dsi_cmds_tx(mfd, &otm_tx_buf, otm_cmd_display_on_cmds,
		ARRAY_SIZE(otm_cmd_display_on_cmds));
				

/*FOR READ IC ID*/
#if 0
mipi_dsi_cmd_bta_sw_trigger();  /* clean up ack_err_status*/ 
mipi_novatek_manufacture_id(mfd);
#endif 
	return 0;
}

static int mipi_dnj_otm_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	mfd = platform_get_drvdata(pdev);
	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	mipi_dsi_cmds_tx(mfd, &otm_tx_buf, otm_display_off_cmds,
			ARRAY_SIZE(otm_display_off_cmds));
	gpio_set_value_cansleep(GPIO_OTM_LCD_RESET, 0);

	return 0;
}

static int __devinit mipi_dnj_otm9608a_lcd_probe(struct platform_device *pdev)
{
	int rc = 0;

	if (pdev->id == 0) {
		mipi_otm_pdata = pdev->dev.platform_data;
	   	if (mipi_otm_pdata) {
			gpio_backlight_en = mipi_otm_pdata->gpio;		
	   	}
	    	return rc;
	}
	msm_fb_add_device(pdev);

	return rc;
}

static struct platform_driver this_driver = {
	.probe  = mipi_dnj_otm9608a_lcd_probe,
	.driver = {
		.name   = "mipi_dnj_orise_otm9608a_qhd_pt",
	},
};

static void set_dnj_backlight_by_driver(struct msm_fb_data_type *mfd)
{
	int bl_level = mfd->bl_level;

	OTM_CUSTOMER_0X51[1] = bl_level;
 	mipi_dsi_mdp_busy_wait(mfd);
	mipi_dsi_cmds_tx(mfd, &otm_tx_buf, otm_cmd_drive_backlight_cmds,
			ARRAY_SIZE(otm_cmd_drive_backlight_cmds));


#if 1
	if(0 == bl_level) {
		//Start===Allen
		keyboard_led_light(KEY_LED_USER_LCD, 0);
		//End===Allen
	}
	else {
		//Start===Allen
		keyboard_led_light(KEY_LED_USER_LCD, 1);
		//End===Allen
	}
#endif

}

#define BL_LEVEL	255
static void set_dnj_backlight_by_host(struct msm_fb_data_type *mfd)
{
	//gpio_set_value_cansleep(gpio_backlight_en, 1);
#if 0

	int step;
	int i;
	int bl_level;
         step = 0;
	 i = 0;
	 bl_level = mfd->bl_level;
	/* real backlight level, 1 - max, 16 - min, 17 - off */
	bl_level = BL_LEVEL - bl_level;

	if (bl_level > prev_bl) {
		step = bl_level - prev_bl;
		if (bl_level == BL_LEVEL)
			step--;
	} else if (bl_level < prev_bl) {
		step = bl_level + 16 - prev_bl;
	} else {
		pr_debug("%s: no change\n", __func__);
		return;
	}
	if (bl_level == BL_LEVEL) {
		/* turn off backlight */
		mipi_otm_pdata->pmic_backlight(0);
	} else {
		if (prev_bl == BL_LEVEL) {
			/* turn on backlight */
			mipi_otm_pdata->pmic_backlight(1);
			udelay(30);
		}
		/* adjust backlight level */
		for (i = 0; i < step; i++) {
			mipi_otm_pdata->pmic_backlight(0);
			udelay(1);
			mipi_otm_pdata->pmic_backlight(1);
			udelay(1);
		}
	}
	msleep(20);
	prev_bl = bl_level;
printk("luke : %s    %d\n",__func__, __LINE__);
#endif
#if 1
        unsigned long flags;
	int step;
	int i;
	int bl_level;
         step = 0;
	 i = 0;
	 bl_level = mfd->bl_level;
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

		gpio_set_value_cansleep(gpio_backlight_en, 0);
		/*lilonghui add it for the keybaord led 2012-3-13*/
			mdelay(4);
		//gpio_set_value_cansleep(gpio_keybaord_en, 0);
			udelay(30);
	} else {
        	local_irq_save(flags);	
         
		if (prev_bl == (BACKLIGHT_BRIGHTNESS_LEVEL + 1)) {
			/* turn on backlight */	
			gpio_set_value_cansleep(gpio_backlight_en, 1);
			udelay(400);
			/*lilonghui add it for the keybaord led 2012-3-13*/
		       // gpio_set_value_cansleep(gpio_keybaord_en, 1);
		   	udelay(30);
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
	prev_bl = bl_level;    
#endif
	return;
}

static void mipi_dnj_otm_set_backlight(struct msm_fb_data_type *mfd)
{
	if(driver_by_host) {		
		set_dnj_backlight_by_host(mfd);
	}else{
		set_dnj_backlight_by_driver(mfd);
	}
}

static struct msm_fb_panel_data otm_panel_data = {
	.on  = mipi_dnj_otm_lcd_on,
	.off = mipi_dnj_otm_lcd_off,
	.set_backlight = mipi_dnj_otm_set_backlight,
};

static int ch_used[3];

int mipi_dnj_otm9608a_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;
	ch_used[channel] = TRUE;
	pdev = platform_device_alloc("mipi_dnj_orise_otm9608a_qhd_pt", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	otm_panel_data.panel_info = *pinfo;
	ret = platform_device_add_data(pdev, &otm_panel_data,
		sizeof(otm_panel_data));
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

static int mipi_otm9608a_qhd_lcd_late_init(void)
{	
	mipi_dsi_buf_alloc(&otm_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&otm_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

static int __init mipi_otm9608a_qhd_lcd_init(void)
{
  mipi_otm9608a_qhd_lcd_late_init();
  return 0;
}
module_init(mipi_otm9608a_qhd_lcd_init);


