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

#include "msm_sensor.h"
#include "msm.h"
#include "ov2655_v4l2.h"
#include <../../../../../../build/buildplus/target/QRDExt_target.h>
#define SENSOR_NAME "ov2655"
#define PLATFORM_DRIVER_NAME "msm_camera_ov2655"
#define ov2655_obj ov2655_##obj

#ifdef CDBG
#undef CDBG
#endif
#ifdef CDBG_HIGH
#undef CDBG_HIGH
#endif

#define OV2655_VERBOSE_DGB

#ifdef OV2655_VERBOSE_DGB
#define CDBG(fmt, args...) printk(fmt, ##args)
#define CDBG_HIGH(fmt, args...) printk(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#define CDBG_HIGH(fmt, args...) printk(fmt, ##args)
#endif

#define INVMASK(v)  (0xff-v)

static struct msm_sensor_ctrl_t ov2655_s_ctrl;
static int effect_value = CAMERA_EFFECT_OFF;
static unsigned int SAT_U = 0x80; /* DEFAULT SATURATION VALUES*/
static unsigned int SAT_V = 0x80; /* DEFAULT SATURATION VALUES*/

DEFINE_MUTEX(ov2655_mut);

static struct msm_camera_i2c_reg_conf ov2655_start_settings[] = {
};

static struct msm_camera_i2c_reg_conf ov2655_stop_settings[] = {
};

static struct msm_camera_i2c_reg_conf ov2655_prev_30fps_settings[] = {
	#if 0
	{0x3013, 0xf7},

	//add
	{0x3011, 0x00},
	{0x300e, 0x3a},


	{0x3010, 0x81},//82
	{0x3012, 0x10},
	{0x302a, 0x02},
	{0x302b, 0x6a},
	{0x306f, 0x14},
	{0x3362, 0x90},
	{0x3070, 0x5d},
	{0x3072, 0x5d},
	{0x301c, 0x07},
	{0x301d, 0x07},

	{0x3020, 0x01},
	{0x3021, 0x1a},
	{0x3022, 0x00},
	{0x3023, 0x06},
	{0x3024, 0x06},
	{0x3025, 0x58},
	{0x3026, 0x02},
	{0x3027, 0x61},
	{0x3088, 0x02},
	{0x3089, 0x80},
	{0x308A, 0x01},
	{0x308B, 0xe0},
	{0x3316, 0x64},
	{0x3317, 0x25},
	{0x3318, 0x80},
	{0x3319, 0x08},
	{0x331A, 0x28},
	{0x331B, 0x1e},
	{0x331C, 0x00},
	{0x331D, 0x38},
	{0x3302, 0x11},
	#else
	{0x3086, 0x0f},

	{0x3013, 0xf7},

	{0x3373, 0x50},
	{0x3376, 0x08},

	{0x3015, 0x22},//0x32
	{0x3014, 0x8c},//0x8c

	{0x3012, 0x10},
	{0x3016, 0x82},
	{0x3023, 0x06},
	{0x3026, 0x02},
	{0x3027, 0x5e},

	{0x330c, 0x00},
	{0x3301, 0xff},
	{0x3069, 0x80},
	{0x306f, 0x14},
	{0x3088, 0x03},
	{0x3089, 0x20},
	{0x308a, 0x02},
	{0x308b, 0x58},
	{0x308e, 0x00},
	{0x30a1, 0x41},
	{0x30a3, 0x80},
	{0x30d9, 0x95},
	{0x3302, 0x11},
	{0x3317, 0x25},
	{0x3318, 0x80},
	{0x3319, 0x08},
	{0x331d, 0x38},
	{0x3362, 0x90},
	{0x3302, 0x11},
	{0x3088, 0x02},
	{0x3089, 0x80},
	{0x308a, 0x01},
	{0x308b, 0xe0},
	{0x331a, 0x28},
	{0x331b, 0x1e},
	{0x331c, 0x00},
	{0x3302, 0x11},
	{0x363b, 0x01},
	{0x309e, 0x08},
	{0x3606, 0x00},
	{0x3630, 0x35},
	{0x304e, 0x04},
	{0x363b, 0x01},
	{0x309e, 0x08},
	{0x3606, 0x00},
	{0x3084, 0x01},
	{0x3634, 0x26},

	//banding
	{0x3011, 0x00},
	{0x300e, 0x3a},
	{0x3010, 0x81},

	{0x302a, 0x02},
	{0x302b, 0x6a},

	{0x3071, 0x00},
	{0x3070, 0x5d},
	{0x3073, 0x00},
	{0x3072, 0x4d},
	{0x301c, 0x05},
	{0x301d, 0x07},

	{0x3086, 0x00},
	#endif
};

static struct msm_camera_i2c_reg_conf ov2655_snap_settings[] = {
	#if 1
	//{0x3013, 0xf2},//f2

	{0x3011, 0x01},

	{0x3373, 0x40},
	{0x3376, 0x05},

	{0x3010, 0x81},
	{0x3012, 0x00},
	{0x302a, 0x04},
	{0x302b, 0xD4},
	{0x306f, 0x54},
	{0x3362, 0x80},
	{0x3070, 0x5d},
	{0x3072, 0x5d},
	{0x301c, 0x0f},
	{0x301d, 0x0f},
	{0x3020, 0x01},
	{0x3021, 0x18},
	{0x3022, 0x00},
	{0x3023, 0x0A},
	{0x3024, 0x06},
	{0x3025, 0x58},
	{0x3026, 0x04},
	{0x3027, 0xbc},
	{0x3088, 0x06},
	{0x3089, 0x40},
	{0x308A, 0x04},
	{0x308B, 0xB0},
	{0x3316, 0x64},
	{0x3317, 0x4B},
	{0x3318, 0x00},
	{0x3319, 0x2C},
	{0x331A, 0x64},
	{0x331B, 0x4B},
	{0x331C, 0x00},
	{0x331D, 0x4C},
	{0x3302, 0x01},
	#else
	{0x300e, 0x3a},//34
	{0x3011, 0x01},
	{0x3010, 0x81},
	{0x3012, 0x00},
	{0x3015, 0x04},
	{0x3016, 0xc2},
	{0x3023, 0x0c},
	{0x3026, 0x04},
	{0x3027, 0xbc},
	{0x302a, 0x04},
	{0x302b, 0xd4},
	{0x3069, 0x80},
	{0x306f, 0x54},
	{0x3088, 0x06},
	{0x3089, 0x40},
	{0x308a, 0x04},
	{0x308b, 0xb0},
	{0x308e, 0x64},
	{0x30a1, 0x41},
	{0x30a3, 0x80},
	{0x30d9, 0x95},
	{0x3071, 0x00},
	{0x3070, 0x5d},
	{0x3073, 0x00},
	{0x3072, 0x4d},
	{0x301c, 0x0c},
	{0x301d, 0x0f},
	{0x3302, 0x01},
	{0x3317, 0x4b},
	{0x3318, 0x00},
	{0x3319, 0x4c},
	{0x331d, 0x6c},
	{0x3362, 0x80},
	{0x3373, 0x40},
	{0x3376, 0x03},
	#endif
};

