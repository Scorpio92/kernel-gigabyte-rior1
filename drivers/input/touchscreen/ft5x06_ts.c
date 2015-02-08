/* drivers/input/touchscreen/ft5x06_ts.c
 *
 * FocalTech ft5x06 TouchScreen driver.
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
//#define DEBUG
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/input/ft5x06_ts.h>
#include <mach/pmic.h>
#include <linux/leds.h>
#include <linux/board-ragentek-cfg.h>
#include <../../../../build/buildplus/target/QRDExt_target.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
/* Early-suspend level */
#define FT5X06_SUSPEND_LEVEL 1
#endif

#define CONFIG_SUPPORT_FTS_CTP_UPG
#define NODE_CTP_UPG
#define CFG_MAX_TOUCH_POINTS	5

#define FT_STARTUP_DLY		150
#define FT_RESET_DLY		20

#define FT_PRESS		0x7F
#define FT_MAX_ID		0x0F
#define FT_TOUCH_STEP		6
#define FT_TOUCH_X_H_POS	3
#define FT_TOUCH_X_L_POS	4
#define FT_TOUCH_Y_H_POS	5
#define FT_TOUCH_Y_L_POS	6
#define FT_TOUCH_PRESSURE   7
#define FT_TOUCH_SIZE   8
#define FT_TOUCH_EVENT_POS	3
#define FT_TOUCH_ID_POS		5

#define POINT_READ_BUF	(3 + FT_TOUCH_STEP * CFG_MAX_TOUCH_POINTS)

/*register address*/
#define FT5X06_REG_PMODE	0xA5
#define FT5X06_REG_FW_VER	0xA6
#define FT5X06_REG_POINT_RATE	0x88
#define FT5X06_REG_THGROUP	0x80
#define FT5X06_REG_ONLY_ID	0xB6
#define FT5X06_REG_UPGRADE_ID 0xA3

/* power register bits*/
#define FT5X06_PMODE_ACTIVE		0x00
#define FT5X06_PMODE_MONITOR		0x01
#define FT5X06_PMODE_STANDBY		0x02
#define FT5X06_PMODE_HIBERNATE		0x03

#define FT5X06_VTG_MIN_UV	2850000
#define FT5X06_VTG_MAX_UV	2850000
#define FT5X06_I2C_VTG_MIN_UV	1800000
#define FT5X06_I2C_VTG_MAX_UV	1800000
#define FAILURE_COUNTS 3
#define RESET_FAILURE_COUNTS  8

struct i2c_client *this_client_g;

struct ts_event {
	u16 x[CFG_MAX_TOUCH_POINTS];	/*x coordinate */
	u16 y[CFG_MAX_TOUCH_POINTS];	/*y coordinate */
	/* touch event: 0 -- down; 1-- contact; 2 -- contact */
	u8 touch_event[CFG_MAX_TOUCH_POINTS];
	u8 finger_id[CFG_MAX_TOUCH_POINTS];	/*touch ID */
	u16 pressure[CFG_MAX_TOUCH_POINTS];
	u16 size[CFG_MAX_TOUCH_POINTS];
	u8 touch_point;
};

struct ft5x06_ts_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct ts_event event;
	const struct ft5x06_ts_platform_data *pdata;
	struct regulator *vdd;
	struct regulator *vcc_i2c;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
};

static struct i2c_client *this_client;
static int suspend_status=0;
static unsigned char vendor_id;
static unsigned char only_id;
static unsigned char upgrade_id;
static int isUpdate = -1;
static int is_ft5x06 = 1;//QELS-1791 pengjinlong add  20121122 
static unsigned char i_file_version, ic_firmware_version; 
static int use_adb_upgrade = 0;

extern int  sym_chg_connected(void);


// add for get TP info
static  unsigned char  tp_info[512]; 

static int tp_fm_dect_enable_cit = 0;
static int is_tp_fw_effect =0;


#if defined(ENABLE_TP_FIRMWARE_DECT_ENABLE_CIT) && ENABLE_TP_FIRMWARE_DECT_ENABLE_CIT

static int support_tps_per_customer = 0;
struct tp_firmware_info
{
	unsigned char verndor_id;
	unsigned char only_id;
	unsigned char firmware_version;
};

#if 0
enum PROJECT_CUSTOMERS
{
	Q801_QC = 0,
	Q801_VOBIS,
	Q801_CELKON,
	Q802_QC_S2,
	Q802_VOBIS_S2,
	Q802_FLY_S2,
	Q802_FLY_S23,
	 Q802_QC_S22,
	 Q802_QC_VIDEOCOM_S22,
	 Q802_GIGABYTE_S22,

};

struct tp_firmware_info tps_fm_info[][MAX_MDDELS_ONE_CUSTOMER] =
{
	[Q801_QC] = {
					[0] ={0x5a,0x12,0x18}, // TRULY CT1158-E Q801_QC , Q801_VOBIS
					[1] ={0x5a,0x12,0x17}, // TRULY CT1285-E Q801_QC , Q801_Celkon
				},
	[Q801_VOBIS] = {
					[0] ={0x5a,0x12,0x18}, // TRULY CT1158-E Q801_QC , Q801_VOBIS
				},
	[Q801_CELKON] = {
					[0] ={0x5a,0x12,0x17}, // TRULY CT1285-E Q801_QC , Q801_Celkon
				},
	[Q802_QC_S2] = {
					[0] ={0x5a,0x12,0x12}, // TRULY CT1039(black color) Q802_QC(Q802-S2)
					[1] ={0x5a,0x12,0x12}, // TRULY CT1276(white color) Q802_QC(Q802-S2)
				},
	[Q802_VOBIS_S2] = {
					[0] ={0x5a,0x12,0x15}, // TRULY CT1274-E (black and no logo) Q802_VOBIS(Q802-S2),Q802_FLY(Q802-S2)
				},
	[Q802_FLY_S2] = {
					[1] ={0x5a,0x12,0x15}, // TRULY CT1274-E (black and no logo) Q802_VOBIS(Q802-S2),Q802_FLY(Q802-S2)
				},
	[Q802_FLY_S23] = {
					[0] ={0x5a,0x12,0x12}, // TRULY CT1275-E (white and no logo) Q802_FLY(Q802-S23)
				},
	[Q802_QC_S22] = {
					[0] ={0x3e,0x5,0x11}, // XINHAO T223-B1  Q802_QC(Q802-S22),Q802_VIDEOCOM(Q802-S22),Q802_GIGABYTE(Q802-S22)
				},
	[Q802_QC_VIDEOCOM_S22] = {
					[0] ={0x3e,0x5,0x11}, // XINHAO T223-B1  Q802_QC(Q802-S22),Q802_VIDEOCOM(Q802-S22),Q802_GIGABYTE(Q802-S22)
				},
	[Q802_GIGABYTE_S22] = {
					[0] ={0x3e,0x5,0x11}, // XINHAO T223-B1  Q802_QC(Q802-S22),Q802_VIDEOCOM(Q802-S22),Q802_GIGABYTE(Q802-S22)
				},
};
#else
struct tp_firmware_info tps_fm_info[] =
{
#if defined(CONFIG_Q801_QC) && CONFIG_Q801_QC
	[0] ={0x5a,0x12,0x18}, // TRULY CT1158-E Q801_QC , Q801_VOBIS
	[1] ={0x5a,0x12,0x17}, // TRULY CT1285-E Q801_QC , Q801_Celkon
#elif  defined(CONFIG_Q801_VOBIS) && CONFIG_Q801_VOBIS
	[0] ={0x5a,0x12,0x18}, // TRULY CT1158-E Q801_QC , Q801_VOBIS
#elif  defined(CONFIG_Q801_CELKON) && CONFIG_Q801_CELKON
	[0] ={0x5a,0x12,0x17}, // TRULY CT1285-E Q801_QC , Q801_Celkon
#elif defined(CONFIG_Q802_QC) && CONFIG_Q802_QC
	[0] ={0x5a,0x12,0x12}, // TRULY CT1039(black color) Q802_QC(Q802-S2)
	[1] ={0x5a,0x12,0x12}, // TRULY CT1276(white color) Q802_QC(Q802-S2)
	[2] ={0x3e,0x5,0x11}, // XINHAO T223-B1  Q802_QC(Q802-S22),Q802_VIDEOCOM(Q802-S22),Q802_GIGABYTE(Q802-S22)
#elif  defined(CONFIG_Q802_VOBIS) && CONFIG_Q802_VOBIS
	[0] ={0x5a,0x12,0x15}, // TRULY CT1274-E (black and no logo) Q802_VOBIS(Q802-S2),Q802_FLY(Q802-S2)
#elif defined(CONFIG_Q802_FLY) && CONFIG_Q802_FLY
	[0] ={0x5a,0x12,0x15}, // TRULY CT1274-E (black and no logo) Q802_VOBIS(Q802-S2),Q802_FLY(Q802-S2)
	[1] ={0x5a,0x12,0x12}, // TRULY CT1275-E (white and no logo) Q802_FLY(Q802-S23)
#elif defined(CONFIG_Q802_VIDEOCOM) && CONFIG_Q802_VIDEOCOM
	[0] ={0x3e,0x5,0x11}, // XINHAO T223-B1  Q802_QC(Q802-S22),Q802_VIDEOCOM(Q802-S22),Q802_GIGABYTE(Q802-S22)
#elif  defined(CONFIG_Q802_GIGABYTE) && CONFIG_Q802_GIGABYTE
	[0] ={0x3e,0x5,0x11}, // XINHAO T223-B1  Q802_QC(Q802-S22),Q802_VIDEOCOM(Q802-S22),Q802_GIGABYTE(Q802-S22)
#else
	[0] ={0x00,0x00,0x00}, // XINHAO T223-B1  Q802_QC(Q802-S22),Q802_VIDEOCOM(Q802-S22),Q802_GIGABYTE(Q802-S22)
#endif
};

