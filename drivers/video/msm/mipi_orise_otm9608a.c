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
#include "mipi_orise_otm9608a.h"

#include <mach/pmic.h>
#include <linux/leds.h>  //add by hanxiaohui
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

/*common setting */
static char exit_sleep[2] = {0x11, 0x00};
static char display_on[2] = {0x29, 0x00};
static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};

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

static char cmd_pixel_format_setting[2] = {0x3A, 0x77};
static char cmd_2A_setting[5] = {0x2A, 0x00, 0x00,0x02,0x1B};
static char cmd_2B_setting[5] = {0x2B, 0x00, 0x00,0x03,0xBF};

static char otm9608a_0xFF_0[4] = {0xFF, 0x96, 0x08, 0x01}; //enable cmd

static char otm9608a_0x00_0[2] = {0x00, 0x80};
static char otm9608a_0xFF_1[3] = {0xFF, 0x96, 0x08};

#if 0
/*FOR ONE LANE*/
static char otm9608a_0x00_X1[2] = {0x00, 0x92};
static char otm9608a_0xFF_X2[2] = {0xFF, 0x00};

static char otm9608a_0x00_X3[2] = {0x00, 0x93};
static char otm9608a_0xFF_X4[2] = {0xFF, 0x02};
//=============
#endif

//OTP select region
static char otm9608a_0x00_1[2] = {0x00, 0x00};
static char otm9608a_0xA0_0[2] = {0xA0, 0x00};

//Command Set Option Parameter
static char otm9608a_0x00_2[2] = {0x00, 0x80};
static char otm9608a_0xB3_0[6] = {0xB3, 0x00, 0x00, 0x20, 0x00, 0x00};

//SRAM Setting
static char otm9608a_0x00_3[2] = {0x00, 0xC0};
static char otm9608a_0xB3_1[2] = {0xB3, 0x09};

//TCON Setting Parameters
static char otm9608a_0x00_4[2] = {0x00, 0x80};
static char otm9608a_0xC0_0[10]= {0xC0, 0x00, 0x48,0x00,0x10,0x10,0x00,0x48,0x10,0x10};

//Panel Timing Setting Parameter
static char otm9608a_0x00_5[2] = {0x00, 0x92};
static char otm9608a_0xC0_1[5] = {0xC0,0x00,0x10,0x00,0x13};

//Panel Timing Setting Parameter.
static char otm9608a_0x00_6[2] = {0x00, 0xA2};
static char otm9608a_0xC0_2[4] = {0xC0,0x0C,0x05,0x02};


//Interval Scan Frame Setting
static char otm9608a_0x00_7[2] = {0x00, 0xB3};
static char otm9608a_0xC0_3[3] = {0xC0,0x00,0x50};

 //Oscillator Adjustment for Idle/Normal Mode
static char otm9608a_0x00_8[2] = {0x00, 0x81};
static char otm9608a_0xC1[2]   = {0xC1,0x55};

static char otm9608a_0x00_9[2] = {0x00, 0x80};
static char otm9608a_0xC4_0[3] = {0xC4, 0x00, 0x84};

//DC2DC Setting
static char otm9608a_0x00_10[2] = {0x00, 0xA0};
static char otm9608a_0xC4_1[9]  = {0xC4,0x33,0x09,0x90,0x2B,0x33,0x09,0x90,0x54};

static char otm9608a_0x00_11[2] = {0x00, 0x80};
static char otm9608a_0xC5_0[5]  = {0xC5,0x08,0x00,0xA0,0x11};

//Power Control Setting2 for Normal Mode
static char otm9608a_0x00_12[2] = {0x00, 0x90};
static char otm9608a_0xC5_1[8]  = {0xC5,0x96,0x19,0x01,0x79,0x33,0x33,0x34};
				   
//Power Control Setting3 for Idle Mode
static char otm9608a_0x00_13[2] = {0x00, 0xA0};
static char otm9608a_0xC5_2[8]  = {0xC5,0x96,0x19,0x00,0x79,0x33,0x33,0x34};

//Power Control Setting3 for DC Voltage Settings
static char otm9608a_0x00_14[2] = {0x00, 0xB0};
static char otm9608a_0xC5_3[3]  = {0xC5,0x04,0xA8};

//Vcom setting
static char otm9608a_0x00_15[2] = {0x00, 0x00};
static char otm9608a_0xD8[3]    = {0xD8,0x6F,0x6F};

static char otm9608a_0x00_16[2] = {0x00, 0x00};
static char otm9608a_0xD9[2]    = {0xD9,0x46};

//ABC Parameter 1.
static char otm9608a_0x00_17[2] = {0x00, 0x80};
static char otm9608a_0xC6_0[2]  = {0xC6, 0x64};