//set sensor init setting
static struct msm_camera_i2c_reg_conf ov2655_init_settings[] = {
	{0x308c, 0x80},
	{0x308d, 0x0e},
	{0x360b, 0x00},
	{0x30b0, 0xff},
	{0x30b1, 0xff},
	{0x30b2, 0x04},
	{0x300e, 0x34},
	{0x300f, 0xa6},
	{0x3010, 0x81},
	{0x3082, 0x01},
	{0x30f4, 0x01},
	{0x3090, 0x43},
	{0x3091, 0xc0},
	{0x30ac, 0x42},
	{0x30d1, 0x08},
	{0x30a8, 0x54},
	{0x3015, 0x04},
	{0x3093, 0x00},
	{0x307e, 0xe5},
	{0x3079, 0x00},
	{0x30aa, 0x52},
	{0x3017, 0x40},
	{0x30f3, 0x83},
	{0x306a, 0x0c},
	{0x306d, 0x00},
	{0x336a, 0x3c},
	{0x3076, 0x6a},
	{0x30d9, 0x95},
	{0x3016, 0x52},
	{0x3601, 0x30},
	{0x304e, 0x88},
	{0x30f1, 0x82},
	{0x306f, 0x14},
	{0x302a, 0x02},
	{0x302b, 0x6a},
	{0x3012, 0x10},
	{0x3011, 0x01},
	{0x30af, 0x00},
	{0x3048, 0x1f},
	{0x3049, 0x4e},
	{0x304a, 0x20},
	{0x304f, 0x20},
	{0x304b, 0x02},
	{0x304c, 0x00},
	{0x304d, 0x02},
	{0x304f, 0x20},
	{0x30a3, 0x10},
	{0x3013, 0xe7},
	{0x3014, 0x84},
	{0x3071, 0x00},
	{0x3070, 0x5d},
	{0x3073, 0x00},
	{0x3072, 0x4d},
	{0x301c, 0x07},
	{0x301d, 0x08},
	{0x304d, 0x42},
	{0x304a, 0x40},
	{0x304f, 0x40},
	{0x3095, 0x07},
	{0x3096, 0x16},
	{0x3097, 0x1d},
	{0x3020, 0x01},
	{0x3021, 0x18},
	{0x3022, 0x00},
	{0x3023, 0x06},
	{0x3024, 0x06},
	{0x3025, 0x58},
	{0x3026, 0x02},
	{0x3027, 0x61},
	{0x3088, 0x02},
	{0x3089, 0x80},
	{0x308a, 0x01},
	{0x308b, 0xe0},
	{0x3316, 0x64},
	{0x3317, 0x25},
	{0x3318, 0x80},
	{0x3319, 0x08},
	{0x331a, 0x28},
	{0x331b, 0x1e},
	{0x331c, 0x00},
	{0x331d, 0x38},
	{0x3100, 0x00},

	#if 0
	//awb
	{0x3320, 0xfa},
	{0x3321, 0x11},
	{0x3322, 0x92},
	{0x3323, 0x01},
	{0x3324, 0x97},
	{0x3325, 0x02},
	{0x3326, 0xff},
	{0x3327, 0x10},
	{0x3328, 0x10},
	{0x3329, 0x1f},
	{0x332a, 0x56},
	{0x332b, 0x54},
	{0x332c, 0xbe},
	{0x332d, 0xce},
	{0x332e, 0x2e},
	{0x332f, 0x30},
	{0x3330, 0x4d},
	{0x3331, 0x44},
	{0x3332, 0xf0},
	{0x3333, 0x0a},
	{0x3334, 0xf0},
	{0x3335, 0xf0},
	{0x3336, 0xf0},
	{0x3337, 0x40},
	{0x3338, 0x40},
	{0x3339, 0x40},
	{0x333a, 0x00},
	{0x333b, 0x00},
	//color matrix
	{0x3380, 0x20},
	{0x3381, 0x4b},
	{0x3382, 0x17},
	{0x3383, 0x15},
	{0x3384, 0x86},
	{0x3385, 0x9B},
	{0x3386, 0xa2},
	{0x3387, 0x9B},
	{0x3388, 0x07},
	{0x3389, 0x98},
	{0x338a, 0x01},
	//gamma
	{0x3340, 0x08},
	{0x3341, 0x13},
	{0x3342, 0x24},
	{0x3343, 0x35},
	{0x3344, 0x47},
	{0x3345, 0x56},
	{0x3346, 0x66},
	{0x3347, 0x74},
	{0x3348, 0x81},
	{0x3349, 0x96},
	{0x334a, 0xa7},
	{0x334b, 0xb6},
	{0x334c, 0xca},
	{0x334d, 0xdb},
	{0x334e, 0xe8},
	{0x334f, 0x20},
	#else
	//awb
	{0x3320, 0xfa},
	{0x3321, 0x11},
	{0x3322, 0x92},
	{0x3323, 0x01},
	{0x3324, 0x97},
	{0x3325, 0x02},
	{0x3326, 0xff},
	{0x3327, 0x10},
	{0x3328, 0x10}, 
	{0x3329, 0x1f},
	{0x332a, 0x56},
	{0x332b, 0x54},
	{0x332c, 0xbe},
	{0x332d, 0xce},
	{0x332e, 0x2e},
	{0x332f, 0x30},
	{0x3330, 0x4d},
	{0x3331, 0x44},
	{0x3332, 0xf0},
	{0x3333, 0x0a},
	{0x3334, 0xf0},
	{0x3335, 0xf0},
	{0x3336, 0xf0},
	{0x3337, 0x40},
	{0x3338, 0x40},
	{0x3339, 0x40},
	{0x333a, 0x00},
	{0x333b, 0x00}, 
	//color matrix
	{0x3380, 0x28}, 
	{0x3381, 0x48}, 
	{0x3382, 0x14}, 
	{0x3383, 0x17}, 
	{0x3384, 0x90}, 
	{0x3385, 0xa7}, 
	{0x3386, 0xbf}, 
	{0x3387, 0xb7}, 
	{0x3388, 0x08}, 
	{0x3389, 0x98}, 
	{0x338a, 0x01}, 
	//gamma
	{0x3340, 0x06},
	{0x3341, 0x0c},
	{0x3342, 0x19},
	{0x3343, 0x34},
	{0x3344, 0x4e},
	{0x3345, 0x5f},
	{0x3346, 0x6d},
	{0x3347, 0x78},
	{0x3348, 0x84},
	{0x3349, 0x95},
	{0x334a, 0xa5},
	{0x334b, 0xb4},
	{0x334c, 0xcc},
	{0x334d, 0xe2},
	{0x334e, 0xf6},
	{0x334f, 0x11},
	#endif

	//lens shading
	#if 1
	{0x3090, 0x03},
	{0x307c, 0x10},

	{0x3350, 0x32},
	{0x3351, 0x28},
	{0x3352, 0x00},
	{0x3353, 0x28},
	{0x3354, 0x00},
	{0x3355, 0x85},
	{0x3356, 0x32},
	{0x3357, 0x28},
	{0x3358, 0x00},
	{0x3359, 0x22},
	{0x335a, 0x00},
	{0x335b, 0x85},
	{0x335c, 0x32},
	{0x335d, 0x28},
	{0x335e, 0x00},
	{0x335f, 0x20},
	{0x3360, 0x00},
	{0x3361, 0x85},
	{0x3363, 0x70},
	{0x3364, 0x7f},
	{0x3365, 0x00},
	{0x3366, 0x00},
	#else //180
	{0x3090, 0x0b},
	{0x307c, 0x13},

	{0x3350, 0x32},
	{0x3351, 0x28},
	{0x3352, 0x00},
	{0x3353, 0x28},
	{0x3354, 0x00},
	{0x3355, 0x85},
	{0x3356, 0x32},
	{0x3357, 0x28},
	{0x3358, 0x00},
	{0x3359, 0x22},
	{0x335a, 0x00},
	{0x335b, 0x85},
	{0x335c, 0x32},
	{0x335d, 0x28},
	{0x335e, 0x00},
	{0x335f, 0x20},
	{0x3360, 0x00},
	{0x3361, 0x85},
	{0x3363, 0x70},
	{0x3364, 0x7f},
	{0x3365, 0x00},
	{0x3366, 0x00},
	#endif

	//uv adjust
	{0x3301, 0xff},
	{0x338B, 0x13},
	{0x338c, 0x10},
	{0x338d, 0x40},
	//sharpness&noise
	{0x3370, 0xd0},
	{0x3371, 0x00},
	{0x3372, 0x00},
	{0x3373, 0x50},
	{0x3374, 0x10},
	{0x3375, 0x10},
	{0x3376, 0x06},
	{0x3377, 0x00},
	{0x3378, 0x04},
	{0x3379, 0x80},

