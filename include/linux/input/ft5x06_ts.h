/*
 *
 * FocalTech ft5x06 TouchScreen driver header file.
 *
 * Copyright (c) 2010  Focal tech Ltd.
 * Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __LINUX_FT5X06_TS_H__
#define __LINUX_FT5X06_TS_H__

struct ft5x06_ts_platform_data {
	unsigned long irqflags;
	u32 x_max;
	u32 y_max;
	u32 irq_gpio;
	u32 reset_gpio;
	int (*power_init) (bool);
	int (*power_on) (bool);
};
enum ft5x0x_ts_regs {
	FT5X0X_REG_THGROUP = 0x80,
	FT5X0X_REG_THPEAK = 0x81,
	FT5X0X_REG_THCAL = 0x82,
	FT5X0X_REG_THWATER = 0x83,
	FT5X0X_REG_THTEMP = 0x84,
	FT5X0X_REG_THDIFF = 0x85,
	FT5X0X_REG_CTRL = 0x86,
	FT5X0X_REG_TIMEENTERMONITOR = 0x87,
	FT5X0X_REG_PERIODACTIVE = 0x88,
	FT5X0X_REG_PERIODMONITOR = 0x89,
	FT5X0X_REG_HEIGHT_B = 0x8a,
	FT5X0X_REG_MAX_FRAME = 0x8b,
	FT5X0X_REG_DIST_MOVE = 0x8c,
	FT5X0X_REG_DIST_POINT = 0x8d,
	FT5X0X_REG_FEG_FRAME = 0x8e,
	FT5X0X_REG_SINGLE_CLICK_OFFSET = 0x8f,
	FT5X0X_REG_DOUBLE_CLICK_TIME_MIN = 0x90,
	FT5X0X_REG_SINGLE_CLICK_TIME = 0x91,
	FT5X0X_REG_LEFT_RIGHT_OFFSET = 0x92,
	FT5X0X_REG_UP_DOWN_OFFSET = 0x93,
	FT5X0X_REG_DISTANCE_LEFT_RIGHT = 0x94,
	FT5X0X_REG_DISTANCE_UP_DOWN = 0x95,
	FT5X0X_REG_ZOOM_DIS_SQR = 0x96,
	FT5X0X_REG_RADIAN_VALUE = 0x97,
	FT5X0X_REG_MAX_X_HIGH = 0x98,
	FT5X0X_REG_MAX_X_LOW = 0x99,
	FT5X0X_REG_MAX_Y_HIGH = 0x9a,
	FT5X0X_REG_MAX_Y_LOW = 0x9b,
	FT5X0X_REG_K_X_HIGH = 0x9c,
	FT5X0X_REG_K_X_LOW = 0x9d,
	FT5X0X_REG_K_Y_HIGH = 0x9e,
	FT5X0X_REG_K_Y_LOW = 0x9f,
	FT5X0X_REG_AUTO_CLB_MODE = 0xa0,
	FT5X0X_REG_LIB_VERSION_H = 0xa1,
	FT5X0X_REG_LIB_VERSION_L = 0xa2,
	FT5X0X_REG_CIPHER = 0xa3,
	FT5X0X_REG_MODE = 0xa4,
	FT5X0X_REG_PMODE = 0xa5,	/* Power Consume Mode           */
	FT5X0X_REG_FIRMID = 0xa6,
	FT5X0X_REG_STATE = 0xa7,
	FT5X0X_REG_FT5201ID = 0xa8,
	FT5X0X_REG_ERR = 0xa9,
	FT5X0X_REG_CLB = 0xaa,
};

enum {
        Vendor_Id_XinHaoTP = 0x3e,
        Vendor_Id_XinLiTP  = 0x5a,
        Vendor_Id_DiJinTP = 0x44,
        Vendor_Id_WangDeTP  = 0x45,
        Vendor_Id_HuaXinDaTP = 0x49,
    };

    enum {
        Only_Id_XinHaoTP_1 = 0x01,
        Only_Id_XinHaoTP_2 = 0x02,
        Only_Id_XinHaoTP_3 = 0x03,
        Only_Id_XinHaoTP_4 = 0x04,
        Only_Id_XinHaoTP_5 = 0x05,
        Only_Id_XinLiTP_1 = 0x12,
        Only_Id_XinLiTP_2 = 0x14,
        Only_Id_XinLiTP_3 = 0x15,
        Only_Id_DiJin_1 = 0x23,
        Only_Id_DiJin_2 = 0x24,
        Only_Id_WangDe_2 = 0x35,
        Only_Id_HuaXinDa_1 = 0x44,
        
    };
#endif