//ABC Parameter 2.
static char otm9608a_0x00_18[2] = {0x00, 0xB0};
static char otm9608a_0xC6_1[6]  = {0xC6,0x03,0x10,0x00,0x1F,0x12};

//ID1
static char otm9608a_0x00_19[2] = {0x00, 0x00};
static char otm9608a_0xD0[2]    = {0xD0,0x40};

//ID2&ID3
static char otm9608a_0x00_20[2] = {0x00, 0x00};
static char otm9608a_0xD1[3]    = {0xD1, 0x00, 0x00};

static char otm9608a_0x00_21[2] = {0x00, 0xB7};
static char otm9608a_0xB0_0[2]  = {0xB0, 0x10};

static char otm9608a_0x00_22[2] = {0x00, 0xC0};
static char otm9608a_0xB0_1[2]  = {0xB0, 0x55};

static char otm9608a_0x00_23[2] = {0x00, 0xB1};
static char otm9608a_0xB0_2[2]  = {0xB0, 0x03};

//static char otm9608a_0x35_1[2] =  {0X35,0x00};     

//static char otm9608a_0x00_x[2] = {0x00, 0xA1};
//static char otm9608a_0xB0_x[2] = {0xB0, 0x02};

//gamma
static char otm9608a_0x00_24[2] = {0x00, 0x00};
static char otm9608a_0xE1[17] = {
	0xE1,0x08,0x15,0x1B,
	0x0E,0x07,0x10,0x0B,
	0x0A,0x03,0x07,0x0C,
	0x09,0x0F,0x0C,0x06,
	0x03
};

/*delay 10ms*/
static char otm9608a_0x00_25[2] = {0x00, 0x00};
static char otm9608a_0xE2[17] = {
	0xE2,0x08,0x15,0x1B,
	0x0E,0x07,0x10,0x0B,
	0x0A,0x03,0x07,0x0C,
	0x09,0x0F,0x0C,0x06,
	0x03
};

/*delay 10ms*/
static char otm9608a_0x00_26[2] = {0x00, 0x80};//TCON GOA WAVE(Panel timing state control)
static char otm9608a_0xCB_0[11] = {
	0xCB,0x05,0x00,0x00,
	0x00,0x00,0x05,0x00,
	0x00,0x00,0x00
};

static char otm9608a_0x00_27[2] = {0x00, 0x90};//TCON GOA WAVE(Panel timing state control)
static char otm9608a_0xCB_1[16] = {
	0xCB,0x55,0x55,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00
};

static char otm9608a_0x00_28[2] = {0x00, 0xA0};//TCON GOA WAVE(Panel timing state control)
static char otm9608a_0xCB_2[16] = {
	0xCB,0x00,0x00,0x00,
	0x00,0x00,0x55,0x55,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00
};

static char otm9608a_0x00_29[2] = {0x00, 0xB0};//TCON GOA WAVE(Panel timing state control)
static char otm9608a_0xCB_3[11] = {
	0xCB,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00
};

static char otm9608a_0x00_30[2] = {0x00, 0xC0};//TCON GOA WAVE(Panel timing state control)
static char otm9608a_0xCB_4[16] = {
	0xCB,0x55,0x55,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x04,0x00,
	0x04,0x04,0x00,0x00
};

static char otm9608a_0x00_31[2] = {0x00, 0xD0};
static char otm9608a_0xCB_5[16] = {
	0xCB,0x00,0x00,0x00,
	0x00,0x00,0x55,0x55,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x04
};

static char otm9608a_0x00_32[2] = {0x00, 0xE0};
static char otm9608a_0xCB_6[11] = {
	0xCB,0x00,0x04,0x04,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00
};
static char otm9608a_0x00_33[2] = {0x00, 0xF0};
static char otm9608a_0xCB_7[11] = {
	0xCB,0x0F,0x00,0xCC,
	0x00,0x00,0x0F,0x00,
	0xCC,0x03,0x00
};

static char otm9608a_0x00_34[2] = {0x00, 0x80};
static char otm9608a_0xCC_0[11] = {
	0xCC,0x25,0x26,0x00,
	0x00,0x00,0x00,0x00,
	0x0D,0x00,0x0A
};

static char otm9608a_0x00_35[2] = {0x00, 0x90};
static char otm9608a_0xCC_1[16] = {
	0xCC,0x00,0x0C,0x02,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x25,
	0x26,0x00,0x00,0x00
};

static char otm9608a_0x00_36[2] = {0x00, 0xA0};
static char otm9608a_0xCC_2[16] = {
	0xCC,0x00,0x00,0x00,
	0x00,0x09,0x00,0x0B,
	0x01,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00
};

