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
#include "mipi_azet_otm8018b.h"
#include <linux/leds.h>  // add by hanxiaohui


//#define DRVIE_BACKLIGHT_BY_HOST
#define GPIO_AZET_LCD_RESET 85
/* common setting */
#define AZET_CMD_DELAY 0
#define AZET_SLEEP_OFF_DELAY 150
#define AZET_DISPLAY_ON_DELAY 0
#define BACKLIGHT_BRIGHTNESS_LEVEL 32


static struct msm_panel_common_pdata *mipi_azet8018b_pdata;
static struct dsi_buf azet8018b_tx_buf;
static struct dsi_buf azet8018b_rx_buf;

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

static char AZET_OTM8018B_CMD1[]  =  {0xff,0x80,0x09,0x01};
static char AZET_OTM8018B_CMD2[]  =  {0x00,0x80};
static char AZET_OTM8018B_CMD3[]  =  {0xff,0x80,0x09};
static char AZET_OTM8018B_CMD4[]  =  {0x00,0x80};
static char AZET_OTM8018B_CMD5[]  =  {0xf5,0x01,0x18,0x02,0x18,0x10,0x18,0x02,0x18,0x0e,0x18,0x0f,0x20};
static char AZET_OTM8018B_CMD6[]  =  {0x00,0x90};
static char AZET_OTM8018B_CMD7[]  =  {0xf5,0x02,0x18,0x08,0x18,0x06,0x18,0x0d,0x18,0x0b,0x18};
static char AZET_OTM8018B_CMD8[]  =  {0x00,0xa0};
static char AZET_OTM8018B_CMD9[]  =  {0xf5,0x10,0x18,0x01,0x18,0x14,0x18,0x14,0x18};

static char AZET_OTM8018B_CMD10[] =  {0x00,0xb0};
static char AZET_OTM8018B_CMD11[] =  {0xf5,0x14,0x18,0x12,0x18,0x13,0x18,0x11,0x18,0x13,0x18,0x00,0x00};
static char AZET_OTM8018B_CMD12[] =  {0x00,0x80};
static char AZET_OTM8018B_CMD13[] =  {0xc4,0x30};
static char AZET_OTM8018B_CMD14[] =  {0x00,0x80};
static char AZET_OTM8018B_CMD15[] =  {0xc0,0x00,0x5F,0x00,0x0A,0x0A,0x00};
static char AZET_OTM8018B_CMD16[] =  {0x00,0xb4};
static char AZET_OTM8018B_CMD17[] =  {0xc0,0x10};
static char AZET_OTM8018B_CMD18[] =  {0x00,0x82};
static char AZET_OTM8018B_CMD19[] =  {0xc5,0xa3};

static char AZET_OTM8018B_CMD20[] =  {0x00,0x90};
static char AZET_OTM8018B_CMD21[] =  {0xc5,0xD6,0xD6};
static char AZET_OTM8018B_CMD22[] =  {0x00,0x00};
static char AZET_OTM8018B_CMD23[] =  {0xd8,0x8F,0x8F};
static char AZET_OTM8018B_CMD24[] =  {0x00,0x00};
static char AZET_OTM8018B_CMD25[] =  {0xd9,0x56};  //56
static char AZET_OTM8018B_CMD26[] =  {0x00,0x81};
static char AZET_OTM8018B_CMD27[] =  {0xc1,0x55};
static char AZET_OTM8018B_CMD28[] =  {0x00,0xa1};
static char AZET_OTM8018B_CMD29[] =  {0xc1,0x08};