	{0x3069, 0x86},
	{0x3087, 0x02},

	{0x30a8, 0x54},
	{0x30aa, 0x82},
	{0x30a3, 0x91},
	{0x30a1, 0x41},

	{0x3300, 0xfc},
	{0x3302, 0x11},
	{0x3400, 0x00},
	{0x3606, 0x20},
	{0x3601, 0x30},
	{0x300e, 0x34},
	{0x30f3, 0x83},
	{0x304e, 0x88},

	{0x3391, 0x06},
	{0x3394, 0x40},//0x48
	{0x3395, 0x40},//0x48

	{0x3018, 0x78},//0x88
	{0x3019, 0x68},//0x78
	{0x301a, 0xa5},
	//kerr iq setting

	{0x3380,0x27}, //;cmx
	{0x3381,0x5c}, 
	{0x3382,0x0a}, 
	{0x3383,0x2b}, 
	{0x3384,0xb5}, 
	{0x3385,0xe1}, 
	{0x3386,0xd5}, 
	{0x3387,0xd2}, 
	{0x3388,0x03}, 
	{0x3389,0x98}, 
	{0x338a,0x01}, //;awb
	{0x3320,0xfa}, 
	{0x3321,0x11}, 
	{0x3322,0x92}, 
	{0x3323,0x01}, 
	{0x3324,0x97}, 
	{0x3325,0x02}, 
	{0x3326,0xff}, 
	{0x3327,0x10}, 
	{0x3328,0x11}, 
	{0x3329,0x1d}, 
	{0x332a,0x55}, 
	{0x332b,0x5a}, 
	{0x332c,0x90}, 
	{0x332d,0xc0}, 
	{0x332e,0x46}, 
	{0x332f,0x2f}, 
	{0x3330,0x2f}, 
	{0x3331,0x44}, 
	{0x3332,0xff}, 
	{0x3333,0x00}, 
	{0x3334,0xf0}, 
	{0x3335,0xf0}, 
	{0x3336,0xf0}, 
	{0x3337,0x40}, 
	{0x3338,0x40}, 
	{0x3339,0x40}, 
	{0x333a,0x00}, 
	{0x333b,0x00}, 
	{0x3300,0xFC}, //;lens normal correction
	{0x3350,0x33},
	{0x3351,0x29},
	{0x3352,0x88},
	{0x3353,0x21},
	{0x3354,0x00},
	{0x3355,0x85}, 
	{0x3356,0x34},
	{0x3357,0x29},
	{0x3358,0x88},
	{0x3359,0x1e},
	{0x335a,0x00},
	{0x335b,0x85}, 
	{0x335c,0x32},
	{0x335d,0x29},
	{0x335e,0x88},
	{0x335f,0x1b},
	{0x3360,0x00},
	{0x3361,0x85},
	{0x307c,0x10},
	{0x3090,0x33},
	{0x3363,0x70},
	{0x3364,0x7f},
	{0x3365,0x00},
	{0x3366,0x00}, 
	{0x334f,0x20},   //;gamma
	{0x3340,0x08},    
	{0x3341,0x16},    
	{0x3342,0x2f},    
	{0x3343,0x45},    
	{0x3344,0x56},    
	{0x3345,0x66},    
	{0x3346,0x72},    
	{0x3347,0x7c},    
	{0x3348,0x86},    
	{0x3349,0x96},    
	{0x334a,0xa3},    
	{0x334b,0xaf},    
	{0x334c,0xc4},    
	{0x334d,0xd7},    
	{0x334e,0xe8},    
	{0x3306,0x00},  
	{0x3376,0x06}, 
	{0x3377,0x00}, 
	{0x3378,0x04}, 
	{0x3379,0x40}, 
	{0x3373,0x50},
	{0x3303,0x00},
	{0x3308,0x00},
	{0x3069,0x80},
	{0x338b,0x12},

	
};

