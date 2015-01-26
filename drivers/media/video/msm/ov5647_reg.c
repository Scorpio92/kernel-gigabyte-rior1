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


#include "ov5647.h"
struct ov5647_i2c_reg_conf ov5647_prev_settings[] = {
	//1280*960 Reference Setting 24M MCLK 2lane 280Mbps/lane 30fps
	//for back to preview
	{0x3035, 0x21},
	{0x3036, 0x37},
	{0x3821, 0x07},
	{0x3820, 0x41},
	{0x3612, 0x09},
	{0x3618, 0x00},
	{0x380c, 0x07},
	{0x380d, 0x68},
	{0x380e, 0x03},
	{0x380f, 0xd8},
	{0x3814, 0x31},
	{0x3815, 0x31},
	{0x3709, 0x52},
	{0x3808, 0x05},
	{0x3809, 0x00},
	{0x380a, 0x03},
	{0x380b, 0xc0},
	{0x3800, 0x00},
	{0x3801, 0x18},
	{0x3802, 0x00},
	{0x3803, 0x0e},
	{0x3804, 0x0a},
	{0x3805, 0x27},
	{0x3806, 0x07},
	{0x3807, 0x95},
	//{0x3a09, 0x27},
	//{0x3a0a, 0x00},
	//{0x3a0b, 0xf6},
	//{0x3a0d, 0x04},
	//{0x3a0e, 0x03},
	{0x4004, 0x02},
};

struct ov5647_i2c_reg_conf ov5647_snap_settings[] = {
#if 1
	//2608*1952 Reference Setting 24M MCLK 2lane 280Mbps/lane 30fps
	{0x3035, 0x21},
	{0x3036, 0x4f},
	{0x3821, 0x06},
	{0x3820, 0x00},
	{0x3612, 0x0b},
	{0x3618, 0x04},
	{0x380c, 0x0a},
	{0x380d, 0x8c},
	{0x380e, 0x07},
	{0x380f, 0xb0},
	{0x3814, 0x11},
	{0x3815, 0x11},
	{0x3709, 0x12},
	{0x3808, 0x0a},
	{0x3809, 0x30},
	{0x380a, 0x07},
	{0x380b, 0xa0},
	{0x3800, 0x00},
	{0x3801, 0x04},
	{0x3802, 0x00},
	{0x3803, 0x00},
	{0x3804, 0x0a},
	{0x3805, 0x3b},
	{0x3806, 0x07},
	{0x3807, 0xa3},
//	{0x3a09, 0x28},
//	{0x3a0a, 0x00},
//	{0x3a0b, 0xf6},
//	{0x3a0d, 0x08},
//	{0x3a0e, 0x06},
	{0x4004, 0x04},
#else
	//2592*1944 Reference Setting 24M MCLK 2lane 400Mbps/lane 15fps
	//for full size capture
	{0x3035, 0x21},
	{0x3036, 0x64},
	{0x3821, 0x06},
	{0x3820, 0x00},
	{0x3612, 0x0b},
	{0x3618, 0x04},
	{0x380c, 0x0a},
	{0x380d, 0x8c},
	{0x380e, 0x07},
	{0x380f, 0xb6},
	{0x3814, 0x11},
	{0x3815, 0x11},
	{0x3709, 0x12},
	{0x3808, 0x0a},
	{0x3809, 0x20},
	{0x380a, 0x07},
	{0x380b, 0x98},
	{0x3800, 0x00},
	{0x3801, 0x0c},
	{0x3802, 0x00},
	{0x3803, 0x04},
	{0x3804, 0x0a},
	{0x3805, 0x33},
	{0x3806, 0x07},
	{0x3807, 0xa3},
	{0x3a09, 0x28},
	{0x3a0a, 0x00},
	{0x3a0b, 0xf6},
	{0x3a0d, 0x08},
	{0x3a0e, 0x06},
	{0x4004, 0x04},
#endif
};