static char AZET_OTM8018B_CMD30[] =  {0x00,0xa3};
static char AZET_OTM8018B_CMD31[] =  {0xc0,0x1B};
static char AZET_OTM8018B_CMD32[] =  {0x00,0x81};
static char AZET_OTM8018B_CMD33[] =  {0xc4,0x83};
static char AZET_OTM8018B_CMD34[] =  {0x00,0x92};
static char AZET_OTM8018B_CMD35[] =  {0xc5,0x00};
static char AZET_OTM8018B_CMD36[] =  {0x00,0xb1};
static char AZET_OTM8018B_CMD37[] =  {0xc5,0xa9};
static char AZET_OTM8018B_CMD38[] =  {0x00,0x00};
static char AZET_OTM8018B_CMD39[] =  {0x00,0x00};
static char AZET_OTM8018B_CMD40[] =  {0x00,0x90};

static char AZET_OTM8018B_CMD41[] =  {0xc0,0x00,0x44,0x00,0x00,0x00,0x03};
static char AZET_OTM8018B_CMD42[] =  {0x00,0xa6};
static char AZET_OTM8018B_CMD43[] =  {0xc1,0x01,0x00,0x00};
static char AZET_OTM8018B_CMD44[] =  {0x00,0x80};
static char AZET_OTM8018B_CMD45[] =  {0xce,0x89,0x03,0x1c,0x88,0x03,0x1c};
static char AZET_OTM8018B_CMD46[] =  {0x00,0xa0};
static char AZET_OTM8018B_CMD47[] =  {0xce,0x38,0x07,0x03,0x24,0x00,0x1c,0x00,0x38,0x06,0x03,0x25,0x00,0x1c,0x00};
static char AZET_OTM8018B_CMD48[] =  {0x00,0xb0};
static char AZET_OTM8018B_CMD49[] =  {0xce,0x38,0x05,0x03,0x26,0x00,0x1c,0x00,0x38,0x04,0x03,0x28,0x00,0x1c,0x00};
static char AZET_OTM8018B_CMD50[] =  {0x00,0xc0};

static char AZET_OTM8018B_CMD51[] =  {0xce,0x38,0x03,0x03,0x20,0x00,0x1c,0x00,0x38,0x02,0x03,0x21,0x00,0x1c,0x00};
static char AZET_OTM8018B_CMD52[] =  {0x00,0xd0};
static char AZET_OTM8018B_CMD53[] =  {0xce,0x38,0x01,0x03,0x22,0x00,0x1c,0x00,0x38,0x00,0x03,0x23,0x00,0x1c,0x00};
static char AZET_OTM8018B_CMD54[] =  {0x00,0xc0};
static char AZET_OTM8018B_CMD55[] =  {0xcf,0x02,0x02,0x20,0x20,0x00,0x00,0x00,0x80,0x20,0x0A};
static char AZET_OTM8018B_CMD56[] =  {0x00,0xc1};
static char AZET_OTM8018B_CMD57[] =  {0xcb,0x54,0x54,0x54,0x54};
static char AZET_OTM8018B_CMD58[] =  {0x00,0xc6};
static char AZET_OTM8018B_CMD59[] =  {0xcb,0x54,0x54,0x54};

static char AZET_OTM8018B_CMD60[] =  {0x00,0xca};
static char AZET_OTM8018B_CMD61[] =  {0xcb,0x54};
static char AZET_OTM8018B_CMD62[] =  {0x00,0xd6};
static char AZET_OTM8018B_CMD63[] =  {0xcb,0x54,0x54,0x54,0x54};
static char AZET_OTM8018B_CMD64[] =  {0x00,0xdb};
static char AZET_OTM8018B_CMD65[] =  {0xcb,0x54,0x54,0x54};
static char AZET_OTM8018B_CMD66[] =  {0x00,0xe0};
static char AZET_OTM8018B_CMD67[] =  {0xcb,0x54};
static char AZET_OTM8018B_CMD68[] =  {0x00,0x81};
static char AZET_OTM8018B_CMD69[] =  {0xcc,0x10,0x0e,0x0c,0x0a};
static char AZET_OTM8018B_CMD70[] =  {0x00,0x86};