#endif

#endif

/*
*ft5x06_i2c_read-read data and write data by i2c
*@client: handle of i2c
*@writebuf: Data that will be written to the slave
*@writelen: How many bytes to write
*@readbuf: Where to store data read from slave
*@readlen: How many bytes to read
*
*Returns negative errno, else the number of messages executed
*
*/
static int ft5x06_i2c_read(struct i2c_client *client, char *writebuf,
			   int writelen, char *readbuf, int readlen)
{
	int ret;

	if (writelen > 0) {
		struct i2c_msg msgs[] = {
			{
			 .addr = client->addr,
			 .flags = 0,
			 .len = writelen,
			 .buf = writebuf,
			 },
			{
			 .addr = client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 2);
		if (ret < 0)
			dev_err(&client->dev, "f%s: i2c read error.\n",
				__func__);
	} else {
		struct i2c_msg msgs[] = {
			{
			 .addr = client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 1);
		if (ret < 0)
			dev_err(&client->dev, "%s:i2c read error.\n", __func__);
	}
	return ret;
}

/*
*write data by i2c
*/
static int ft5x06_i2c_write(struct i2c_client *client, char *writebuf,
			    int writelen)
{
	int ret;

	struct i2c_msg msgs[] = {
		{
		 .addr = client->addr,
		 .flags = 0,
		 .len = writelen,
		 .buf = writebuf,
		 },
	};
	ret = i2c_transfer(client->adapter, msgs, 1);
	if (ret < 0)
		dev_err(&client->dev, "f%s: i2c write error.\n", __func__);

	return ret;
}

static int ft5x06_write_reg(u8 addr, u8 para)
{
 	int ret = 0;

	char buf[2] = { 0 };
	buf[0] =addr;
	buf[1] =para;

	ret = ft5x06_i2c_write(this_client, buf, 2);
	return ret;
}

static int ft5x06_read_reg(u8 addr, u8 * pdata)
{
	int ret;
	u8 buf[2] = { 0 };
	struct i2c_msg msgs[] = {
		{
		 .addr = this_client->addr,
		 .flags = 0,
		 .len = 1,
		 .buf = buf,
		 },
		{
		 .addr = this_client->addr,
		 .flags = I2C_M_RD,
		 .len = 1,
		 .buf = buf,
		 },
	};

	buf[0] = addr;
	//msleep(1);
	ret = i2c_transfer(this_client->adapter, msgs, 2);
	if (ret < 0)
		pr_err("msg %s i2c read error: %d\n", __func__, ret);

	*pdata = buf[0];
	return ret;

}

#if (defined(CONFIG_SUPPORT_FTS_CTP_UPG) || defined(NODE_CTP_UPG))

typedef enum {
	ERR_OK,
	ERR_MODE,
	ERR_READID,
	ERR_ERASE,
	ERR_STATUS,
	ERR_ECC,
	ERR_DL_ERASE_FAIL,
	ERR_DL_PROGRAM_FAIL,
	ERR_DL_VERIFY_FAIL
} E_UPGRADE_ERR_TYPE;

 enum {
	UPGRADE_FAIL = -1,
	UPGRADE_OK,
	NOT_UPGRADE,
} ;



#define FTS_NULL                0x0
#define FTS_TRUE                0x01
#define FTS_FALSE              0x0

#define I2C_CTPM_ADDRESS       0x70

void delay_qt_ms(unsigned long w_ms)
{
	unsigned long i;
	unsigned long j;

	for (i = 0; i < w_ms; i++) {
		for (j = 0; j < 1000; j++) {
			udelay(1);
		}
	}
}

static int ft5x06_i2c_write_with_addr_param(struct i2c_client *client, char dest_addr, char *writebuf,
			    int writelen)
{
	int ret;

	struct i2c_msg msgs[] = {
		{
		 .addr = client->addr,
		 .flags = 0,
		 .len = 1,
		 .buf = &dest_addr,
		 },
		{
		 .addr = client->addr,
		 .flags = 0,
		 .len = writelen,
		 .buf = writebuf,
		 },
	};
	ret = i2c_transfer(client->adapter, msgs, 2);
	if (ret < 0)
		dev_err(&client->dev, "f%s: i2c write error.\n", __func__);

	return ret;
}


/*
[function]:
    callback: read data from ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :the address of the ctpm;
    pbt_buf[out]        :data buffer;
    dw_lenth[in]        :the length of the data buffer;
[return]:
    FTS_TRUE     :success;
    FTS_FALSE    :fail;
*/
bool i2c_read_interface(char read_addr, char * pbt_buf, int dw_lenth)
{
	int ret;
	char writebuf[1] = {read_addr};

	ret = ft5x06_i2c_read(this_client, writebuf, 1, pbt_buf, dw_lenth);

	if (ret <= 0) {
		printk("[TSP]i2c_read_interface error\n");
		return FTS_FALSE;
	}

	return FTS_TRUE;
}

/*
[function]:
    callback: write data to ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :the address of the ctpm;
    pbt_buf[in]        :data buffer;
    dw_lenth[in]        :the length of the data buffer;
[return]:
    FTS_TRUE     :success;
    FTS_FALSE    :fail;
*/
bool i2c_write_interface(char bt_ctpm_addr, char * pbt_buf,
			     int dw_lenth)
{
	int ret;
	ret = ft5x06_i2c_write_with_addr_param(this_client, bt_ctpm_addr, pbt_buf, dw_lenth);
	if (ret <= 0) {
		printk("[TSP]i2c_write_interface error line = %d, ret = %d\n",
		       __LINE__, ret);
		return FTS_FALSE;
	}

	return FTS_TRUE;
}

/*
[function]:
    send a command to ctpm.
[parameters]:
    btcmd[in]        :command code;
    btPara1[in]    :parameter 1;
    btPara2[in]    :parameter 2;
    btPara3[in]    :parameter 3;
    num[in]        :the valid input parameter numbers, if only command code needed and no parameters followed,then the num is 1;
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
bool cmd_write(char btcmd, char btPara1, char btPara2,
		   char btPara3, char num)
{
	char write_cmd[4] = { 0 };

	write_cmd[0] = btcmd;
	write_cmd[1] = btPara1;
	write_cmd[2] = btPara2;
	write_cmd[3] = btPara3;
	return i2c_write_interface(I2C_CTPM_ADDRESS, write_cmd, num);
}

/*
[function]:
    write data to ctpm , the destination address is 0.
[parameters]:
    pbt_buf[in]    :point to data buffer;
    bt_len[in]        :the data numbers;
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
bool byte_write(char * pbt_buf, int dw_len)
{

	return i2c_write_interface(I2C_CTPM_ADDRESS, pbt_buf, dw_len);
}

/*
[function]:
    read out data from ctpm,the destination address is 0.
[parameters]:
    pbt_buf[out]    :point to data buffer;
    bt_len[in]        :the data numbers;
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
bool byte_read(char * pbt_buf, char bt_len)
{
	return i2c_read_interface(I2C_CTPM_ADDRESS, pbt_buf, bt_len);
}

/*
[function]:
    burn the FW to ctpm.
[parameters]:(ref. SPEC)
    pbt_buf[in]    :point to Head+FW ;
    dw_lenth[in]:the length of the FW + 6(the Head length);
    bt_ecc[in]    :the ECC of the FW
[return]:
    ERR_OK        :no error;
    ERR_MODE    :fail to switch to UPDATE mode;
    ERR_READID    :read id fail;
    ERR_ERASE    :erase chip fail;
    ERR_STATUS    :status error;
    ERR_ECC        :ecc error.
*/

#define    FTS_PACKET_LENGTH        128

static unsigned char CTPM_801S1_XL_GO_FW[] = {
#include "Ragentek_Q801_Truly_0x17_app.i" //"Ragentek_Q801_Truly_0x15_app.i" //#include "Ragentek_Q801_Truly_0x14_app.i"   			//"Ragentek_Q802S2_Tuly_Greenorange_0x11_app.i"
};

static unsigned char CTPM_801S1_XH_GO_FW[] = {
#include "Ragentek_Q801_Xinhao_0x16_app.i" //"Ragentek_Q801_Xinhao_0x14_app.i" 		//	"Ragentek_Q802S2_Xinhao_Greenorange_0x11_app.i"
};

static unsigned char CTPM_802S1_XH_35_FW[] = {
	#include  "Ragentek_Q802S1_Xinhao_35Phone_0x11_app.i"   // "Ragentek_Q802S1_Xinhao_35Phone_0x10_app.i"
};

static unsigned char CTPM_802S2_XL_GO_FW[] = {
#include "Ragentek_Q802S2_Tuly_Greenorange_0x12_app.i"   			//"Ragentek_Q802S2_Tuly_Greenorange_0x11_app.i"
};

static unsigned char CTPM_802S2_XH_GO_FW[] = {
#include "Ragentek_Q802S2_Xinhao_Greenorange_0x12_app.i" 		//	"Ragentek_Q802S2_Xinhao_Greenorange_0x11_app.i"
};
#if 0
static unsigned char CTPM_802S21_XH_BU1_FW[] = {
#include "Ragentek_Q802S21_Xinhao_BU1_0x14_app.i"				//"Ragentek_Q802S21_Xinhao_BU1_0x12_app.i"
};
#endif

static unsigned char CTPM_802CY_DJ_FW[] = {
//#include "Ragentek_Q802S3_Dijing_Tacano_0x12_app.i"				//"Ragentek_Q802S3_Dijing_Tacano_0x11_app.i"
#include "Ragentek_Q802S22_Dijing_0x13_app.i"				//"Ragentek_Q802S3_Dijing_Tacano_0x11_app.i"
};

static unsigned char CTPM_803IFX_WD_FW[] = {
#include "Ragentek_Q803_Wande_Infinix_0x11_app.i"				//"Ragentek_Q802S3_Dijing_Tacano_0x11_app.i"
};


static unsigned char CTPM_802S3_XH_FW[] = {           //802 202¹²ÓÃ£¬²»¿ÉÉýŒ¶
#include "Ragentek_Q802S3_Xinhao_Tacano_0x10_app.i"
};

static unsigned char CTPM_802S3_XH_POSITIVE_FW[] = {           //802 202¹²ÓÃ£¬²»¿ÉÉý\u0152¶
#include "Ragentek_Q802S3_Xinhao_Q8030HXD_Positivo_0x11_app.i"
};


E_UPGRADE_ERR_TYPE fts_ctpm_fw_upgrade(char * pbt_buf, int dw_lenth)
{
	char reg_val[2] = { 0 };
	int i = 0;

	int packet_number;
	int j;
	int temp;
	int lenght;
	char packet_buf[FTS_PACKET_LENGTH + 6];
	char auc_i2c_write_buf[10];
	char bt_ecc;
	int i_ret;

    /*********Step 1:Reset  CTPM *****/
	/*write 0xaa to register 0xfc */
	i_ret = ft5x06_write_reg(0xfc, 0xaa);
	if(i_ret < 0){
		printk("fts_ctpm_fw_upgrade  ft5x06_write_reg(0xfc, 0xaa) fail\n");
		return -1;
	}
	delay_qt_ms(50);
	/*write 0x55 to register 0xfc */
	ft5x06_write_reg(0xfc, 0x55);
	printk("[TSP] Step 1: Reset CTPM test\n");

	delay_qt_ms(30);

    /*********Step 2:Enter upgrade mode *****/
	auc_i2c_write_buf[0] = 0x55;
	auc_i2c_write_buf[1] = 0xaa;
	do {
		i++;
		i_ret = ft5x06_i2c_write(this_client, auc_i2c_write_buf, 2);
		delay_qt_ms(5);
	} while (i_ret <= 0 && i < 5);
	printk("[TSP] Step 2:Enter upgrade mode");
    /*********Step 3:check READ-ID***********************/
	cmd_write(0x90, 0x00, 0x00, 0x00, 4);
	byte_read(reg_val, 2);
	if (reg_val[0] == 0x79 && reg_val[1] == 0x3) {
		printk("[TSP] Step 3^: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",
		       reg_val[0], reg_val[1]);
	} else {
		printk("[TSP] Step 3V: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",
		       reg_val[0], reg_val[1]);
		//return ERR_READID;
		//i_is_new_protocol = 1;
	}

     /*********Step 4:erase app*******************************/
	cmd_write(0x61, 0x00, 0x00, 0x00, 1);

	delay_qt_ms(1500);
	printk("[TSP] Step 4: erase. \n");

    /*********Step 5:write firmware(FW) to ctpm flash*********/
	bt_ecc = 0;
	printk("[TSP] Step 5: start upgrade. \n");
	dw_lenth = dw_lenth - 8;
	packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
	packet_buf[0] = 0xbf;
	packet_buf[1] = 0x00;
	for (j = 0; j < packet_number; j++) {
		temp = j * FTS_PACKET_LENGTH;
		packet_buf[2] = (char) (temp >> 8);
		packet_buf[3] = (char) temp;
		lenght = FTS_PACKET_LENGTH;
		packet_buf[4] = (char) (lenght >> 8);
		packet_buf[5] = (char) lenght;

		for (i = 0; i < FTS_PACKET_LENGTH; i++) {
			packet_buf[6 + i] = pbt_buf[j * FTS_PACKET_LENGTH + i];
			bt_ecc ^= packet_buf[6 + i];
		}

		byte_write(&packet_buf[0], FTS_PACKET_LENGTH + 6);
		delay_qt_ms(FTS_PACKET_LENGTH / 6 + 1);
		if ((j * FTS_PACKET_LENGTH % 1024) == 0) {
			printk("[TSP] upgrade the 0x%x th byte.\n",
			       ((unsigned int)j) * FTS_PACKET_LENGTH);
		}
	}

	if ((dw_lenth) % FTS_PACKET_LENGTH > 0) {
		temp = packet_number * FTS_PACKET_LENGTH;
		packet_buf[2] = (char) (temp >> 8);
		packet_buf[3] = (char) temp;

		temp = (dw_lenth) % FTS_PACKET_LENGTH;
		packet_buf[4] = (char) (temp >> 8);
		packet_buf[5] = (char) temp;

		for (i = 0; i < temp; i++) {
			packet_buf[6 + i] =
			    pbt_buf[packet_number * FTS_PACKET_LENGTH + i];
			bt_ecc ^= packet_buf[6 + i];
		}

		byte_write(&packet_buf[0], temp + 6);
		delay_qt_ms(20);
	}
	//send the last six byte
	for (i = 0; i < 6; i++) {
		temp = 0x6ffa + i;
		packet_buf[2] = (char) (temp >> 8);
		packet_buf[3] = (char) temp;
		temp = 1;
		packet_buf[4] = (char) (temp >> 8);
		packet_buf[5] = (char) temp;
		packet_buf[6] = pbt_buf[dw_lenth + i];
		bt_ecc ^= packet_buf[6];

		byte_write(&packet_buf[0], 7);
		delay_qt_ms(20);
	}

    /*********Step 6: read out checksum***********************/
	/*send the opration head */
	cmd_write(0xcc, 0x00, 0x00, 0x00, 1);
	byte_read(reg_val, 1);
	printk("[TSP] Step 6:  ecc read 0x%x, new firmware 0x%x. \n",
	       reg_val[0], bt_ecc);
	if (reg_val[0] != bt_ecc) {
		return ERR_OK;//return ERR_ECC;  			// sym modify
	}

    /*********Step 7: reset the new FW***********************/
	cmd_write(0x07, 0x00, 0x00, 0x00, 1);
	printk("[TSP] Step 7: reset the new FW");
	return ERR_OK;
}

#ifdef CONFIG_SUPPORT_FTS_CTP_UPG
// reflash TP fw every time after upgrade event by anxiang.xiao 2012-11-08
static void refresh_tp_info(void)
{
	ft5x06_read_reg(FT5X06_REG_FW_VER, &ic_firmware_version);
	ft5x06_read_reg(FT5X0X_REG_FT5201ID, &vendor_id);
	ft5x06_read_reg(FT5X06_REG_ONLY_ID, &only_id);
}
#endif

int fts_ctpm_fw_upgrade_with_i_file(void)
{
	char *pbt_buf = FTS_NULL;
	int fwlen = 0;
	int i_ret = 0;
	printk("fts_ctpm_fw_upgrade_with_i_file   vendor_id=0x%x,only_id=0x%x\n",vendor_id,only_id);
	if(is_rgtk_product(RGTK_PRODUCT_Q801)) 
	{
		printk("upgrade 801 \n");
		//vendor_id = 0x3e;
		//only_id = 0x02;
		switch(vendor_id){
			case 0x5a:		//ÐÅÀû
				switch(only_id){
					case 0x12: 		 //s1 Çà³È TRULY
						fwlen = sizeof(CTPM_801S1_XL_GO_FW);
						pbt_buf = CTPM_801S1_XL_GO_FW;
						break;
					default:
						printk("no such only_id=0x%x in vendor_id=%x0x\n",only_id,vendor_id);
						break;
					}
					break;
						
			case 0x3e:		//ÐÅºÀ
				switch(only_id){
					case 0x02:		//s2 Çà³È ÐÅå©
						fwlen = sizeof(CTPM_801S1_XH_GO_FW);
						pbt_buf = CTPM_801S1_XH_GO_FW;					
						break;					
					default:
						printk("no such only_id=0x%x in vendor_id=%x0x\n",only_id,vendor_id);
						break;
					}
					break;
			default:
					printk("software has compatible with  such  vendor_id=0x%x\n",vendor_id);
					break;
		}
	}
	else if(is_rgtk_product(RGTK_PRODUCT_Q802)) 
	{	
		printk("upgrade 802 \n");
		//vendor_id = 0x44;
		//only_id = 0x23;
		switch(vendor_id){
			case 0x5a:		//ÐÅÀû
				switch(only_id){
					case 0x12: 		 //s2 Çà³È TRULY
						fwlen = sizeof(CTPM_802S2_XL_GO_FW);
						pbt_buf = CTPM_802S2_XL_GO_FW;
						break;
					default:
						printk("no such only_id=0x%x in vendor_id=%x0x\n",only_id,vendor_id);
						break;
					}
					break;
						
			case 0x3e:		//ÐÅºÀ
				switch(only_id){
					case 0x04:			//s21 xinhao
						fwlen = sizeof(CTPM_802S3_XH_POSITIVE_FW);
						pbt_buf = CTPM_802S3_XH_POSITIVE_FW;
						break;
					case 0x03:  			 
						fwlen = sizeof(CTPM_802S3_XH_FW);
						pbt_buf = CTPM_802S3_XH_FW;
						break;
					case 0x02:		//s2 Çà³È ÐÅå©
						fwlen = sizeof(CTPM_802S2_XH_GO_FW);
						pbt_buf = CTPM_802S2_XH_GO_FW;					
						break;
					case 0x01:  			 //for 802-35 hulian
						fwlen = sizeof(CTPM_802S1_XH_35_FW);
						pbt_buf = CTPM_802S1_XH_35_FW;
						break;		
					case 0x05:
						fwlen = sizeof(CTPM_802S3_XH_POSITIVE_FW);
						pbt_buf = CTPM_802S3_XH_POSITIVE_FW;
					default:
						printk("no such only_id=0x%x in vendor_id=%x0x\n",only_id,vendor_id);
						break;
					}
					break;
				
			case 0x44:  		//µÛŸ§
				switch(only_id){
					case 0x23:			//CY Tacano dijing
						fwlen = sizeof(CTPM_802CY_DJ_FW);
						pbt_buf = CTPM_802CY_DJ_FW;
						break;
					case 0x24:			//CY Tacano dijing
						fwlen = sizeof(CTPM_802CY_DJ_FW);
						pbt_buf = CTPM_802CY_DJ_FW;
						break;
					default:
						printk("no such only_id=0x%x in vendor_id=%x0x\n",only_id,vendor_id);
						break;
					}
					break;
			default:
					printk("software has compatible with  such  vendor_id=%x0x\n",vendor_id);
					break;
		}

	}
	else if(is_rgtk_product(RGTK_PRODUCT_Q803) || is_rgtk_product(RGTK_PRODUCT_Q203))
	{
		printk("upgrade 803 \n");
	//	vendor_id = 0x45;
	//	only_id = 0x35;
		switch(vendor_id){
			case 0x45:		//ÐÅÀû
				switch(only_id){
					case 0x35: 		 //s1 Çà³È TRULY
						fwlen = sizeof(CTPM_803IFX_WD_FW);
						pbt_buf = CTPM_803IFX_WD_FW;
						break;
					default:
						printk("no such only_id=0x%x in vendor_id=%x0x\n",only_id,vendor_id);
						break;
					}
					break;
						
			default:
					printk("software has compatible with  such  vendor_id=%x0x\n",vendor_id);
					break;
		}
	}
	else
	{
		fwlen = 0;
	}

	if(fwlen != 0){
		// change to support upgrade tp fw during booting process, by anxiang.xiao 2012-11-08
		i_file_version = pbt_buf[fwlen - 2];
		printk("@@@i file version is 0x%x\n@@@", i_file_version);
		if (use_adb_upgrade)
			i_ret = fts_ctpm_fw_upgrade(pbt_buf, fwlen);
		else if ((i_file_version > ic_firmware_version) || (vendor_id == 0x3e && only_id == 0x04 &&  ic_firmware_version == 0x14))
			i_ret = fts_ctpm_fw_upgrade(pbt_buf, fwlen);
		else
		{
			i_ret = NOT_UPGRADE;
			printk("i file fw version <= tp ic fw version\n");
		}

		if (i_ret != 0) {
			isUpdate = 0;
			printk("fts_ctpm_fw_upgrade fail or not upgrade \n");
		}else{
			isUpdate = 1;
		}
		// end	
	}
	return i_ret;
	
}

#endif


/*
*report the point information
*/
static void ft5x06_report_value(struct ft5x06_ts_data *data)
{
	struct ts_event *event = &data->event;
	int i;
	int fingerdown = 0;


		for (i = 0; i < event->touch_point; i++) {
			#if 0
			if(0/*is_rgtk_product(RGTK_PRODUCT_Q801)*/){
				if (event->touch_event[i] == 0 || event->touch_event[i] == 2) {
					fingerdown++;
				} 
			}else{
				if (event->touch_event[i] == 0 || event->touch_event[i] == 2) {
					event->pressure[i] = FT_PRESS;   // <0x7F
					event->size[i] = 1;   // <16
					fingerdown++;
				} else {
					event->pressure[i] = 0;
					event->size[i] = 0;
				}
			}
			#endif

			if (event->touch_event[i] == 0 || event->touch_event[i] == 2)
			{
				if(event->pressure[i] == 0)
				{
					event->pressure[i] = FT_PRESS;   // <0x7F
					event->size[i] = 1;   // <16
				}
					fingerdown++;
			}
			else
			{
					event->pressure[i] = 0;
					event->size[i] = 0;
			}
			

			
		input_report_abs(data->input_dev, ABS_MT_POSITION_X,
				 event->x[i]);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y,
				 event->y[i]);
				input_report_abs(data->input_dev, ABS_MT_TRACKING_ID,
						 event->finger_id[i]);
				input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR,
							 event->size[i]);
				input_report_abs(data->input_dev, ABS_MT_PRESSURE,
						 event->pressure[i]);
				pr_debug("FTS: x: %d; y: %d", event->x[i], event->y[i]);
				//printk("pressure=0x%x,size=0x%x",event->pressure[i],event->size[i]);

		input_mt_sync(data->input_dev);
	}

	input_report_key(data->input_dev, BTN_TOUCH, !!fingerdown);
	input_sync(data->input_dev);
}