static char otm9608a_0x00_37[2] = {0x00, 0xB0};
static char otm9608a_0xCC_3[11] = {
	0xCC,0x26,0x25,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x0B
};

static char otm9608a_0x00_38[2] = {0x00, 0xC0};
static char otm9608a_0xCC_4[16] = {
	0xCC,0x00,0x09,0x01,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x26,
	0x25,0x00,0x00,0x00
};

static char otm9608a_0x00_39[2] = {0x00, 0xD0};
static char otm9608a_0xCC_5[16] = {
	0xCC,0x00,0x00,0x00,
	0x00,0x0C,0x00,0x0A,
	0x02,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00
};

static char otm9608a_0x00_40[2] = {0x00, 0x80};
static char otm9608a_0xCE_0[13] = {
	0xCE,0x84,0x01,0x24,
	0x83,0x01,0x24,0x00,
	0x00,0x00,0x00,0x00,
	0x00
};

static char otm9608a_0x00_41[2] = {0x00, 0x90};
static char otm9608a_0xCE_1[15] = {
	0xCE,0xF0,0x00,0x00,
	0xF0,0x00,0x00,0xF0,
	0x00,0x00,0xF0,0x00,
	0x00,0x00,0x00
};

static char otm9608a_0x00_42[2] = {0x00, 0xA0};
static char otm9608a_0xCE_2[15] = {
	0xCE,0x18,0x02,0x03,
	0xC1,0x00,0x24,0x00,
	0x18,0x01,0x03,0xC2,
	0x00,0x24,0x00
};

static char otm9608a_0x00_43[2] = {0x00, 0xB0};
static char otm9608a_0xCE_3[15] = {
	0xCE,0x18,0x00,0x03,
	0xC3,0x00,0x24,0x00,
	0x10,0x00,0x03,0xC4,
	0x00,0x24,0x00
};

static char otm9608a_0x00_44[2] = {0x00, 0xC0};
static char otm9608a_0xCE_4[15] = {
	0xCE,0xF0,0x00,0x00,
	0x10,0x00,0x00,0x00,
	0xF0,0x00,0x00,0x10,
	0x00,0x00,0x00
};

static char otm9608a_0x00_45[2] = {0x00, 0xD0};
static char otm9608a_0xCE_5[15] = {
	0xCE,0xF0,0x00,0x00,
	0x10,0x00,0x00,0x00,
	0xF0,0x00,0x00,0x10,
	0x00,0x00,0x00
};

static char otm9608a_0x00_46[2] = {0x00, 0x80};
static char otm9608a_0xCF_0[15] = {
	0xCF,0xF0,0x00,0x00,
	0x10,0x00,0x00,0x00,
	0xF0,0x00,0x00,0x10,
	0x00,0x00,0x00
};

static char otm9608a_0x00_47[2] = {0x00, 0x90};
static char otm9608a_0xCF_1[15] = {
	0xCF,0xF0,0x00,0x00,
	0x10,0x00,0x00,0x00,
	0xF0,0x00,0x00,0x10,
	0x00,0x00,0x00
};

static char otm9608a_0x00_48[2] = {0x00, 0xA0};
static char otm9608a_0xCF_2[15] = {
	0xCF,0xF0,0x00,0x00,
	0x10,0x00,0x00,0x00,
	0xF0,0x00,0x00,0x10,
	0x00,0x00,0x00
};

static char otm9608a_0x00_49[2] = {0x00, 0xB0};
static char otm9608a_0xCF_3[15] = {
	0xCF,0xF0,0x00,0x00,
	0x10,0x00,0x00,0x00,
	0xF0,0x00,0x00,0x10,
	0x00,0x00,0x00
};

static char otm9608a_0x00_50[2] = {0x00, 0xC0};
static char otm9608a_0xCF_4[11] = {
	0xCF,0x01,0x01,0x20,
	0x20,0x00,0x00,0x02,
	0x01,0x00,0x00
};

static char otm9608a_0x00_51[2] = {0x00, 0x00};
static char otm9608a_0xFF_2[4] = {0xFF, 0xFF, 0xFF, 0xFF};



static struct dsi_cmd_desc otm_cmd_display_on_cmds[] = {

#if 1
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xFF_0), otm9608a_0xFF_0},

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0x00_0), otm9608a_0x00_0},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM_CMD_DELAY, sizeof(otm9608a_0xFF_1), otm9608a_0xFF_1},

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

	{DTYPE_DCS_LWRITE, 1, 0, 0, 50, sizeof(cmd_2A_setting), cmd_2A_setting},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 50, sizeof(cmd_2B_setting), cmd_2B_setting},

	{DTYPE_DCS_WRITE1, 1, 0, 0, OTM_CMD_DELAY, sizeof(cmd_pixel_format_setting), cmd_pixel_format_setting},

	{DTYPE_DCS_WRITE, 1, 0, 0, 200,            sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120,            sizeof(display_on), display_on},

	//{DTYPE_DCS_WRITE1, 1, 0, 0, 50,            sizeof(write_memory_start), write_memory_start},