static char AZET_OTM8018B_CMD71[] =  {0xcc,0x02,0x00,0x21};
static char AZET_OTM8018B_CMD72[] =  {0x00,0x90};
static char AZET_OTM8018B_CMD73[] =  {0xcc,0x22};
static char AZET_OTM8018B_CMD74[] =  {0x00,0x9b};
static char AZET_OTM8018B_CMD75[] =  {0xcc,0x0f,0x0d,0x0b,0x09};
static char AZET_OTM8018B_CMD76[] =  {0x00,0xa1};
static char AZET_OTM8018B_CMD77[] =  {0xcc,0x01,0x00,0x21};
static char AZET_OTM8018B_CMD78[] =  {0x00,0xa5};
static char AZET_OTM8018B_CMD79[] =  {0xcc,0x22};
static char AZET_OTM8018B_CMD80[] =  {0x00,0xb1};

static char AZET_OTM8018B_CMD81[] =  {0xcc,0x09,0x0b,0x0d,0x0f};
static char AZET_OTM8018B_CMD82[] =  {0x00,0xb6};
static char AZET_OTM8018B_CMD83[] =  {0xcc,0x00,0x01,0x21};
static char AZET_OTM8018B_CMD84[] =  {0x00,0xc0};
static char AZET_OTM8018B_CMD85[] =  {0xcc,0x22};
static char AZET_OTM8018B_CMD86[] =  {0x00,0xcb};
static char AZET_OTM8018B_CMD87[] =  {0xcc,0x0a,0x0c,0x0e,0x10};
static char AZET_OTM8018B_CMD88[] =  {0x00,0xd1};
static char AZET_OTM8018B_CMD89[] =  {0xcc,0x00,0x02,0x21};
static char AZET_OTM8018B_CMD90[] =  {0x00,0xd5};

static char AZET_OTM8018B_CMD91[] =  {0xcc,0x22};
static char AZET_OTM8018B_CMD92[] =  {0x00,0x00};
static char AZET_OTM8018B_CMD93[] =  {0xe1,0x01,0x07,0x12,0x0F,0x09,0x1D,0x0F,0x0D,0x00,0x04,0x02,0x07,0x0F,0x24,0x20,0x0A};
static char AZET_OTM8018B_CMD94[] =  {0x00,0x00};
static char AZET_OTM8018B_CMD95[] =  {0xe2,0x01,0x07,0x12,0x0F,0x09,0x1D,0x0F,0x0D,0x00,0x04,0x02,0x07,0x0F,0x24,0x20,0x0A};
static char AZET_OTM8018B_CMD96[] =  {0x00,0x00};
static char AZET_OTM8018B_CMD97[] =  {0x35,0x00};
static char AZET_OTM8018B_CMD98[] =  {0x3a,0x77};
//static char AZET_OTM8018B_CMD99[] =  {0x21};
static char AZET_OTM8018B_CMD100[] =  {0x11};
static char AZET_OTM8018B_CMD101[] =  {0x29};
static char AZET_OTM8018B_CMD102[] =  {0x2c};