/*Read touch point information when the interrupt  is asserted.*/
static int ft5x06_handle_touchdata(struct ft5x06_ts_data *data)
{
	struct ts_event *event = &data->event;
	int ret, i;
	u8 buf[POINT_READ_BUF] = { 0 };
	u8 pointid = FT_MAX_ID;

	ret = ft5x06_i2c_read(data->client, buf, 1, buf, POINT_READ_BUF);
	if (ret < 0) {
		dev_err(&data->client->dev, "%s read touchdata failed.\n",
			__func__);
		return ret;
	}
	memset(event, 0, sizeof(struct ts_event));

	event->touch_point = 0;
	for (i = 0; i < CFG_MAX_TOUCH_POINTS; i++) {
		pointid = (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;
		if (pointid >= FT_MAX_ID)
			break;
		else
			event->touch_point++;
		event->x[i] =
		    (s16) (buf[FT_TOUCH_X_H_POS + FT_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FT_TOUCH_X_L_POS + FT_TOUCH_STEP * i];
		event->y[i] =
		    (s16) (buf[FT_TOUCH_Y_H_POS + FT_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FT_TOUCH_Y_L_POS + FT_TOUCH_STEP * i];
		event->touch_event[i] =
		    buf[FT_TOUCH_EVENT_POS + FT_TOUCH_STEP * i] >> 6;
		event->finger_id[i] =
		    (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;
		
		if(1/*is_rgtk_product(RGTK_PRODUCT_Q801)*/){
			event->pressure[i] =
				buf[FT_TOUCH_PRESSURE + FT_TOUCH_STEP * i];
			event->size[i] =
				buf[FT_TOUCH_SIZE + FT_TOUCH_STEP * i];
		}
				
	}

	ft5x06_report_value(data);

	return 0;
}

/*The ft5x06 device will signal the host about TRIGGER_FALLING.
*Processed when the interrupt is asserted.
*/
static irqreturn_t ft5x06_ts_interrupt(int irq, void *dev_id)
{
	struct ft5x06_ts_data *data = dev_id;
	int rc;

	rc = ft5x06_handle_touchdata(data);
	if (rc)
		pr_err("%s: handling touchdata failed\n", __func__);

	return IRQ_HANDLED;
}

static int ft5x06_power_on(struct ft5x06_ts_data *data, bool on)
{
	int rc;

	if (!on)
		goto power_off;

	rc = regulator_enable(data->vdd);
	if (rc) {
		dev_err(&data->client->dev,
			"Regulator vdd enable failed rc=%d\n", rc);
		return rc;
	}

	rc = regulator_enable(data->vcc_i2c);
	if (rc) {
		dev_err(&data->client->dev,
			"Regulator vcc_i2c enable failed rc=%d\n", rc);
		regulator_disable(data->vdd);
	}

	return rc;

power_off:
	rc = regulator_disable(data->vdd);
	if (rc) {
		dev_err(&data->client->dev,
			"Regulator vdd disable failed rc=%d\n", rc);
		return rc;
	}

	rc = regulator_disable(data->vcc_i2c);
	if (rc) {
		dev_err(&data->client->dev,
			"Regulator vcc_i2c disable failed rc=%d\n", rc);
		regulator_enable(data->vdd);
	}

	return rc;
}

static int ft5x06_power_init(struct ft5x06_ts_data *data, bool on)
{
	int rc;

	if (!on)
		goto pwr_deinit;

	data->vdd = regulator_get(&data->client->dev, "vdd");
	if (IS_ERR(data->vdd)) {
		rc = PTR_ERR(data->vdd);
		dev_err(&data->client->dev,
			"Regulator get failed vdd rc=%d\n", rc);
		return rc;
	}

	if (regulator_count_voltages(data->vdd) > 0) {
		rc = regulator_set_voltage(data->vdd, FT5X06_VTG_MIN_UV,
					   FT5X06_VTG_MAX_UV);
		if (rc) {
			dev_err(&data->client->dev,
				"regulator set_vtg failed rc=%d\n", rc);
			goto reg_vdd_put;
		}
	}

	data->vcc_i2c = regulator_get(&data->client->dev, "vcc_i2c");
	if (IS_ERR(data->vcc_i2c)) {
		rc = PTR_ERR(data->vcc_i2c);
		dev_err(&data->client->dev,
			"Regulator get failed vdd rc=%d\n", rc);
		goto reg_vdd_set_vtg;
	}

	if (regulator_count_voltages(data->vcc_i2c) > 0) {
		rc = regulator_set_voltage(data->vcc_i2c, FT5X06_I2C_VTG_MIN_UV,
					   FT5X06_I2C_VTG_MAX_UV);
		if (rc) {
			dev_err(&data->client->dev,
				"regulator set_vtg failed rc=%d\n", rc);
			goto reg_vcc_i2c_put;
		}
	}
	return rc;

reg_vcc_i2c_put:
	regulator_put(data->vcc_i2c);
reg_vdd_set_vtg:
	if (regulator_count_voltages(data->vdd) > 0)
		regulator_set_voltage(data->vdd, 0, FT5X06_VTG_MAX_UV);
reg_vdd_put:
	regulator_put(data->vdd);
	return rc;

pwr_deinit:
	if (regulator_count_voltages(data->vdd) > 0)
		regulator_set_voltage(data->vdd, 0, FT5X06_VTG_MAX_UV);

	regulator_put(data->vdd);

	if (regulator_count_voltages(data->vcc_i2c) > 0)
		regulator_set_voltage(data->vcc_i2c, 0, FT5X06_I2C_VTG_MAX_UV);

	regulator_put(data->vcc_i2c);
	return 0;
}

void ft5x06_does_when_chg_connected(int is_chg_connected)
{
	u8 reg_addr;
	u8 reg_value;
	int err;
	// int i = 0;
	//QELS-1791 pengjinlong add  20121122 (start) 
	printk("%s\n",__func__);
	if (0 == is_ft5x06){
			printk("TP is not ft5x06, return\n");
			return;
	}
	//QELS-1791 pengjinlong add  20121122 (end) 
	if (1 == suspend_status){
			printk("sym in suspend mode, needn't to write register\n");
			return;
	}

	reg_addr = FT5X06_REG_THGROUP;
		if (is_chg_connected == 1){	
			reg_value = 0x30;
		}else{
			if ((upgrade_id == 0x66 && only_id == 0x04) || (upgrade_id == 0x55 && only_id == 0xea))       //after upgrade)
				reg_value = 0x10;
			else if (only_id == 0x12 && vendor_id == 0x5a && is_rgtk_product(RGTK_PRODUCT_Q801)){   //802 QC truely 
				reg_value = 0x10;
				printk("802 qc xinli\n");
			}
			else 
				reg_value = 0x15;
		}
#if 1
	err = ft5x06_write_reg(reg_addr,reg_value);
#else
		for(i=0; i<RESET_FAILURE_COUNTS; i++){
			err = ft5x06_write_reg(reg_addr,reg_value);
			if (err < 0 ){
				if(i == RESET_FAILURE_COUNTS - 1)
						printk("sym write 0x%x failed ultimately\n",reg_value);
				else{
					printk("sym write 0x%x failed for %d times\n",reg_value,i+1);
					msleep(50);
					continue;
				}
			}else{
			printk("sym write %x successful\n",reg_value);
				break;	
			}
		}
#endif
}EXPORT_SYMBOL(ft5x06_does_when_chg_connected);

#ifdef CONFIG_PM
static int ft5x06_ts_suspend(struct device *dev)
{
	struct ft5x06_ts_data *data = dev_get_drvdata(dev);
	char txbuf[2];
printk("%s\n",__func__);
	disable_irq(data->client->irq);

	if (gpio_is_valid(data->pdata->reset_gpio)) {
		txbuf[0] = FT5X06_REG_PMODE;
		txbuf[1] = FT5X06_PMODE_HIBERNATE;
		ft5x06_i2c_write(data->client, txbuf, sizeof(txbuf));
	}
	suspend_status = 1;
	return 0;
}

static int ft5x06_ts_resume(struct device *dev)
{
	struct ft5x06_ts_data *data = dev_get_drvdata(dev);
	int err;

printk("%s\n",__func__);
	if (gpio_is_valid(data->pdata->reset_gpio)) {
		gpio_set_value_cansleep(data->pdata->reset_gpio, 0);
		msleep(FT_RESET_DLY);
		gpio_set_value_cansleep(data->pdata->reset_gpio, 1);
	}
	

	msleep(100);
	suspend_status = 0;
   ft5x06_does_when_chg_connected(sym_chg_connected());
		 err = ft5x06_write_reg(FT5X06_REG_POINT_RATE, 0x08);
	if (err < 0)
		printk("write FT5X06_REG_POINT_RATE 0x08 err\n");
	enable_irq(data->client->irq);
	
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void ft5x06_ts_early_suspend(struct early_suspend *handler)
{
	struct ft5x06_ts_data *data = container_of(handler,
						   struct ft5x06_ts_data,
						   early_suspend);
	printk("%s\n",__func__);
	ft5x06_ts_suspend(&data->client->dev);

}

static void ft5x06_ts_late_resume(struct early_suspend *handler)
{
	struct ft5x06_ts_data *data = container_of(handler,
						   struct ft5x06_ts_data,
						   early_suspend);
	printk("%s\n",__func__);
	ft5x06_ts_resume(&data->client->dev);
	//ft5x06_does_when_chg_connected(1);
}
#endif

static const struct dev_pm_ops ft5x06_ts_pm_ops = {
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = ft5x06_ts_suspend,
	.resume = ft5x06_ts_resume,
#endif
};
#endif

#if 1
static ssize_t
ft5x06_virtual_keys_register_for_q801(struct kobject *kobj,
			     struct kobj_attribute *attr,
			     char *buf)
{
	return snprintf(buf, 200,
	__stringify(EV_KEY) ":" __stringify(KEY_HOME)  ":81:999:80:60"
	":" __stringify(EV_KEY) ":" __stringify(KEY_MENU)   ":189:999:80:60"
	":" __stringify(EV_KEY) ":" __stringify(KEY_BACK) ":351:999:80:60"
	":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH)   ":459:999:80:60"
	"\n");

}

static ssize_t
ft5x06_virtual_keys_register_for_q802(struct kobject *kobj,
			     struct kobj_attribute *attr,
			     char *buf)
{
	
	switch(vendor_id){
			case 0x5a:		//ÐÅÀû
				switch(only_id){
					case 0x12: 		 //s2 Çà³È TRULY
						return snprintf(buf, 200,
						__stringify(EV_KEY) ":" __stringify(KEY_HOME)  ":72:866:80:60"
						":" __stringify(EV_KEY) ":" __stringify(KEY_MENU)   ":168:866:80:60"
						":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":408:866:80:60"
						":" __stringify(EV_KEY) ":" __stringify(KEY_BACK)   ":312:866:80:60"
						"\n");
						break;
					default:
						printk("no such only_id=0x%x in vendor_id=%x0x\n",only_id,vendor_id);
							return snprintf(buf, 200,
						__stringify(EV_KEY) ":" __stringify(KEY_MENU)  ":72:866:80:60"
						":" __stringify(EV_KEY) ":" __stringify(KEY_HOME)   ":168:866:80:60"
						":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":408:866:80:60"
						":" __stringify(EV_KEY) ":" __stringify(KEY_BACK)   ":312:866:80:60"
						"\n");
						break;
					}
					break;
						
			case 0x3e:		//ÐÅºÀ
				switch(only_id){					
					case 0x02:		//s2 Çà³È ÐÅå©
							return snprintf(buf, 200,
						__stringify(EV_KEY) ":" __stringify(KEY_HOME)  ":72:866:80:60"
						":" __stringify(EV_KEY) ":" __stringify(KEY_MENU)   ":168:866:80:60"
						":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":408:866:80:60"
						":" __stringify(EV_KEY) ":" __stringify(KEY_BACK)   ":312:866:80:60"
						"\n");
						break;
						
					case 0x04:			//s21 xinhao
					case 0x01:  	
					default:
						printk(" only_id=0x%x in vendor_id=%x0x\n",only_id,vendor_id);
							return snprintf(buf, 200,
						__stringify(EV_KEY) ":" __stringify(KEY_HOME)  ":72:866:80:60"
						":" __stringify(EV_KEY) ":" __stringify(KEY_MENU)   ":168:866:80:60"
						":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":408:866:80:60"
						":" __stringify(EV_KEY) ":" __stringify(KEY_BACK)   ":312:866:80:60"
						"\n");
						break;
					}
					break;
				
			case 0x44:  		//µÛŸ§
				switch(only_id){
					case 0x23:	 //CY Tacano dijing
					default:
						printk(" only_id=0x%x in vendor_id=%x0x\n",only_id,vendor_id);
							return snprintf(buf, 200,
						__stringify(EV_KEY) ":" __stringify(KEY_MENU)  ":72:866:80:60"
						":" __stringify(EV_KEY) ":" __stringify(KEY_HOME)   ":168:866:80:60"
						":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":408:866:80:60"
						":" __stringify(EV_KEY) ":" __stringify(KEY_BACK)   ":312:866:80:60"
						"\n");
						break;
					}
					break;
			default:
							return snprintf(buf, 200,
						__stringify(EV_KEY) ":" __stringify(KEY_MENU)  ":72:866:80:60"
						":" __stringify(EV_KEY) ":" __stringify(KEY_HOME)   ":168:866:80:60"
						":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":408:866:80:60"
						":" __stringify(EV_KEY) ":" __stringify(KEY_BACK)   ":312:866:80:60"
						"\n");
					printk("software has compatible with  such  vendor_id=%x0x\n",vendor_id);
					break;
		}
#if 0
printk("only_id = 0x%x",only_id);
switch(only_id)
{
	case 0x01:  			 //for 802-35 hulian
	case 0x23:			//cy  Tacano dijing
	case 0x04:			//s21 xinhao
			return snprintf(buf, 200,
		__stringify(EV_KEY) ":" __stringify(KEY_MENU)  ":72:866:80:60"
		":" __stringify(EV_KEY) ":" __stringify(KEY_HOME)   ":168:866:80:60"
		":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":408:866:80:60"
		":" __stringify(EV_KEY) ":" __stringify(KEY_BACK)   ":312:866:80:60"
		"\n");
		break;
		
	case 0x02: 		 //s2 Çà³È ÐÅå©
	case 0x12:		//s2 Çà³È ÐÅÀû
		return snprintf(buf, 200,
		__stringify(EV_KEY) ":" __stringify(KEY_HOME)  ":72:866:80:60"
		":" __stringify(EV_KEY) ":" __stringify(KEY_MENU)   ":168:866:80:60"
		":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":408:866:80:60"
		":" __stringify(EV_KEY) ":" __stringify(KEY_BACK)   ":312:866:80:60"
		"\n");
		break;
		
	default:            //»¹Ã»ÓÐÌíŒÓ0xb6 ŒÄŽæÆ÷µÄ
		#if(ENABLE_PROJECT_FOR_802QC)
		printk("sym 802 qc\n");
		return snprintf(buf, 200,
		__stringify(EV_KEY) ":" __stringify(KEY_HOME)  ":72:866:80:60"
		":" __stringify(EV_KEY) ":" __stringify(KEY_MENU)   ":168:866:80:60"
		":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":408:866:80:60"
		":" __stringify(EV_KEY) ":" __stringify(KEY_BACK)   ":312:866:80:60"
		"\n");
		#else
		return snprintf(buf, 200,
		__stringify(EV_KEY) ":" __stringify(KEY_MENU)  ":72:866:80:60"
		":" __stringify(EV_KEY) ":" __stringify(KEY_HOME)   ":168:866:80:60"
		":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":408:866:80:60"
		":" __stringify(EV_KEY) ":" __stringify(KEY_BACK)   ":312:866:80:60"
		"\n");
		#endif
		break;
	}
#endif
	return 0;
}

static ssize_t
ft5x06_virtual_keys_register_for_q803(struct kobject *kobj,
			     struct kobj_attribute *attr,
			     char *buf)
{
	return snprintf(buf, 200,
	__stringify(EV_KEY) ":" __stringify(KEY_BACK)  ":64:852:80:80"
	":" __stringify(EV_KEY) ":" __stringify(KEY_HOME)   ":243:852:80:80"
	":" __stringify(EV_KEY) ":" __stringify(KEY_MENU)   ":402:852:80:80"
	"\n");
}

static ssize_t
ft5x06_virtual_keys_register(struct kobject *kobj,
			     struct kobj_attribute *attr,
			     char *buf)
{
	return snprintf(buf, 200,
	__stringify(EV_KEY) ":" __stringify(KEY_MENU)  ":40:510:80:60"
	":" __stringify(EV_KEY) ":" __stringify(KEY_HOMEPAGE)   ":120:510:80:60"
	":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":200:510:80:60"
	":" __stringify(EV_KEY) ":" __stringify(KEY_BACK)   ":280:510:80:60"
	"\n");
}

static struct kobj_attribute ft5x06_virtual_keys_attr = {
	.attr = {
		.name = "virtualkeys.ft5x06_ts",
		.mode = S_IRUGO,
	},
	.show = &ft5x06_virtual_keys_register,
};

#if 0
static struct attribute *ft5x06_virtual_key_properties_attrs[] = {
	&ft5x06_virtual_keys_attr.attr,
	NULL,
};

static struct attribute_group ft5x06_virtual_key_properties_attr_group = {
	.attrs = ft5x06_virtual_key_properties_attrs,
};
#endif


struct kobject *ft5x06_virtual_key_properties_kobj;


static int ft5x06_virtualkey_set(void)
{	
	int rc = 0;
	if(is_rgtk_product(RGTK_PRODUCT_Q801)) {
		ft5x06_virtual_keys_attr.show = &ft5x06_virtual_keys_register_for_q801;
	}
	
	if(is_rgtk_product(RGTK_PRODUCT_Q802)) {
		ft5x06_virtual_keys_attr.show = &ft5x06_virtual_keys_register_for_q802;
	}

	if(is_rgtk_product(RGTK_PRODUCT_Q803) || is_rgtk_product(RGTK_PRODUCT_Q203)) {
		ft5x06_virtual_keys_attr.show = &ft5x06_virtual_keys_register_for_q803;
	}


	ft5x06_virtual_key_properties_kobj =
			kobject_create_and_add("board_properties", NULL);

	if (ft5x06_virtual_key_properties_kobj){
		rc = sysfs_create_file(ft5x06_virtual_key_properties_kobj, &ft5x06_virtual_keys_attr.attr);
	}	
//		rc = sysfs_create_group(ft5x06_virtual_key_properties_kobj,
//				&ft5x06_virtual_key_properties_attr_group);

	if (!ft5x06_virtual_key_properties_kobj || rc)
		pr_err("%s: failed to create board_properties\n", __func__);
	return rc;
}
#endif


static u8 refresh_fw_verson(void)
{
	u8 reg_value;
	u8 reg_addr;
	int k;
	int err;

	
	/*get some register information */
	reg_addr = FT5X06_REG_FW_VER;
	for(k=0; k<FAILURE_COUNTS; k++){   //sym add for compatible with gt818x
		err = ft5x06_i2c_read(this_client_g, &reg_addr, 1, &reg_value, 1);
		if (err<0){
			
				dev_err(&this_client_g->dev, "version read failed");
				if (k==FAILURE_COUNTS-1)
					return -1;
				else
					continue;
		}
		else
			return reg_value;
	}

	return 0;
}


#ifdef NODE_CTP_UPG	//CONFIG_SUPPORT_FTS_CTP_UPG
static ssize_t ft5x06_show_fw_update(struct kobject *kobj, struct kobj_attribute *attr, char *buf){			

	char *temp;
	u8 current_fw_version;

	current_fw_version = refresh_fw_verson();
	
	if(isUpdate == 0x01)
		temp = "upgrade_fw_success";
	else if(isUpdate == 0x00)
		temp = "upgrade_fw_fail";
	else
		temp = "havn't upgrade_fw";
	
		 if(is_rgtk_product(RGTK_PRODUCT_Q801)) {
						 return sprintf(buf, "\n%s\n current fw version : 0x%x\n\
801 QC XL	---------- 1 \n\
801 QC XH	---------- 2 \n", temp,current_fw_version);	
		 }else  if(is_rgtk_product(RGTK_PRODUCT_Q802)) {
		 				 return sprintf(buf, "\n%s\n current fw version : 0x%x\n\
802_35_xh(35Phone) 		---------- 1\n \
802_s21_XH(brrnD) (only id: 0x04,firmware :0x14)	---------- 2\n \
802_CY_DJ(INFINIX)		---------- 3\n \
802_qc_xh_old(only id: 0x02)	---------- 4\n \
802_xh_new_T223_B1(only id: 0x05,firmware :0x11)---------- 5\n \
802_qc_xl(Green@orange)	---------- 6\n ", temp,current_fw_version);	
		 }else  if(is_rgtk_product(RGTK_PRODUCT_Q803) || is_rgtk_product(RGTK_PRODUCT_Q203)) {
		 						 return sprintf(buf, "\n%s\n current fw version : 0x%x\n\
803_CY WD	---------- 1 \n", temp, current_fw_version);	
		
		 }else
		 	 return sprintf(buf, "\nft5x06_show_fw_update   product ID error \n");	

}

static ssize_t ft5x06_store_fw_update(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{

	
		
		unsigned long val = simple_strtoul(buf, NULL, 10);
				if(buf == NULL){
				printk("'select the project through clicking  'cat ftp_fw_update'");
				return -1;
			}
		use_adb_upgrade = 1;
		printk("ft5x06_store_fw_update\n");
		 if(is_rgtk_product(RGTK_PRODUCT_Q801)) {
		 		if(val == 1){
					vendor_id = 0x5a;
					only_id = 0x12;
				}else if(val == 2){
					vendor_id = 0x3e;
					only_id = 0x02;
				}
		 }else  if(is_rgtk_product(RGTK_PRODUCT_Q802)) {
		 		 if(val == 1){
					vendor_id = 0x3e;
					only_id = 0x01;
				}else if(val == 2){
					vendor_id = 0x3e;
					only_id = 0x04;
				}else if(val == 3){
					vendor_id = 0x44;
					only_id = 0x23;
				}else if(val == 4){
					vendor_id = 0x3e;
					only_id = 0x02;
				}
				else if(val == 5){
					vendor_id = 0x3e;
					only_id = 0x05;
				}
				else if(val == 6){
					vendor_id = 0x5a;
					only_id = 0x12;
				}

		 }else  if(is_rgtk_product(RGTK_PRODUCT_Q803) ||is_rgtk_product(RGTK_PRODUCT_Q203) ) {
		 		if(val == 1){
					vendor_id = 0x45;
					only_id = 0x35;
		 		}
		 }else
		 	printk("ft5x06_show_fw_update   product ID error\n ");
		 
		msleep(400);
		fts_ctpm_fw_upgrade_with_i_file();

		gpio_set_value(26, 0);	//set wake = 0,base on system
		msleep(10);
		gpio_set_value(26, 1);	//set wake = 1,base on system
		msleep(300);
		printk("ft5x06_store_fw_update Firmware Upgrade finish\n");
		return 1;
		
}

//static int is_fwupd_success;
static struct kobj_attribute ft5x06_fw_update_attr = {
	.attr = {
		.name = "ftp_fw_update",
//modify BUG_ID:QELS-782 daizhiyi 20121206 (start)
#if 0
		.mode =  S_IRUGO | S_IWUGO ,
#else
		.mode =  0644 ,
#endif
//modify BUG_ID:QELS-782 daizhiyi 20121206 (end)
	},
	.show = &ft5x06_show_fw_update,
	.store = &ft5x06_store_fw_update,
};

#endif

static int ft5x06_ts_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	const struct ft5x06_ts_platform_data *pdata = client->dev.platform_data;
	struct ft5x06_ts_data *data;
	struct input_dev *input_dev;
	u8 reg_value;
	u8 reg_addr;
	int err;
	int k;
#if defined(ENABLE_TP_FIRMWARE_DECT_ENABLE_CIT) && ENABLE_TP_FIRMWARE_DECT_ENABLE_CIT
	int i;
#endif	
	int len = 0;
	u8 current_fw_version;


	this_client_g = client;

	if (!pdata) {
		dev_err(&client->dev, "Invalid pdata\n");
		return -EINVAL;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "I2C not supported\n");
		return -ENODEV;
	}

	data = kzalloc(sizeof(struct ft5x06_ts_data), GFP_KERNEL);
	if (!data) {
		dev_err(&client->dev, "Not enough memory\n");
		return -ENOMEM;
	}

	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		dev_err(&client->dev, "failed to allocate input device\n");
		goto free_mem;
	}
	this_client = client;

	data->input_dev = input_dev;
	data->client = client;
	data->pdata = pdata;

	input_dev->name = "ft5x06_ts";
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;

	input_set_drvdata(input_dev, data);
	i2c_set_clientdata(client, data);

	set_bit(EV_KEY, input_dev->evbit);
	set_bit(EV_ABS, input_dev->evbit);
	set_bit(BTN_MISC, input_dev->keybit);
	set_bit(BTN_TOUCH, input_dev->keybit);

	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0,
			     pdata->x_max, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0,
			     pdata->y_max, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0,
			     CFG_MAX_TOUCH_POINTS, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, FT_PRESS, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, FT_PRESS, 0, 0);

	err = input_register_device(input_dev);
	if (err) {
		dev_err(&client->dev, "Input device registration failed\n");
		goto free_inputdev;
	}

	if (pdata->power_init) {
		err = pdata->power_init(true);
		if (err) {
			dev_err(&client->dev, "power init failed");
			goto unreg_inputdev;
		}
	} else {
		err = ft5x06_power_init(data, true);
		if (err) {
			dev_err(&client->dev, "power init failed");
			goto unreg_inputdev;
		}
	}

	if (pdata->power_on) {
		err = pdata->power_on(true);
		if (err) {
			dev_err(&client->dev, "power on failed");
			goto pwr_deinit;
		}
	} else {
		err = ft5x06_power_on(data, true);
		if (err) {
			dev_err(&client->dev, "power on failed");
			goto pwr_deinit;
		}
	}

	if (gpio_is_valid(pdata->irq_gpio)) {
		err = gpio_request(pdata->irq_gpio, "ft5x06_irq_gpio");
		if (err) {
			dev_err(&client->dev, "irq gpio request failed");
			goto pwr_off;
		}
		err = gpio_direction_input(pdata->irq_gpio);
		if (err) {
			dev_err(&client->dev,
				"set_direction for irq gpio failed\n");
			goto free_irq_gpio;
		}
	}

	if (gpio_is_valid(pdata->reset_gpio)) {
		err = gpio_request(pdata->reset_gpio, "ft5x06_reset_gpio");
		if (err) {
			dev_err(&client->dev, "reset gpio request failed");
			goto free_irq_gpio;
		}

		err = gpio_direction_output(pdata->reset_gpio, 0);
		if (err) {
			dev_err(&client->dev,
				"set_direction for reset gpio failed\n");
			goto free_reset_gpio;
		}
		msleep(FT_RESET_DLY);
		gpio_set_value_cansleep(data->pdata->reset_gpio, 1);
	}
	/* make sure CTP already finish startup process */
	msleep(FT_STARTUP_DLY);

	/*get some register information */
	reg_addr = FT5X06_REG_FW_VER;
	for(k=0; k<FAILURE_COUNTS; k++){   //sym add for compatible with gt818x
	err = ft5x06_i2c_read(client, &reg_addr, 1, &reg_value, 1);
	if (err<0){
		
			dev_err(&client->dev, "version read failed");
			if (k==FAILURE_COUNTS-1)
				return -1;
			else
				continue;
		}
	else
		break;
	}
	dev_info(&client->dev, "[FTS] Firmware version = 0x%x\n", reg_value);
	current_fw_version  = reg_value;
	ic_firmware_version = reg_value;
	pr_err("[FTS] Firmware version = 0x%x\n", reg_value);
	msleep(1);
	
	reg_addr = FT5X06_REG_POINT_RATE;
	for(k=0; k<FAILURE_COUNTS; k++){
	err = ft5x06_i2c_read(client, &reg_addr, 1, &reg_value, 1);
	if (err<0){
	
			dev_err(&client->dev, "report rate read failed");
			if (k==FAILURE_COUNTS-1)
				return -1;
			else
				continue;
		}
	else
		break;
	}
	dev_info(&client->dev, "[FTS] report rate is %dHz.\n", reg_value * 10);
	msleep(1);
	
	reg_addr = FT5X06_REG_THGROUP;
	for(k=0; k<FAILURE_COUNTS; k++){
	err = ft5x06_i2c_read(client, &reg_addr, 1, &reg_value, 1);	
	if (err<0){
			dev_err(&client->dev, "threshold read failed");
			if (k==FAILURE_COUNTS-1)
				return -1;
			else
				continue;
		}
	else
		break;
	}
	dev_dbg(&client->dev, "[FTS] touch threshold is %d.\n", reg_value * 4);
	msleep(1);

	
		//get only ID 
	err = ft5x06_read_reg(FT5X06_REG_ONLY_ID, &only_id);
	printk("FT5X06_REG_ONLY_ID: 0x%x,err =%d\n", only_id,err);

	err = ft5x06_read_reg(FT5X06_REG_UPGRADE_ID, &upgrade_id);
		printk("FT5X06_REG_UPGRADE_ID: 0x%x,err =%d\n", upgrade_id,err);

	ft5x06_read_reg(FT5X0X_REG_FT5201ID, &vendor_id);
	printk("Firmware Upgrade begin: manufacture ID: 0x%x\n", vendor_id);


	//compatible with tp which the firmware is not updated before and the 0xb6 is not existed
	err = 0;	
	if ((upgrade_id == 0x66 && only_id == 0x04) || (upgrade_id == 0x55 && only_id == 0xea)) 
		reg_value = 0x10;
	else if (only_id == 0x12 && vendor_id == 0x5a && is_rgtk_product(RGTK_PRODUCT_Q801))
		reg_value = 0x10;
	else 
		reg_value = 0x15;
	
	for(k=0; k<10; k++){
		if (reg_value == 0x10 || reg_value == 0x15){
			err = ft5x06_write_reg(FT5X06_REG_THGROUP, reg_value);
			if (err < 0){
				msleep(1);
				printk("write FT5X06_REG_THGROUP  0x%x err\n",reg_value);
			}
			else
				printk("write FT5X06_REG_THGROUP  0x%x success\n",reg_value);
		}
		else 
			break;
	}

	err = ft5x06_write_reg(FT5X06_REG_POINT_RATE, 0x08);
	if (err <0)
		printk("write FT5X06_REG_POINT_RATE 0x08 err\n");
	msleep(1);

#ifdef CONFIG_SUPPORT_FTS_CTP_UPG
		msleep(400);
		err = fts_ctpm_fw_upgrade_with_i_file();
	//wake the CTPM
		gpio_set_value(pdata->reset_gpio, 0);	//set wake = 0,base on system
		msleep(100);
		gpio_set_value(pdata->reset_gpio, 1);	//set wake = 1,base on system
		msleep(100);
	//	ft5x06_write_reg(0x88, 0x05);	//5, 6,7,8
	//	ft5x06_write_reg(0x80, 30);
		msleep(50);
		ft5x06_read_reg(FT5X06_REG_FW_VER, &reg_value);
		printk("FT5X06 firmware version ID: 0x%x\n", reg_value);
		printk("Firmware Upgrade finish\n");

		if (err == UPGRADE_OK)
		{
			printk("Firmware Upgrade finish\n");
			refresh_tp_info();
			printk("TP firmware update sucess booting: \n after update  fw version :0x%x, vendor id : 0x%x, only id : 0x%x\n", \
					ic_firmware_version, vendor_id, only_id);
		}
#endif

	if (1 == isUpdate)
		current_fw_version = refresh_fw_verson();
	
#if defined(ENABLE_TP_FIRMWARE_DECT_ENABLE_CIT) && ENABLE_TP_FIRMWARE_DECT_ENABLE_CIT
		tp_fm_dect_enable_cit = 1;
	// dectect TP firmware effect or not , by anxiang.xiao 2013-01-17
		support_tps_per_customer = sizeof(tps_fm_info) / sizeof(struct tp_firmware_info);
		printk("\ncurrent customer support TP nums : %d\n", support_tps_per_customer);
		for (i = 0; i < support_tps_per_customer; i++)
		{
			if (tps_fm_info[i].verndor_id == vendor_id && tps_fm_info[i].only_id == only_id && 
				tps_fm_info[i].firmware_version == current_fw_version )	
					is_tp_fw_effect = 1;
				
		}
	//end 
#endif


	 // add for get TP info begin, by anxiang.xiao
	if (Vendor_Id_XinHaoTP == vendor_id)
		len = sprintf(tp_info, "%s, %s(0x%x),%s(0x%x),0x%x,0x%x,%d,%d,","ft5x06","XinHaoTP",vendor_id, "unkown model style",only_id, current_fw_version,client->addr,is_tp_fw_effect, tp_fm_dect_enable_cit);
	else if (Vendor_Id_XinLiTP == vendor_id)
		len = sprintf(tp_info, "%s, %s(0x%x),%s(0x%x),0x%x,0x%x,%d,%d,","ft5x06","XinLiTP",vendor_id, "unkown model style",only_id, current_fw_version,client->addr,is_tp_fw_effect, tp_fm_dect_enable_cit);
	else if (Vendor_Id_DiJinTP == vendor_id)
		len = sprintf(tp_info, "%s, %s(0x%x),%s(0x%x),0x%x,0x%x,%d,%d,","ft5x06","DiJinTP",vendor_id, "unkown model style",only_id, current_fw_version,client->addr,is_tp_fw_effect, tp_fm_dect_enable_cit);
	else if (Vendor_Id_WangDeTP == vendor_id)
		len = sprintf(tp_info, "%s, %s(0x%x),%s(0x%x),0x%x,0x%x,%d,%d,","ft5x06","WangDeTP",vendor_id, "unkown model style",only_id, current_fw_version,client->addr, is_tp_fw_effect,tp_fm_dect_enable_cit);
	else if (Vendor_Id_HuaXinDaTP == vendor_id)
		len = sprintf(tp_info, "%s, %s(0x%x),%s(0x%x),0x%x,0x%x,%d,%d,","ft5x06","HuaXinDaTP",vendor_id, "unkown model style",only_id, current_fw_version,client->addr, is_tp_fw_effect,tp_fm_dect_enable_cit);
	else
		len = sprintf(tp_info, "%s, %s(0x%x),%s(0x%x),0x%x,0x%x,%d,%d,","ft5x06","others",vendor_id, "others",only_id, current_fw_version,client->addr,is_tp_fw_effect, tp_fm_dect_enable_cit);


	fix_tp_proc_info(tp_info, len);
	printk("\nHHHHHHHH  tp info = %s HHHHHHHHH\n", tp_info);
      // add for get TP info end


	ft5x06_virtualkey_set();
	
	err = sysfs_create_file(ft5x06_virtual_key_properties_kobj, &ft5x06_fw_update_attr.attr);

	err = request_threaded_irq(client->irq, NULL,
				   ft5x06_ts_interrupt, pdata->irqflags,
				   client->dev.driver->name, data);
	if (err) {
		dev_err(&client->dev, "request irq failed\n");
		goto free_reset_gpio;
	}
#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN +
	    FT5X06_SUSPEND_LEVEL;
	data->early_suspend.suspend = ft5x06_ts_early_suspend;
	data->early_suspend.resume = ft5x06_ts_late_resume;
	register_early_suspend(&data->early_suspend);
#endif

	return 0;

free_reset_gpio:
	if (gpio_is_valid(pdata->reset_gpio))
		gpio_free(pdata->reset_gpio);
free_irq_gpio:
	if (gpio_is_valid(pdata->irq_gpio))
		gpio_free(pdata->reset_gpio);
pwr_off:
	if (pdata->power_on)
		pdata->power_on(false);
	else
		ft5x06_power_on(data, false);
pwr_deinit:
	if (pdata->power_init)
		pdata->power_init(false);
	else
		ft5x06_power_init(data, false);
unreg_inputdev:
	input_unregister_device(input_dev);
	input_dev = NULL;
free_inputdev:
	input_free_device(input_dev);
free_mem:
	kfree(data);
	return err;
}

static int __devexit ft5x06_ts_remove(struct i2c_client *client)
{
	struct ft5x06_ts_data *data = i2c_get_clientdata(client);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&data->early_suspend);
#endif
	free_irq(client->irq, data);

	if (gpio_is_valid(data->pdata->reset_gpio))
		gpio_free(data->pdata->reset_gpio);

	if (gpio_is_valid(data->pdata->irq_gpio))
		gpio_free(data->pdata->reset_gpio);

	if (data->pdata->power_on)
		data->pdata->power_on(false);
	else
		ft5x06_power_on(data, false);

	if (data->pdata->power_init)
		data->pdata->power_init(false);
	else
		ft5x06_power_init(data, false);

	input_unregister_device(data->input_dev);
	kfree(data);

	return 0;
}

static const struct i2c_device_id ft5x06_ts_id[] = {
	{"ft5x06_ts", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, ft5x06_ts_id);

static struct i2c_driver ft5x06_ts_driver = {
	.probe = ft5x06_ts_probe,
	.remove = __devexit_p(ft5x06_ts_remove),
	.driver = {
		   .name = "ft5x06_ts",
		   .owner = THIS_MODULE,
#ifdef CONFIG_PM
		   .pm = &ft5x06_ts_pm_ops,
#endif
		   },
	.id_table = ft5x06_ts_id,
};

static int __init ft5x06_ts_init(void)
{
	return i2c_add_driver(&ft5x06_ts_driver);
}
module_init(ft5x06_ts_init);

static void __exit ft5x06_ts_exit(void)
{
	i2c_del_driver(&ft5x06_ts_driver);
}
module_exit(ft5x06_ts_exit);

MODULE_DESCRIPTION("FocalTech ft5x06 TouchScreen driver");
MODULE_LICENSE("GPL v2");