static struct msm_camera_i2c_conf_array ov2655_init_conf[] = {
	{&ov2655_init_settings[0],
	ARRAY_SIZE(ov2655_init_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_camera_i2c_conf_array ov2655_confs[] = {
	{&ov2655_snap_settings[0],
	ARRAY_SIZE(ov2655_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov2655_prev_30fps_settings[0],
	ARRAY_SIZE(ov2655_prev_30fps_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_camera_csi_params ov2655_csi_params = {
	.data_format = CSI_8BIT,
	.lane_cnt    = 1,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 0x6,
};

static struct v4l2_subdev_info ov2655_subdev_info[] = {
	{
	.code	= V4L2_MBUS_FMT_YUYV8_2X8,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt	= 1,
	.order	  = 0,
	}
	/* more can be supported, to be added later */
};

/*renwei add it for the ov2655 function at 2012-8-11*/
static struct msm_camera_i2c_reg_conf ov2655_saturation[][5] = {

	//{{0x5001, 0x80, INVMASK(0x80)},{0x5583, 0x10},{0x5584, 0x10},{0x5580, 0x02, INVMASK(0x02)}, {0x5588, 0x40, INVMASK(0x40)},},
	{{0x3301, 0x80, INVMASK(0x80)},{0x3391, 0x02, INVMASK(0x02)}, {0x3394, 0x00},{0x3395, 0x00},},//0
	{{0x3301, 0x80, INVMASK(0x80)},{0x3391, 0x02, INVMASK(0x02)}, {0x3394, 0x10},{0x3395, 0x10},},//1
	{{0x3301, 0x80, INVMASK(0x80)},{0x3391, 0x02, INVMASK(0x02)}, {0x3394, 0x20},{0x3395, 0x20},},//2
	{{0x3301, 0x80, INVMASK(0x80)}, {0x3391, 0x02, INVMASK(0x02)}, {0x3394, 0x30},{0x3395, 0x30},},//3
	{{0x3301, 0x80, INVMASK(0x80)}, {0x3391, 0x02, INVMASK(0x02)}, {0x3394, 0x40},{0x3395, 0x40},},//4
	{{0x3301, 0x80, INVMASK(0x80)}, {0x3391, 0x02, INVMASK(0x02)}, {0x3394, 0x40},{0x3395, 0x40},},//5
	{{0x3301, 0x80, INVMASK(0x80)}, {0x3391, 0x02, INVMASK(0x02)}, {0x3394, 0x50},{0x3395, 0x50},},//6
	{{0x3301, 0x80, INVMASK(0x80)}, {0x3391, 0x02, INVMASK(0x02)}, {0x3394, 0x60},{0x3395, 0x60},},//7
	{{0x3301, 0x80, INVMASK(0x80)},{0x3391, 0x02, INVMASK(0x02)}, {0x3394, 0x70},{0x3395, 0x70},},//8
	{{0x3301, 0x80, INVMASK(0x80)},{0x3391, 0x02, INVMASK(0x02)}, {0x3394, 0x80},{0x3395, 0x80},},//9
	{{0x3301, 0x80, INVMASK(0x80)}, {0x3391, 0x02, INVMASK(0x02)}, {0x3394, 0x90},{0x3395, 0x90},},//10

};
static struct msm_camera_i2c_conf_array ov2655_saturation_confs[][1] = {
	{{ov2655_saturation[0], ARRAY_SIZE(ov2655_saturation[0]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_saturation[1], ARRAY_SIZE(ov2655_saturation[1]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_saturation[2], ARRAY_SIZE(ov2655_saturation[2]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_saturation[3], ARRAY_SIZE(ov2655_saturation[3]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_saturation[4], ARRAY_SIZE(ov2655_saturation[4]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_saturation[5], ARRAY_SIZE(ov2655_saturation[5]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_saturation[6], ARRAY_SIZE(ov2655_saturation[6]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_saturation[7], ARRAY_SIZE(ov2655_saturation[7]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_saturation[8], ARRAY_SIZE(ov2655_saturation[8]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_saturation[9], ARRAY_SIZE(ov2655_saturation[9]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_saturation[10], ARRAY_SIZE(ov2655_saturation[10]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
};

static int ov2655_saturation_enum_map[] = {
	MSM_V4L2_SATURATION_L0,
	MSM_V4L2_SATURATION_L1,
	MSM_V4L2_SATURATION_L2,
	MSM_V4L2_SATURATION_L3,
	MSM_V4L2_SATURATION_L4,
	MSM_V4L2_SATURATION_L5,
	MSM_V4L2_SATURATION_L6,
	MSM_V4L2_SATURATION_L7,
	MSM_V4L2_SATURATION_L8,
	MSM_V4L2_SATURATION_L9,
	MSM_V4L2_SATURATION_L10,
};
/*add end*/
static struct msm_sensor_output_info_t ov2655_dimensions[] = {
	{ /* For SNAPSHOT */
		.x_output = 0x640,  /*1600*/  /*for 2Mp*/ /*0x640*/
		.y_output = 0x4B0,  /*1200*/ /*0x4B0*/
		.line_length_pclk = 0x793,
		.frame_length_lines = 0x4D4,
		.vt_pixel_clk = 42000000,
		.op_pixel_clk = 42000000,
		.binning_factor = 0x0,
	},
	{ /* For PREVIEW 30fps*/
		.x_output = 0x280,  /*640*/  /*for 2Mp*/
		.y_output = 0x1E0,  /*480*/
		.line_length_pclk = 0x793,
		.frame_length_lines = 0x26A,
		.vt_pixel_clk = 56000000,
		.op_pixel_clk = 56000000,
		.binning_factor = 0x0,
	},
};

/*renwei add it for the ov2655 effect at 2012-8-11*/
static struct msm_camera_i2c_enum_conf_array ov2655_saturation_enum_confs = {
	.conf = &ov2655_saturation_confs[0][0],
	.conf_enum = ov2655_saturation_enum_map,
	.num_enum = ARRAY_SIZE(ov2655_saturation_enum_map),
	.num_index = ARRAY_SIZE(ov2655_saturation_confs),
	.num_conf = ARRAY_SIZE(ov2655_saturation_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

static struct msm_camera_i2c_reg_conf ov2655_contrast[][4] = {
	{{0x3391, 0x04, INVMASK(0x04)},{0x3390, 0x45,INVMASK(0x04)},{0x3398, 0x12},{0x3399, 0x12},},//0
	{{0x3391, 0x04, INVMASK(0x04)},{0x3390, 0x45,INVMASK(0x04)},{0x3398, 0x14},{0x3399, 0x14},},//1
	{{0x3391, 0x04, INVMASK(0x04)},{0x3390, 0x45,INVMASK(0x04)},{0x3398, 0x18},{0x3399, 0x18},},//2
	{{0x3391, 0x04, INVMASK(0x04)},{0x3390, 0x45,INVMASK(0x04)},{0x3398, 0x1a},{0x3399, 0x1a},},//3
	{{0x3391, 0x04, INVMASK(0x04)},{0x3390, 0x45,INVMASK(0x04)},{0x3398, 0x1c},{0x3399, 0x1c},},//4

	{{0x3391, 0x04, INVMASK(0x04)},{0x3390, 0x41,INVMASK(0x04)},{0x3398, 0x20},{0x3399, 0x20},},//5
	{{0x3391, 0x04, INVMASK(0x04)},{0x3390, 0x45,INVMASK(0x04)},{0x3398, 0x24},{0x3399, 0x24},},//6
	{{0x3391, 0x04, INVMASK(0x04)},{0x3390, 0x45,INVMASK(0x04)},{0x3398, 0x28},{0x3399, 0x28},},//7
	{{0x3391, 0x04, INVMASK(0x04)},{0x3390, 0x45,INVMASK(0x04)},{0x3398, 0x2b},{0x3399, 0x2b},},//8
	{{0x3391, 0x04, INVMASK(0x04)},{0x3390, 0x45,INVMASK(0x04)},{0x3398, 0x30},{0x3399, 0x30},},//9
	{{0x3391, 0x04, INVMASK(0x04)},{0x3390, 0x41,INVMASK(0x04)},{0x3398, 0x34},{0x3399, 0x34},},//10

};

static struct msm_camera_i2c_conf_array ov2655_contrast_confs[][1] = {
	{{ov2655_contrast[0], ARRAY_SIZE(ov2655_contrast[0]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_contrast[1], ARRAY_SIZE(ov2655_contrast[1]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_contrast[2], ARRAY_SIZE(ov2655_contrast[2]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_contrast[3], ARRAY_SIZE(ov2655_contrast[3]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_contrast[4], ARRAY_SIZE(ov2655_contrast[4]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_contrast[5], ARRAY_SIZE(ov2655_contrast[5]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_contrast[6], ARRAY_SIZE(ov2655_contrast[6]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_contrast[7], ARRAY_SIZE(ov2655_contrast[7]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_contrast[8], ARRAY_SIZE(ov2655_contrast[8]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_contrast[9], ARRAY_SIZE(ov2655_contrast[9]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_contrast[10], ARRAY_SIZE(ov2655_contrast[10]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
};


static int ov2655_contrast_enum_map[] = {
	MSM_V4L2_CONTRAST_L0,
	MSM_V4L2_CONTRAST_L1,
	MSM_V4L2_CONTRAST_L2,
	MSM_V4L2_CONTRAST_L3,
	MSM_V4L2_CONTRAST_L4,
	MSM_V4L2_CONTRAST_L5,
	MSM_V4L2_CONTRAST_L6,
	MSM_V4L2_CONTRAST_L7,
	MSM_V4L2_CONTRAST_L8,
	MSM_V4L2_CONTRAST_L9,
	MSM_V4L2_CONTRAST_L10,
};

static struct msm_camera_i2c_enum_conf_array ov2655_contrast_enum_confs = {
	.conf = &ov2655_contrast_confs[0][0],
	.conf_enum = ov2655_contrast_enum_map,
	.num_enum = ARRAY_SIZE(ov2655_contrast_enum_map),
	.num_index = ARRAY_SIZE(ov2655_contrast_confs),
	.num_conf = ARRAY_SIZE(ov2655_contrast_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};
static struct msm_camera_i2c_reg_conf ov2655_sharpness[][5] = {
	{{0x3306, 0x00, INVMASK(0x08)},{0x3376, 0x01},{0x3377, 0x00},{0x3378, 0x10},{0x3379, 0x80},},	
	{{0x3306, 0x00, INVMASK(0x08)},{0x3376, 0x02},{0x3377, 0x00},{0x3378, 0x08},{0x3379, 0x80},},
	{{0x3306, 0x00, INVMASK(0x08)},{0x3376, 0x04}, {0x3377, 0x00},{0x3378, 0x04},{0x3379, 0x80},},
	{{0x3306, 0x00, INVMASK(0x08)},{0x3376, 0x0a},{0x3377, 0x00},{0x3378, 0x04},{0x3379, 0x80},},
	{{0x3306, 0x00, INVMASK(0x08)},{0x3376, 0x15},{0x3377, 0x00},{0x3378, 0x04},{0x3379, 0x80},},
	{{0x3306, 0x00, INVMASK(0x08)},{0x3376, 0x1c},{0x3377, 0x00},{0x3378, 0x04},{0x3379, 0x80},},
};

static struct msm_camera_i2c_conf_array ov2655_sharpness_confs[][1] = {
	{{ov2655_sharpness[0], ARRAY_SIZE(ov2655_sharpness[0]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_sharpness[1], ARRAY_SIZE(ov2655_sharpness[1]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_sharpness[2], ARRAY_SIZE(ov2655_sharpness[2]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_sharpness[3], ARRAY_SIZE(ov2655_sharpness[3]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_sharpness[4], ARRAY_SIZE(ov2655_sharpness[4]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_sharpness[5], ARRAY_SIZE(ov2655_sharpness[5]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
};

static int ov2655_sharpness_enum_map[] = {
	MSM_V4L2_SHARPNESS_L0,
	MSM_V4L2_SHARPNESS_L1,
	MSM_V4L2_SHARPNESS_L2,
	MSM_V4L2_SHARPNESS_L3,
	MSM_V4L2_SHARPNESS_L4,
	MSM_V4L2_SHARPNESS_L5,
};

static struct msm_camera_i2c_enum_conf_array ov2655_sharpness_enum_confs = {
	.conf = &ov2655_sharpness_confs[0][0],
	.conf_enum = ov2655_sharpness_enum_map,
	.num_enum = ARRAY_SIZE(ov2655_sharpness_enum_map),
	.num_index = ARRAY_SIZE(ov2655_sharpness_confs),
	.num_conf = ARRAY_SIZE(ov2655_sharpness_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

static struct msm_camera_i2c_reg_conf ov2655_exposure[][3] = {

	{{0x3391, 0x04, INVMASK(0x04)},{0x3390, 0x49}, {0x339a, 0x30},},
	{{0x3391, 0x04, INVMASK(0x04)},{0x3390, 0x49}, {0x339a, 0x10},},
	{{0x3391, 0x04, INVMASK(0x04)},{0x3390, 0x41}, {0x339a, 0x00},},
	{{0x3391, 0x04, INVMASK(0x04)},{0x3390, 0x41}, {0x339a, 0x10},},
	{{0x3391, 0x04, INVMASK(0x04)},{0x3390, 0x41}, {0x339a, 0x30},},
};

static struct msm_camera_i2c_conf_array ov2655_exposure_confs[][1] = {
	{{ov2655_exposure[0], ARRAY_SIZE(ov2655_exposure[0]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_exposure[1], ARRAY_SIZE(ov2655_exposure[1]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_exposure[2], ARRAY_SIZE(ov2655_exposure[2]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_exposure[3], ARRAY_SIZE(ov2655_exposure[3]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_exposure[4], ARRAY_SIZE(ov2655_exposure[4]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
};

static int ov2655_exposure_enum_map[] = {
	MSM_V4L2_EXPOSURE_N2,
	MSM_V4L2_EXPOSURE_N1,
	MSM_V4L2_EXPOSURE_D,
	MSM_V4L2_EXPOSURE_P1,
	MSM_V4L2_EXPOSURE_P2,
};

static struct msm_camera_i2c_enum_conf_array ov2655_exposure_enum_confs = {
	.conf = &ov2655_exposure_confs[0][0],
	.conf_enum = ov2655_exposure_enum_map,
	.num_enum = ARRAY_SIZE(ov2655_exposure_enum_map),
	.num_index = ARRAY_SIZE(ov2655_exposure_confs),
	.num_conf = ARRAY_SIZE(ov2655_exposure_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

static struct msm_camera_i2c_reg_conf ov2655_iso[][3] = {

	{{0x3015, 0x02, INVMASK(0x0f)},{-1, -1, -1},{-1, -1, -1},}, /*ISO_AUTO*/
	{{-1, -1, -1},{-1, -1, -1},{-1, -1, -1},},		  /*ISO_DEBLUR*/
	{{0x3015, 0x01, INVMASK(0x0f)},}, /*ISO_100*/
	//{{0x3391, 0x04, INVMASK(0x04)},{0x3390, 0x49}, {0x339a, 0x30},},
	{{0x3015, 0x02, INVMASK(0x0f)},{-1, -1, -1},{-1, -1, -1},}, /*ISO_200*/
	{{0x3015, 0x04, INVMASK(0x0f)},{-1, -1, -1},{-1, -1, -1},}, /*ISO_400*/
	{{0x3015, 0x05, INVMASK(0x0f)},{-1, -1, -1},{-1, -1, -1},}, /*ISO_800*/
	{{0x3015, 0x05, INVMASK(0x0f)},}, /*ISO_1600*/
	//{{0x3391, 0x04, INVMASK(0x04)},{0x3390, 0x41}, {0x339a, 0x30},},
};


static struct msm_camera_i2c_conf_array ov2655_iso_confs[][1] = {
	{{ov2655_iso[0], ARRAY_SIZE(ov2655_iso[0]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_iso[1], ARRAY_SIZE(ov2655_iso[1]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_iso[2], ARRAY_SIZE(ov2655_iso[2]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_iso[3], ARRAY_SIZE(ov2655_iso[3]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_iso[4], ARRAY_SIZE(ov2655_iso[4]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_iso[5], ARRAY_SIZE(ov2655_iso[5]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
};

static int ov2655_iso_enum_map[] = {
	MSM_V4L2_ISO_AUTO ,
	MSM_V4L2_ISO_DEBLUR,
	MSM_V4L2_ISO_100,
	MSM_V4L2_ISO_200,
	MSM_V4L2_ISO_400,
	MSM_V4L2_ISO_800,
	MSM_V4L2_ISO_1600,
};


static struct msm_camera_i2c_enum_conf_array ov2655_iso_enum_confs = {
	.conf = &ov2655_iso_confs[0][0],
	.conf_enum = ov2655_iso_enum_map,
	.num_enum = ARRAY_SIZE(ov2655_iso_enum_map),
	.num_index = ARRAY_SIZE(ov2655_iso_confs),
	.num_conf = ARRAY_SIZE(ov2655_iso_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

static struct msm_camera_i2c_reg_conf ov2655_no_effect[] = {
	{0x3391, 0x00},
	{-1, -1, -1},
	{-1, -1, -1},
};

static struct msm_camera_i2c_conf_array ov2655_no_effect_confs[] = {
	{&ov2655_no_effect[0],
	ARRAY_SIZE(ov2655_no_effect), 0,
	MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},
};

static struct msm_camera_i2c_reg_conf ov2655_special_effect[][3] = {
	{{0x3391, 0x00, INVMASK(0x58)},{-1, -1, -1},{-1, -1, -1},},
	{{0x3391, 0x18, INVMASK(0x58)},{0x3396, 0x80},{0x3397, 0x80},},/*mono*/
	{{0x3391, 0x40, INVMASK(0x58)},{-1, -1, -1},{-1, -1, -1},},/*negative */
	{{-1, -1, -1}, {-1, -1, -1},{-1, -1, -1},},
	{{0x3391, 0x18, INVMASK(0x58)},{0x3396, 0x40},{0x3397, 0xa6},},/*sepia*/
	{{-1, -1, -1}, {-1, -1, -1},{-1, -1, -1},},
	{{-1, -1, -1}, {-1, -1, -1},{-1, -1, -1},},
	{{-1, -1, -1}, {-1, -1, -1},{-1, -1, -1},},
	{{0x3391, 0x18, INVMASK(0x58)},{0x3396, 0x9e},{0x3397, 0x35},}, /*greenish*/
	{{-1, -1, -1}, {-1, -1, -1},{-1, -1, -1},},
	{{-1, -1, -1}, {-1, -1, -1},{-1, -1, -1},},
	{{-1, -1, -1}, {-1, -1, -1},{-1, -1, -1},},
	{{-1, -1, -1}, {-1, -1, -1},{-1, -1, -1},},
};

static struct msm_camera_i2c_conf_array ov2655_special_effect_confs[][1] = {
	{{ov2655_special_effect[0],  ARRAY_SIZE(ov2655_special_effect[0]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_special_effect[1],  ARRAY_SIZE(ov2655_special_effect[1]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_special_effect[2],  ARRAY_SIZE(ov2655_special_effect[2]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_special_effect[3],  ARRAY_SIZE(ov2655_special_effect[3]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_special_effect[4],  ARRAY_SIZE(ov2655_special_effect[4]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_special_effect[5],  ARRAY_SIZE(ov2655_special_effect[5]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_special_effect[6],  ARRAY_SIZE(ov2655_special_effect[6]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_special_effect[7],  ARRAY_SIZE(ov2655_special_effect[7]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_special_effect[8],  ARRAY_SIZE(ov2655_special_effect[8]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_special_effect[9],  ARRAY_SIZE(ov2655_special_effect[9]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_special_effect[10], ARRAY_SIZE(ov2655_special_effect[10]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_special_effect[11], ARRAY_SIZE(ov2655_special_effect[11]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_special_effect[12], ARRAY_SIZE(ov2655_special_effect[12]), 0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
};

static int ov2655_special_effect_enum_map[] = {
	MSM_V4L2_EFFECT_OFF,
	MSM_V4L2_EFFECT_MONO,
	MSM_V4L2_EFFECT_NEGATIVE,
	MSM_V4L2_EFFECT_SOLARIZE,
	MSM_V4L2_EFFECT_SEPIA,
	MSM_V4L2_EFFECT_POSTERAIZE,
	MSM_V4L2_EFFECT_WHITEBOARD,
	MSM_V4L2_EFFECT_BLACKBOARD,
	MSM_V4L2_EFFECT_AQUA,
	MSM_V4L2_EFFECT_EMBOSS,
	MSM_V4L2_EFFECT_SKETCH,
	MSM_V4L2_EFFECT_NEON,
	MSM_V4L2_EFFECT_MAX,
};

static struct msm_camera_i2c_enum_conf_array
		 ov2655_special_effect_enum_confs = {
	.conf = &ov2655_special_effect_confs[0][0],
	.conf_enum = ov2655_special_effect_enum_map,
	.num_enum = ARRAY_SIZE(ov2655_special_effect_enum_map),
	.num_index = ARRAY_SIZE(ov2655_special_effect_confs),
	.num_conf = ARRAY_SIZE(ov2655_special_effect_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

static struct msm_camera_i2c_reg_conf ov2655_antibanding[][7] = {
	{{0x3014, 0x00, INVMASK(0xc0)},{0x3070, 0x5c},{0x3071, 0x00},{0x3072, 0x4d},{0x3073, 0x00},{0x301c, 0x06},{0x301d, 0x07},}, /*ANTIBANDING 60HZ*/
	{{0x3014, 0x80, INVMASK(0xc0)},{0x3070, 0x5c},{0x3071, 0x00},{0x3072, 0x4d},{0x3073, 0x00},{0x301c, 0x06},{0x301d, 0x07},}, /*ANTIBANDING 50HZ*/
	{{0x3014, 0xc0, INVMASK(0xc0)},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1},}, /*ANTIBANDING AUTO*/
};


static struct msm_camera_i2c_conf_array ov2655_antibanding_confs[][1] = {
	{{ov2655_antibanding[0], ARRAY_SIZE(ov2655_antibanding[0]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_antibanding[1], ARRAY_SIZE(ov2655_antibanding[1]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_antibanding[2], ARRAY_SIZE(ov2655_antibanding[2]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
};

static int ov2655_antibanding_enum_map[] = {
	MSM_V4L2_POWER_LINE_60HZ,
	MSM_V4L2_POWER_LINE_50HZ,
	MSM_V4L2_POWER_LINE_AUTO,
};


static struct msm_camera_i2c_enum_conf_array ov2655_antibanding_enum_confs = {
	.conf = &ov2655_antibanding_confs[0][0],
	.conf_enum = ov2655_antibanding_enum_map,
	.num_enum = ARRAY_SIZE(ov2655_antibanding_enum_map),
	.num_index = ARRAY_SIZE(ov2655_antibanding_confs),
	.num_conf = ARRAY_SIZE(ov2655_antibanding_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};

static struct msm_camera_i2c_reg_conf ov2655_wb_oem[][4] = {
	{{-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1},
		{-1, -1, -1},},/*WHITEBALNACE OFF*/
	{{0x3306, 0x00, INVMASK(0x02)}, {-1, -1}, {-1, -1, -1},
		{-1, -1, -1},}, /*WHITEBALNACE AUTO*/
	{{0x3306, 0x02, INVMASK(0x02)}, {0x3337, 0x44}, {0x3338, 0x40},
		{0x3339, 0x70},},	/*WHITEBALNACE CUSTOM*/
	{{0x3306, 0x02, INVMASK(0x02)}, {0x3337, 0x52}, {0x3338, 0x40},
		{0x3339, 0x65},},	/*INCANDISCENT*/
	{{0x3306, 0x02, INVMASK(0x02)}, {0x3337, 0x50}, {0x3338, 0x40},
		{0x3339, 0x5a},},	/*FLOURESECT NOT SUPPORTED */
	{{0x3306, 0x02, INVMASK(0x02)}, {0x3337, 0x5e}, {0x3338, 0x40},
		{0x3339, 0x4b},},	/*DAYLIGHT*/
	{{0x3306, 0x02, INVMASK(0x02)}, {0x3337, 0x60}, {0x3338, 0x40},
		{0x3339, 0x3e},},	/*CLOUDY*/

};

static struct msm_camera_i2c_conf_array ov2655_wb_oem_confs[][1] = {
	{{ov2655_wb_oem[0], ARRAY_SIZE(ov2655_wb_oem[0]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_wb_oem[1], ARRAY_SIZE(ov2655_wb_oem[1]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_wb_oem[2], ARRAY_SIZE(ov2655_wb_oem[2]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_wb_oem[3], ARRAY_SIZE(ov2655_wb_oem[3]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_wb_oem[4], ARRAY_SIZE(ov2655_wb_oem[4]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_wb_oem[5], ARRAY_SIZE(ov2655_wb_oem[5]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
	{{ov2655_wb_oem[6], ARRAY_SIZE(ov2655_wb_oem[6]),  0,
		MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},},
};

static int ov2655_wb_oem_enum_map[] = {
	MSM_V4L2_WB_OFF,
	MSM_V4L2_WB_AUTO ,
	MSM_V4L2_WB_CUSTOM,
	MSM_V4L2_WB_INCANDESCENT,
	MSM_V4L2_WB_FLUORESCENT,
	MSM_V4L2_WB_DAYLIGHT,
	MSM_V4L2_WB_CLOUDY_DAYLIGHT,
};

static struct msm_camera_i2c_enum_conf_array ov2655_wb_oem_enum_confs = {
	.conf = &ov2655_wb_oem_confs[0][0],
	.conf_enum = ov2655_wb_oem_enum_map,
	.num_enum = ARRAY_SIZE(ov2655_wb_oem_enum_map),
	.num_index = ARRAY_SIZE(ov2655_wb_oem_confs),
	.num_conf = ARRAY_SIZE(ov2655_wb_oem_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};


int ov2655_saturation_msm_sensor_s_ctrl_by_enum(
		struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
         printk("renwei %s\n",__func__);
	if (effect_value == CAMERA_EFFECT_OFF) {
		rc = msm_sensor_write_enum_conf_array(
			s_ctrl->sensor_i2c_client,
			ctrl_info->enum_cfg_settings, value);
	}
	if (value <= MSM_V4L2_SATURATION_L8)
		SAT_U = SAT_V = value * 0x10;
	printk("--CAMERA-- %s ...(End)\n", __func__);
	return rc;
}


int ov2655_contrast_msm_sensor_s_ctrl_by_enum(
		struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
         printk("renwei %s\n",__func__);
	if (effect_value == CAMERA_EFFECT_OFF) {
		rc = msm_sensor_write_enum_conf_array(
			s_ctrl->sensor_i2c_client,
			ctrl_info->enum_cfg_settings, value);
	}
	return rc;
}

int ov2655_sharpness_msm_sensor_s_ctrl_by_enum(
		struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
      	 printk("renwei %s\n",__func__);
	if (effect_value == CAMERA_EFFECT_OFF) {
		rc = msm_sensor_write_enum_conf_array(
			s_ctrl->sensor_i2c_client,
			ctrl_info->enum_cfg_settings, value);
	}
	return rc;
}

int ov2655_effect_msm_sensor_s_ctrl_by_enum(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	effect_value = value;
	 printk("renwei %s\n",__func__);
	if (effect_value == CAMERA_EFFECT_OFF) {
		rc = msm_sensor_write_conf_array(
			s_ctrl->sensor_i2c_client,
			s_ctrl->msm_sensor_reg->no_effect_settings, 0);
		if (rc < 0) {
			CDBG("write faield\n");
			return rc;
		}
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0xda, SAT_U,
			MSM_CAMERA_I2C_BYTE_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0xdb, SAT_V,
			MSM_CAMERA_I2C_BYTE_DATA);
	} else {
		rc = msm_sensor_write_enum_conf_array(
			s_ctrl->sensor_i2c_client,
			ctrl_info->enum_cfg_settings, value);
	}
	return rc;
}

int ov2655_antibanding_msm_sensor_s_ctrl_by_enum(
		struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	 printk("renwei %s\n",__func__);
	if (effect_value == CAMERA_EFFECT_OFF) {
		rc = msm_sensor_write_enum_conf_array(
			s_ctrl->sensor_i2c_client,
			ctrl_info->enum_cfg_settings, value);
	}
	return rc;
}

int ov2655_msm_sensor_s_ctrl_by_enum(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	 printk("renwei %s\n",__func__);
	rc = msm_sensor_write_enum_conf_array(
		s_ctrl->sensor_i2c_client,
		ctrl_info->enum_cfg_settings, value);
	if (rc < 0) {
		CDBG("write faield\n");
		return rc;
	}
	return rc;
}

struct msm_sensor_v4l2_ctrl_info_t ov2655_v4l2_ctrl_info[] = {
	{
		.ctrl_id = V4L2_CID_SATURATION,
		.min = MSM_V4L2_SATURATION_L0,
		.max = MSM_V4L2_SATURATION_L8,
		.step = 1,
		.enum_cfg_settings = &ov2655_saturation_enum_confs,
		.s_v4l2_ctrl = ov2655_saturation_msm_sensor_s_ctrl_by_enum,
	},
	{
		.ctrl_id = V4L2_CID_CONTRAST,
		.min = MSM_V4L2_CONTRAST_L0,
		.max = MSM_V4L2_CONTRAST_L8,
		.step = 1,
		.enum_cfg_settings = &ov2655_contrast_enum_confs,
		.s_v4l2_ctrl = ov2655_contrast_msm_sensor_s_ctrl_by_enum,
	},
	{
		.ctrl_id = V4L2_CID_SHARPNESS,
		.min = MSM_V4L2_SHARPNESS_L0,
		.max = MSM_V4L2_SHARPNESS_L5,
		.step = 1,
		.enum_cfg_settings = &ov2655_sharpness_enum_confs,
		.s_v4l2_ctrl = ov2655_sharpness_msm_sensor_s_ctrl_by_enum,
	},
	{
		.ctrl_id = V4L2_CID_EXPOSURE,
		.min = MSM_V4L2_EXPOSURE_N2,
		.max = MSM_V4L2_EXPOSURE_P2,
		.step = 1,
		.enum_cfg_settings = &ov2655_exposure_enum_confs,
		.s_v4l2_ctrl = ov2655_msm_sensor_s_ctrl_by_enum,
	},
	{
		.ctrl_id = MSM_V4L2_PID_ISO,
		.min = MSM_V4L2_ISO_AUTO,
		.max = MSM_V4L2_ISO_1600,
		.step = 1,
		.enum_cfg_settings = &ov2655_iso_enum_confs,
		.s_v4l2_ctrl = ov2655_msm_sensor_s_ctrl_by_enum,
	},
	{
		.ctrl_id = V4L2_CID_SPECIAL_EFFECT,
		.min = MSM_V4L2_EFFECT_OFF,
		.max = MSM_V4L2_EFFECT_NEGATIVE,
		.step = 1,
		.enum_cfg_settings = &ov2655_special_effect_enum_confs,
		.s_v4l2_ctrl = ov2655_effect_msm_sensor_s_ctrl_by_enum,
	},
	{
		.ctrl_id = V4L2_CID_POWER_LINE_FREQUENCY,
		.min = MSM_V4L2_POWER_LINE_60HZ,
		.max = MSM_V4L2_POWER_LINE_AUTO,
		.step = 1,
		.enum_cfg_settings = &ov2655_antibanding_enum_confs,
		.s_v4l2_ctrl = ov2655_antibanding_msm_sensor_s_ctrl_by_enum,
	},
	{
		.ctrl_id = V4L2_CID_WHITE_BALANCE_TEMPERATURE,
		.min = MSM_V4L2_WB_OFF,
		.max = MSM_V4L2_WB_CLOUDY_DAYLIGHT,
		.step = 1,
		.enum_cfg_settings = &ov2655_wb_oem_enum_confs,
		.s_v4l2_ctrl = ov2655_msm_sensor_s_ctrl_by_enum,
	},

};

/*add end*/

static struct msm_sensor_output_reg_addr_t ov2655_reg_addr = {
	.x_output = 0x3808,
	.y_output = 0x380A,
	.line_length_pclk = 0x380C,
	.frame_length_lines = 0x380E,
};

static struct msm_camera_csi_params *ov2655_csi_params_array[] = {
	&ov2655_csi_params,
	&ov2655_csi_params,
};

static struct msm_sensor_id_info_t ov2655_id_info = {
	.sensor_id_reg_addr = 0x300a,
	.sensor_id = 0x2656,
};

static struct msm_sensor_exp_gain_info_t ov2655_exp_gain_info = {
	.coarse_int_time_addr = 0x3002,
	.global_gain_addr = 0x3000,
	.vert_offset = 4,
};

static int32_t ov2655_write_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{
	CDBG_HIGH("ov2655_write_exp_gain : Not supported\n");
	return 0;
}

int32_t ov2655_sensor_set_fps(struct msm_sensor_ctrl_t *s_ctrl,
		struct fps_cfg *fps)
{
	CDBG("ov2655_sensor_set_fps: Not supported\n");
	return 0;
}

static const struct i2c_device_id ov2655_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&ov2655_s_ctrl},
	{ }
};

int32_t ov2655_sensor_i2c_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int32_t rc = 0;
	struct msm_sensor_ctrl_t *s_ctrl;

	CDBG("%s IN\r\n", __func__);

	s_ctrl = (struct msm_sensor_ctrl_t *)(id->driver_data);

	rc = msm_sensor_i2c_probe(client, id);

	if (client->dev.platform_data == NULL) {
		CDBG_HIGH("%s: NULL sensor data\n", __func__);
		return -EFAULT;
	}

	usleep_range(5000, 5100);

	return rc;
}

static struct i2c_driver ov2655_i2c_driver = {
	.id_table = ov2655_i2c_id,
	.probe  = ov2655_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client ov2655_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&ov2655_i2c_driver);
}

static struct v4l2_subdev_core_ops ov2655_subdev_core_ops = {
	/*renwei add it for the ov2655 effect at 2012-8-11*/
	.s_ctrl = msm_sensor_v4l2_s_ctrl,
	.queryctrl = msm_sensor_v4l2_query_ctrl,
	/*add end*/
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops ov2655_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops ov2655_subdev_ops = {
	.core = &ov2655_subdev_core_ops,
	.video  = &ov2655_subdev_video_ops,
};
int32_t ov2655_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	struct msm_camera_sensor_info *info = NULL;
	info = s_ctrl->sensordata;

	CDBG("%s IN\r\n", __func__);
	CDBG("%s, sensor_pwd:%d, sensor_reset:%d\r\n",__func__, info->sensor_pwd, info->sensor_reset);
	gpio_direction_output(info->sensor_pwd, 1);
	gpio_direction_output(info->sensor_reset, 0);
	usleep_range(10000, 11000);
	if (info->pmic_gpio_enable) {
		lcd_camera_power_onoff(1);
	}
	usleep_range(10000, 11000);

	rc = msm_sensor_power_up(s_ctrl);
	if (rc < 0) {
		CDBG("%s: msm_sensor_power_up failed\n", __func__);
		return rc;
	}

	usleep_range(1000, 1100);
	/* turn on ldo and vreg */
	gpio_direction_output(info->sensor_pwd, 0);
	msleep(20);
	gpio_direction_output(info->sensor_reset, 1);
	msleep(25);

	return rc;

}

int32_t ov2655_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;

	struct msm_camera_sensor_info *info = NULL;

	CDBG("%s IN\r\n", __func__);
	info = s_ctrl->sensordata;

	msm_sensor_stop_stream(s_ctrl);
	msleep(40);

	gpio_direction_output(info->sensor_pwd, 1);
	usleep_range(5000, 5100);

	rc = msm_sensor_power_down(s_ctrl);
	msleep(40);
	if (s_ctrl->sensordata->pmic_gpio_enable){
		lcd_camera_power_onoff(0);
	}
	return rc;
}

void ov2655_set_shutter(struct msm_sensor_ctrl_t *s_ctrl)
{
	// write shutter, in number of line period
	unsigned short temp = 0;
	unsigned int shutter = 0, extra_line = 0;
	unsigned short ret_l,ret_h;

	ret_l = ret_h = 0;

	// turn off AEC & AGC
	msm_camera_i2c_read(s_ctrl->sensor_i2c_client,
			0x3013, &temp, MSM_CAMERA_I2C_BYTE_DATA);
	temp = temp & 0xfa;
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			0x3013, temp, MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_read(s_ctrl->sensor_i2c_client,
			0x3014, &temp, MSM_CAMERA_I2C_BYTE_DATA);
	temp = temp & 0xf7;
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			0x3014, temp, MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_read(s_ctrl->sensor_i2c_client,
			0x3002, &ret_h, MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_read(s_ctrl->sensor_i2c_client,
			0x3003, &ret_l, MSM_CAMERA_I2C_BYTE_DATA);
	shutter = (ret_h << 8) | (ret_l & 0xff) ;
	//shutter = shutter / 2;
	
	ret_l = ret_h = 0;
	msm_camera_i2c_read(s_ctrl->sensor_i2c_client,
			0x302d, &ret_h, MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_read(s_ctrl->sensor_i2c_client,
			0x302e, &ret_l, MSM_CAMERA_I2C_BYTE_DATA);
	extra_line = (ret_h << 8) | (ret_l & 0xff) ;
	//extra_line = extra_line / 2;

	shutter = (shutter + extra_line) / 2;
	if (shutter > 1236) {
		extra_line = shutter - 1236;
		shutter = 1236;
	} else {
		extra_line = 0;		
	}
	shutter = shutter & 0xffff;
	temp = shutter & 0xff;
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			0x3003, temp, MSM_CAMERA_I2C_BYTE_DATA);
	temp = shutter >> 8;
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			0x3002, temp, MSM_CAMERA_I2C_BYTE_DATA);

	extra_line = extra_line & 0xffff;
	temp = extra_line & 0xff;
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			0x302e, temp, MSM_CAMERA_I2C_BYTE_DATA);
	temp = extra_line >> 8;
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			0x302d, temp, MSM_CAMERA_I2C_BYTE_DATA);
#if 0
#if(CONFIG_Q203)
       /*lilonghui add it for the fake flash led CTA 2012-9-26*/
        gpio_direction_output(34, 1);
        msleep(30);
        gpio_direction_output(34, 0);
        msleep(1);
      /*end*/
      CDBG_HIGH("%s:lilonghui call;  : Not supported\n" ,__func__);
#endif
#endif
}

int32_t ov2655_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;
	static int csi_config;

	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	msleep(30);
	if (update_type == MSM_SENSOR_REG_INIT) {
		CDBG("Register INIT\n");
		s_ctrl->curr_csi_params = NULL;
		msm_sensor_enable_debugfs(s_ctrl);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			0x3012, 0x80, MSM_CAMERA_I2C_BYTE_DATA);
		msleep(5);
		msm_sensor_write_init_settings(s_ctrl);
		csi_config = 0;
	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
		CDBG("PERIODIC : %d\n", res);
		if (!csi_config) {
			s_ctrl->curr_csic_params = s_ctrl->csic_params[res];
			CDBG("CSI config in progress\n");
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_CSIC_CFG,
				s_ctrl->curr_csic_params);
			CDBG("CSI config is done\n");
			mb();
			msleep(50);
			csi_config = 1;
		}

		if (res == MSM_SENSOR_RES_FULL)
			ov2655_set_shutter(s_ctrl);
		
		msm_sensor_write_conf_array(
			s_ctrl->sensor_i2c_client,
			s_ctrl->msm_sensor_reg->mode_settings, res);
		if (res == MSM_SENSOR_RES_QTR)
		{
			msleep(300);
		}
		else if (res == MSM_SENSOR_RES_FULL)
		{
			msleep(50);
		}
		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
			NOTIFY_PCLK_CHANGE,
			&s_ctrl->sensordata->pdata->ioclk.vfe_clk_rate);

		s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
		msleep(50);
	}
	return rc;
}

static struct msm_sensor_fn_t ov2655_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = ov2655_sensor_set_fps,

	.sensor_write_exp_gain = ov2655_write_exp_gain,
	.sensor_write_snapshot_exp_gain = ov2655_write_exp_gain,

	.sensor_csi_setting = ov2655_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = ov2655_sensor_power_up,
	.sensor_power_down = ov2655_sensor_power_down,
};

static struct msm_sensor_reg_t ov2655_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = ov2655_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(ov2655_start_settings),
	.stop_stream_conf = ov2655_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(ov2655_stop_settings),
	.group_hold_on_conf = NULL,
	.group_hold_on_conf_size = 0,
	.group_hold_off_conf = NULL,
	.group_hold_off_conf_size = 0,
	.init_settings = &ov2655_init_conf[0],
	.init_size = ARRAY_SIZE(ov2655_init_conf),
	.mode_settings = &ov2655_confs[0],
	.no_effect_settings = &ov2655_no_effect_confs[0],
	.output_settings = &ov2655_dimensions[0],
	.num_conf = ARRAY_SIZE(ov2655_confs),
};

static struct msm_sensor_ctrl_t ov2655_s_ctrl = {
	.msm_sensor_reg = &ov2655_regs,
	.msm_sensor_v4l2_ctrl_info = ov2655_v4l2_ctrl_info,
	.num_v4l2_ctrl = ARRAY_SIZE(ov2655_v4l2_ctrl_info),
	.sensor_i2c_client = &ov2655_sensor_i2c_client,
	.sensor_i2c_addr =  0x60,
	.sensor_output_reg_addr = &ov2655_reg_addr,
	.sensor_id_info = &ov2655_id_info,
	.sensor_exp_gain_info = &ov2655_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &ov2655_csi_params_array[0],
	.msm_sensor_mutex = &ov2655_mut,
	.sensor_i2c_driver = &ov2655_i2c_driver,
	.sensor_v4l2_subdev_info = ov2655_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(ov2655_subdev_info),
	.sensor_v4l2_subdev_ops = &ov2655_subdev_ops,
	.func_tbl = &ov2655_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Omnivision WXGA YUV sensor driver");
MODULE_LICENSE("GPL v2");