static struct dsi_cmd_desc azet8018b_video_display_on_cmds[] = {

	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD1), AZET_OTM8018B_CMD1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD2), AZET_OTM8018B_CMD2},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD3), AZET_OTM8018B_CMD3},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD4), AZET_OTM8018B_CMD4},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD5), AZET_OTM8018B_CMD5},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD6), AZET_OTM8018B_CMD6},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD7), AZET_OTM8018B_CMD7},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD8), AZET_OTM8018B_CMD8},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD9), AZET_OTM8018B_CMD9},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD10), AZET_OTM8018B_CMD10},

	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD11), AZET_OTM8018B_CMD11},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD12), AZET_OTM8018B_CMD12},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD13), AZET_OTM8018B_CMD13},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD14), AZET_OTM8018B_CMD14},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD15), AZET_OTM8018B_CMD15},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD16), AZET_OTM8018B_CMD16},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD17), AZET_OTM8018B_CMD17},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD18), AZET_OTM8018B_CMD18},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD19), AZET_OTM8018B_CMD19},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD20), AZET_OTM8018B_CMD20},

	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD21), AZET_OTM8018B_CMD21},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD22), AZET_OTM8018B_CMD22},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD23), AZET_OTM8018B_CMD23},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD24), AZET_OTM8018B_CMD24},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD25), AZET_OTM8018B_CMD25},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD26), AZET_OTM8018B_CMD26},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD27), AZET_OTM8018B_CMD27},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD28), AZET_OTM8018B_CMD28},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD29), AZET_OTM8018B_CMD29},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD30), AZET_OTM8018B_CMD30},

	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD31), AZET_OTM8018B_CMD31},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD32), AZET_OTM8018B_CMD32},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD33), AZET_OTM8018B_CMD33},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD34), AZET_OTM8018B_CMD34},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD35), AZET_OTM8018B_CMD35},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD36), AZET_OTM8018B_CMD36},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD37), AZET_OTM8018B_CMD37},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD38), AZET_OTM8018B_CMD38},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD39), AZET_OTM8018B_CMD39},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD40), AZET_OTM8018B_CMD40},

	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD41), AZET_OTM8018B_CMD41},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD42), AZET_OTM8018B_CMD42},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD43), AZET_OTM8018B_CMD43},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD44), AZET_OTM8018B_CMD44},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD45), AZET_OTM8018B_CMD45},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD46), AZET_OTM8018B_CMD46},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD47), AZET_OTM8018B_CMD47},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD48), AZET_OTM8018B_CMD48},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD49), AZET_OTM8018B_CMD49},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD50), AZET_OTM8018B_CMD50},

	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD51), AZET_OTM8018B_CMD51},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD52), AZET_OTM8018B_CMD52},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD53), AZET_OTM8018B_CMD53},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD54), AZET_OTM8018B_CMD54},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD55), AZET_OTM8018B_CMD55},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD56), AZET_OTM8018B_CMD56},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD57), AZET_OTM8018B_CMD57},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD58), AZET_OTM8018B_CMD58},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD59), AZET_OTM8018B_CMD59},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD60), AZET_OTM8018B_CMD60},

	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD61), AZET_OTM8018B_CMD61},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD62), AZET_OTM8018B_CMD62},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD63), AZET_OTM8018B_CMD63},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD64), AZET_OTM8018B_CMD64},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD65), AZET_OTM8018B_CMD65},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD66), AZET_OTM8018B_CMD66},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD67), AZET_OTM8018B_CMD67},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD68), AZET_OTM8018B_CMD68},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD69), AZET_OTM8018B_CMD69},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD70), AZET_OTM8018B_CMD70},

	
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD71), AZET_OTM8018B_CMD71},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD72), AZET_OTM8018B_CMD72},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD73), AZET_OTM8018B_CMD73},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD74), AZET_OTM8018B_CMD74},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD75), AZET_OTM8018B_CMD75},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD76), AZET_OTM8018B_CMD76},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD77), AZET_OTM8018B_CMD77},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD78), AZET_OTM8018B_CMD78},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD79), AZET_OTM8018B_CMD79},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD80), AZET_OTM8018B_CMD80},

	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD81), AZET_OTM8018B_CMD81},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD82), AZET_OTM8018B_CMD82},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD83), AZET_OTM8018B_CMD83},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD84), AZET_OTM8018B_CMD84},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD85), AZET_OTM8018B_CMD85},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD86), AZET_OTM8018B_CMD86},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD87), AZET_OTM8018B_CMD87},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD88), AZET_OTM8018B_CMD88},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD89), AZET_OTM8018B_CMD89},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD90), AZET_OTM8018B_CMD90},
	
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD91), AZET_OTM8018B_CMD91},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD92), AZET_OTM8018B_CMD92},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD93), AZET_OTM8018B_CMD93},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD94), AZET_OTM8018B_CMD94},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD95), AZET_OTM8018B_CMD95},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD96), AZET_OTM8018B_CMD96},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD97), AZET_OTM8018B_CMD97},//35
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD98), AZET_OTM8018B_CMD98},//3a
	//{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD99), AZET_OTM8018B_CMD99},//21
	{DTYPE_GEN_LWRITE, 1, 0, 0, 120, sizeof(AZET_OTM8018B_CMD100), AZET_OTM8018B_CMD100},//11

	{DTYPE_GEN_LWRITE, 1, 0, 0, 120, sizeof(AZET_OTM8018B_CMD101), AZET_OTM8018B_CMD101},//29
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(AZET_OTM8018B_CMD101), AZET_OTM8018B_CMD102},//2c
		
	

};