#endif

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


static int mipi_otm_lcd_reset(void)
{
    int rc = 0;

    pr_debug("%s\n", __func__);

    gpio_set_value_cansleep(GPIO_OTM_LCD_RESET, 1);
    msleep(5);
    gpio_set_value_cansleep(GPIO_OTM_LCD_RESET, 0);
    msleep(10);
    gpio_set_value_cansleep(GPIO_OTM_LCD_RESET, 1);
    msleep(200);

    return rc;
}


static int mipi_otm_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mipi_panel_info *mipi;

	pr_debug("mipi_otm_lcd_on E\n");

	mfd = platform_get_drvdata(pdev);

	if (!mfd) {
		return -ENODEV;
	}

	if (mfd->key != MFD_KEY) {
		return -EINVAL;
	}

	mipi  = &mfd->panel_info.mipi;
	pr_debug("%s: mode = %d\n", __func__, mipi->mode);
	/* select DSI mode */
	//mipi_otm_sel_mode(mipi->mode);

	if (mipi_otm_lcd_reset() < 0) {
		pr_err("mipi_otm_lcd_reset error\n");
		return -EINVAL;
	}
printk("luke : %s    %d\n",__func__, __LINE__);
	mipi_dsi_cmds_tx(mfd, &otm_tx_buf, otm_cmd_display_on_cmds,
			ARRAY_SIZE(otm_cmd_display_on_cmds));

	mipi_dsi_cmds_tx(mfd, &otm_tx_buf, otm_cmd_drive_backlight_cmds,
			ARRAY_SIZE(otm_cmd_drive_backlight_cmds));
	pr_debug("mipi_otm_lcd_on X\n");

/*FOR READ IC ID*/
#if 0
mipi_dsi_cmd_bta_sw_trigger();  /* clean up ack_err_status*/ 
mipi_novatek_manufacture_id(mfd);
#endif 
	return 0;
}

static int mipi_otm_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
printk("luke : %s    %d\n",__func__, __LINE__);
	pr_debug("mipi_otm_lcd_off E\n");

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	mipi_dsi_cmds_tx(mfd, &otm_tx_buf, otm_display_off_cmds,
			ARRAY_SIZE(otm_display_off_cmds));

	gpio_set_value_cansleep(GPIO_OTM_LCD_RESET, 0);

	pr_debug("mipi_otm_lcd_off X\n");

	return 0;
}

static int __devinit mipi_orise_otm9608a_lcd_probe(struct platform_device *pdev)
{
	int rc = 0;
	pr_debug("%s\n", __func__);

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
	.probe  = mipi_orise_otm9608a_lcd_probe,
	.driver = {
		.name   = "mipi_orise_otm9608a_qhd_pt",
	},
};

static void set_backlight_by_driver(struct msm_fb_data_type *mfd)
{
	int bl_level = mfd->bl_level;
	OTM_CUSTOMER_0X51[1] = bl_level;
	printk("luke : %s    %d\n",__func__, __LINE__);
 	mipi_dsi_mdp_busy_wait(mfd);
	mipi_dsi_cmds_tx(mfd, &otm_tx_buf, otm_cmd_drive_backlight_cmds,
			ARRAY_SIZE(otm_cmd_drive_backlight_cmds));
	if(0 == bl_level) {
		//Start===Allen
		keyboard_led_light(KEY_LED_USER_LCD, 0);
		//End===Allen
	}else{
		//Start===Allen
		keyboard_led_light(KEY_LED_USER_LCD, 1);
		//End===Allen
	}
}

#define BL_LEVEL	255
static void set_backlight_by_host(struct msm_fb_data_type *mfd)
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
			msleep(4);
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
      /*end */
printk("luke : %s    %d\n",__func__, __LINE__);
#endif
	return;
}

static void mipi_otm_set_backlight(struct msm_fb_data_type *mfd)
{

	if(driver_by_host) {		
		set_backlight_by_host(mfd);
	}
	else {
		set_backlight_by_driver(mfd);
	}
}

static struct msm_fb_panel_data otm_panel_data = {
	.on	= mipi_otm_lcd_on,
	.off = mipi_otm_lcd_off,
	.set_backlight = mipi_otm_set_backlight,
};

static int ch_used[3];

int mipi_king_otm9608a_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

	pr_debug("mipi_otm_device_register\n");

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_orise_otm9608a_qhd_pt", (panel << 8)|channel);
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
	pr_debug("mipi_otm_lcd_late_init\n");
	
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