struct ov5647_i2c_reg_conf ov5647_recommend_settings[] = {
	{0x3035, 0x11},
	{0x303c, 0x11},
	{0x370c, 0x03},
	{0x5000, 0x06},
	{0x5003, 0x08},
	{0x5a00, 0x08},
	{0x3000, 0xff},
	{0x3001, 0xff},
	{0x3002, 0xff},
	{0x301d, 0xf0},
	{0x3a18, 0x00},
	{0x3a19, 0xf8},
	{0x3c01, 0x80},
	{0x3b07, 0x0c},
	{0x3708, 0x64},
	{0x3630, 0x2e},
	{0x3632, 0xe2},
	{0x3633, 0x23},
	{0x3634, 0x44},
	{0x3620, 0x64},
	{0x3621, 0xe0},
	{0x3600, 0x37},
	{0x3704, 0xa0},
	{0x3703, 0x5a},
	{0x3715, 0x78},
	{0x3717, 0x01},
	{0x3731, 0x02},
	{0x370b, 0x60},
	{0x3705, 0x1a},
	{0x3f05, 0x02},
	{0x3f06, 0x10},
	{0x3f01, 0x0a},
	{0x3a08, 0x01},
	{0x3a0f, 0x58},
	{0x3a10, 0x50},
	{0x3a1b, 0x58},
	{0x3a1e, 0x50},
	{0x3a11, 0x60},
	{0x3a1f, 0x28},
	{0x4001, 0x02},
	{0x4000, 0x09},
	{0x3000, 0x00},
	{0x3001, 0x00},
	{0x3002, 0x00},
	{0x3017, 0xe0},
	{0x301c, 0xfc},
	{0x3636, 0x06},
	{0x3016, 0x08},
	{0x3827, 0xec},
	{0x3018, 0x44},
	{0x3035, 0x21},
	{0x3106, 0xf5},
	{0x3034, 0x18},
	{0x301c, 0xf8},
	//lens setting
	{0x5000, 0x86},
	{0x5800, 0x11},
	{0x5801, 0xc },
	{0x5802, 0xa },
	{0x5803, 0xb },
	{0x5804, 0xd },
	{0x5805, 0x13},
	{0x5806, 0x9 },
	{0x5807, 0x5 },
	{0x5808, 0x3 },
	{0x5809, 0x3 },
	{0x580a, 0x6 },
	{0x580b, 0x8 },
	{0x580c, 0x5 },
	{0x580d, 0x1 },
	{0x580e, 0x0 },
	{0x580f, 0x0 },
	{0x5810, 0x2 },
	{0x5811, 0x6 },
	{0x5812, 0x5 },
	{0x5813, 0x1 },
	{0x5814, 0x0 },
	{0x5815, 0x0 },
	{0x5816, 0x2 },
	{0x5817, 0x6 },
	{0x5818, 0x9 },
	{0x5819, 0x5 },
	{0x581a, 0x4 },
	{0x581b, 0x4 },
	{0x581c, 0x6 },
	{0x581d, 0x9 },
	{0x581e, 0x11},
	{0x581f, 0xc },
	{0x5820, 0xb },
	{0x5821, 0xb },
	{0x5822, 0xd },
	{0x5823, 0x13},
	{0x5824, 0x22},
	{0x5825, 0x26},
	{0x5826, 0x26},
	{0x5827, 0x24},
	{0x5828, 0x24},
	{0x5829, 0x24},
	{0x582a, 0x22},
	{0x582b, 0x20},
	{0x582c, 0x22},
	{0x582d, 0x26},
	{0x582e, 0x22},
	{0x582f, 0x22},
	{0x5830, 0x42},
	{0x5831, 0x22},
	{0x5832, 0x2 },
	{0x5833, 0x24},
	{0x5834, 0x22},
	{0x5835, 0x22},
	{0x5836, 0x22},
	{0x5837, 0x26},
	{0x5838, 0x42},
	{0x5839, 0x26},
	{0x583a, 0x6 },
	{0x583b, 0x26},
	{0x583c, 0x24},
	{0x583d, 0xce},
	//manual AWB,manual AE,close Lenc,open WBC
	{0x3503, 0x03}, //;manual AE
	{0x3501, 0x10},
	{0x3502, 0x80},
	{0x350a, 0x00},
	{0x350b, 0x7f},
	{0x5001, 0x01}, //;manual AWB
	{0x5180, 0x08},
	{0x5186, 0x04},
	{0x5187, 0x00},
	{0x5188, 0x04},
	{0x5189, 0x00},
	{0x518a, 0x04},
	{0x518b, 0x00},
	{0x5000, 0x06}, //;No lenc,WBC on
};

struct ov5647_reg ov5647_regs = {
	.rec_settings = &ov5647_recommend_settings[0],
	.rec_size = ARRAY_SIZE(ov5647_recommend_settings),
	.reg_prev = &ov5647_prev_settings[0],
	.reg_prev_size = ARRAY_SIZE(ov5647_prev_settings),
	.reg_snap = &ov5647_snap_settings[0],
	.reg_snap_size = ARRAY_SIZE(ov5647_snap_settings),
};