static struct dsi_cmd_desc tcl_cmd_drive_backlight_cmds[] = {
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(bl_0x51), bl_0x51},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(bl_0x53), bl_0x53},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(bl_0x55), bl_0x55},
}; 
#if 0
static struct dsi_cmd_desc azet8018b_display_off_cmds[] = {
	
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(display_off), display_off},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 100, sizeof(enter_sleep), enter_sleep},
};
#endif
static struct dsi_cmd_desc azet8018b_cmd_display_on_cmds[] = {

	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_SLEEP_OFF_DELAY, sizeof(exit_sleep), exit_sleep},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(display_on), display_on},
	{DTYPE_GEN_LWRITE, 1, 0, 0, AZET_CMD_DELAY, sizeof(write_memory_start), write_memory_start},
};

static int mipi_tcl_lcd_reset(void)
{
    int rc = 0;
    gpio_set_value_cansleep(GPIO_AZET_LCD_RESET, 1);
    mdelay(5);
    gpio_set_value_cansleep(GPIO_AZET_LCD_RESET, 0);
    msleep(10);
    gpio_set_value_cansleep(GPIO_AZET_LCD_RESET, 1);
    msleep(150);
	printk("mipi_tcl_lcd_reset  rc = %d\n", rc);
    return rc;
}


static bool  mipi_azet8018b_resume = false;
static int mipi_azet8018b_lcd_on(struct platform_device *pdev)
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
	
			mipi_dsi_cmds_tx(mfd, &azet8018b_tx_buf, azet8018b_video_display_on_cmds,
					ARRAY_SIZE(azet8018b_video_display_on_cmds));
	
		}else  if (mfd->panel_info.mipi.mode == DSI_CMD_MODE) {
				mipi_dsi_cmds_tx(mfd, &azet8018b_tx_buf,azet8018b_cmd_display_on_cmds,
					ARRAY_SIZE(azet8018b_cmd_display_on_cmds));
	
			}

	     mipi_azet8018b_resume = true;
	

printk("zhou : exit %s %d\n ", __func__,__LINE__);
	return 0;
}

static int mipi_azet8018b_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
printk("zhou : enter %s %d\n ", __func__,__LINE__);
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	
#if 0
		mipi_dsi_cmds_tx(mfd, &azet8018b_tx_buf, azet8018b_display_off_cmds,
				ARRAY_SIZE(azet8018b_display_off_cmds));
#endif
	gpio_set_value_cansleep(GPIO_AZET_LCD_RESET, 0);
	msleep(100);

	printk("zhou : exit %s %d\n ", __func__,__LINE__);
	return 0;
}

static int __devinit mipi_azet8018b_lcd_probe(struct platform_device *pdev)
{
	int rc = 0;
	pr_debug("%s\n", __func__);
printk("hxh: %s  %d \n", __func__, __LINE__	);
	if (pdev->id == 0) {
		mipi_azet8018b_pdata = pdev->dev.platform_data;
	    	if (mipi_azet8018b_pdata) {
			//gpio_backlight_en = mipi_azet8018b_pdata->gpio;	
			gpio_backlight_en = 96;
			printk("mipi_azet8018b_lcd_probe gpio_backlight_en %d\n", gpio_backlight_en);	
	    	}
		return 0;
	}

	rc = gpio_request(GPIO_AZET_LCD_RESET, "gpio_disp_reset");
	if(rc < 0) {
		pr_err("Error request gpio %d\n", GPIO_AZET_LCD_RESET);
	}

	rc = gpio_tlmm_config(GPIO_CFG(GPIO_AZET_LCD_RESET, 0,
				GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	
	if (rc < 0) {
		pr_err("Error config lcdc gpio:%d\n", GPIO_AZET_LCD_RESET);
	}

	rc = gpio_direction_output(GPIO_AZET_LCD_RESET, 1);
	if (rc < 0) {
		pr_err("Error direct lcdc gpio:%d\n", GPIO_AZET_LCD_RESET);
	}

	msm_fb_add_device(pdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_azet8018b_lcd_probe,
	.driver = {
		.name   = "mipi_azet_otm8018b",
	},
};

static void set_backlight_by_driver(struct msm_fb_data_type *mfd)
{
	int bl_level = mfd->bl_level;
	printk("hxh: %s  %d \n", __func__, __LINE__	);
	bl_0x51[1] = bl_level;
	printk("zhou : enter %s %d\n ", __func__,__LINE__);
     	if (mipi_azet8018b_resume){
		msleep(50);
		mipi_azet8018b_resume= false;
	}
	
	mipi_dsi_cmds_tx(mfd, &azet8018b_tx_buf, tcl_cmd_drive_backlight_cmds,
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
	printk("zhou : exit %s %d\n ", __func__,__LINE__);
	
}

static void set_backlight_by_host(struct msm_fb_data_type *mfd)
{
	int step = 0, i = 0;
    	unsigned long flags;
	int bl_level = (mfd->bl_level+1)/8;
	/* real backlight level, 32 - max, 1 - min, 0 - off */
	bl_0x51[1] = mfd->bl_level;
	bl_level = BACKLIGHT_BRIGHTNESS_LEVEL + 1 - bl_level;

	printk("bl_level = %d\n", bl_level);
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

	printk("step = %d\n", step);
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
			pr_debug("%s: turn on backlight\n", __func__);
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

	mipi_dsi_cmds_tx(mfd, &azet8018b_tx_buf, tcl_cmd_drive_backlight_cmds,
			    ARRAY_SIZE(tcl_cmd_drive_backlight_cmds));
	
	return;
}

#ifdef DRVIE_BACKLIGHT_BY_HOST
	static int driver_by_host = 1;
#else
	static int driver_by_host = 0;
#endif

static void  mipi_azet8018b_set_backlight(struct msm_fb_data_type *mfd)
{

	if(driver_by_host) {
		printk("set_backlight_by_host(mfd)\n");
		set_backlight_by_host(mfd);
	}
	else {
		printk("set_backlight_by_driver(mfd)\n");
		set_backlight_by_driver(mfd);
	}
}

static struct msm_fb_panel_data azet8018b_panel_data = {
	.on  = mipi_azet8018b_lcd_on,
	.off = mipi_azet8018b_lcd_off,
	.set_backlight = mipi_azet8018b_set_backlight,
};

static int ch_used[3];

int mipi_azet8018b_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

printk("hxh: %s  %d \n", __func__, __LINE__	);

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_azet_otm8018b", (panel << 8)|channel);

	if (!pdev)
		return -ENOMEM;

	azet8018b_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &azet8018b_panel_data,
		sizeof(azet8018b_panel_data));

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


static int __init mipi_azet8018b_lcd_init(void)
{
	mipi_dsi_buf_alloc(&azet8018b_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&azet8018b_rx_buf, DSI_BUF_SIZE);

	printk("__init mipi_azet8018b_lcd_init\n");
	return platform_driver_register(&this_driver);
}

module_init(mipi_azet8018b_lcd_init);

