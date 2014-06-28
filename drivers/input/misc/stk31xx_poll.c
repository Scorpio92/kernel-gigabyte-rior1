/*
 *  stk31xx_int.c - Linux kernel modules for proximity/ambient light sensor
 *  (Polling + Switch Mode)
 *
 *  Copyright (C) 2011 Patrick Chang / SenseTek <patrick_chang@sitronix.com.tw>
 *  Copyright (C) 2012 Lex Hsieh     / SenseTek <lex_hsieh@sitronix.com.tw>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/poll.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <linux/kthread.h>
#include <linux/errno.h>
#include <linux/wakelock.h>
#include "linux/stk31xx.h"
#include "linux/stk_defines.h"
#include "linux/stk_lk_defs.h"
#if (defined(STK_MANUAL_GREYCARD_CALI) || defined(STK_AUTO_CT_CALI_SATU))
#include   <linux/fs.h>   
#include  <asm/uaccess.h> 
#endif
#ifdef SITRONIX_PERMISSION_THREAD
#include <linux/fcntl.h>
#include <linux/syscalls.h>
#endif 	//	#ifdef SITRONIX_PERMISSION_THREAD

#define STKALS_DRV_NAME	"stk_als"
#define STKPS_DRV_NAME "stk_ps"
#define DEVICE_NAME		"stk-oss"
#define DRIVER_VERSION  STK_DRIVER_VER
#define LightSensorDevName "stk_als"
#define ProximitySensorDevName "stk_ps"

#define ALS_ODR_DELAY 400
#define PS_ODR_DELAY 20

#define stk_writew i2c_smbus_write_word_data
#define stk_readw i2c_smbus_read_word_data

#define STK_LOCK0 mutex_unlock(&stkps_io_lock)
#define STK_LOCK1 mutex_lock(&stkps_io_lock)

static int32_t init_all_setting(struct stk31xx_platform_data* plat_data);
static int32_t enable_ps(uint8_t enable);
static int32_t enable_als(uint8_t enable);
static int32_t software_reset(void);

static int32_t set_als_it(uint8_t it);
static int32_t set_als_gain(uint8_t gain);
static int32_t set_ps_it(uint8_t it);
static int32_t set_ps_slp(uint8_t slp);
static int32_t set_ps_led_driving_current(uint8_t irdr);
static int32_t set_ps_gc(uint8_t gc);
static int32_t set_ps_thd_l(uint8_t thd_l);
static int32_t set_ps_thd_h(uint8_t thd_h);

static int ps_polling_function(void* arg);
static int als_polling_function(void* arg);
static struct task_struct *ps_polling_tsk=NULL;
static struct task_struct *als_polling_tsk=NULL;

static struct mutex stkps_io_lock;
static struct completion ps_thread_completion;
static struct completion als_thread_completion;
static struct stkps31xx_data* pStkPsData = NULL;
static struct wake_lock proximity_sensor_wakelock;
static struct wake_lock ps_poll_wakelock;
static uint8_t ps_code_low_thd;
static uint8_t ps_code_high_thd;
static int32_t als_transmittance;

inline uint32_t alscode2lux(uint32_t alscode)
{
   alscode += ((alscode<<7)+(alscode<<3)+(alscode>>1));     // 137.5
      //x1       //   x128         x8            x0.5
    alscode<<=3; // x 8 (software extend to 19 bits)
    // Gain & IT setting ==> x8
    // ==> i.e. code x 8800
    // Org : 1 code = 0.88 Lux
    // 8800 code = 0.88 lux --> this means it must be * 1/10000

    alscode/=als_transmittance;
    return alscode;
}

static int32_t init_all_setting(struct stk31xx_platform_data* plat_data)
{
    if (software_reset()<0)
    {
        ERR("STK PS : error --> device not found\n");
        return 0;
    }
	pStkPsData->ps_cmd_reg = plat_data->ps_cmd;
	pStkPsData->als_cmd_reg = plat_data->als_cmd;
	pStkPsData->ps_gc_reg = plat_data->ps_gain;
	
    enable_ps(0);
    enable_als(0);
    set_ps_slp((plat_data->ps_cmd & STK_PS_CMD_SLP_MASK) >> STK_PS_CMD_SLP_SHIFT);
    set_ps_gc(plat_data->ps_gain);
    set_ps_it((plat_data->ps_cmd & STK_PS_CMD_IT_MASK) >> STK_PS_CMD_IT_SHIFT);
    set_ps_led_driving_current((plat_data->ps_cmd & STK_PS_CMD_DR_MASK) >> STK_PS_CMD_DR_SHIFT);
    set_als_gain((plat_data->als_cmd & STK_ALS_CMD_GAIN_MASK) >> STK_ALS_CMD_GAIN_SHIFT); 
    set_als_it((plat_data->als_cmd & STK_ALS_CMD_IT_MASK) >> STK_ALS_CMD_IT_SHIFT); 
    set_ps_thd_h(plat_data->ps_high_thd);
    set_ps_thd_l(plat_data->ps_low_thd);
#if (defined(STK_MANUAL_GREYCARD_CALI) || defined(STK_AUTO_CT_CALI_SATU) || defined(STK_AUTO_CT_CALI_NO_SATU))
	pStkPsData->ps_cali_done = false;
	pStkPsData->cali_file_exist = false;
	pStkPsData->ps_ctk_val = 0;
#endif	
#if (defined(STK_MANUAL_GREYCARD_CALI) || defined(STK_AUTO_CT_CALI_SATU))
	pStkPsData->first_boot = true;	
#endif
    return 1;
}

static int32_t software_reset(void)
{
    // software reset and check stk 83xx is valid
    int32_t r;
    uint8_t w_reg;
    uint8_t org_reg;

    r = i2c_smbus_read_byte_data(pStkPsData->client,STK_PS_GC_REG);
    if (r<0)
    {
        ERR("STK PS software reset: read i2c error\n");
        return r;
    }
    org_reg = (uint8_t)(r&0xf0);
    w_reg = ~((uint8_t)(r&0xff));
    r = i2c_smbus_write_byte_data(pStkPsData->client,STK_PS_GC_REG,w_reg);
    if (r<0)
    {
        ERR("STK PS software reset: write i2c error\n");
        return r;
    }
    r = i2c_smbus_read_byte_data(pStkPsData->client,STK_PS_GC_REG);
    if (w_reg!=(uint8_t)(r&0xff))
    {
        ERR("STK PS software reset: read-back value is not  the same\n");
        return -1;
    }
    r = i2c_smbus_write_byte_data(pStkPsData->client,STK_PS_SOFTWARE_RESET_REG,0);
    msleep(5);
    if (r<0)
    {
        ERR("STK PS software reset: read error after reset\n");
        return r;
    }
    return 0;
}



static int32_t set_als_it(uint8_t it)
{
    pStkPsData->als_cmd_reg &= (~STK_ALS_CMD_IT_MASK);
    pStkPsData->als_cmd_reg |= (STK_ALS_CMD_IT_MASK & STK_ALS_CMD_IT(it));
    return i2c_smbus_write_byte_data(pStkPsData->client,STK_ALS_CMD_REG,pStkPsData->als_cmd_reg);

}
static int32_t set_als_gain(uint8_t gain)
{
	if(gain >= 2)
	{
		INFO("STK PS : als_gain = %d\n", gain);
	}
    pStkPsData->als_cmd_reg &= (~STK_ALS_CMD_GAIN_MASK);
    pStkPsData->als_cmd_reg |= (STK_ALS_CMD_GAIN_MASK & STK_ALS_CMD_GAIN(gain));
    return i2c_smbus_write_byte_data(pStkPsData->client,STK_ALS_CMD_REG,pStkPsData->als_cmd_reg);
}
static int32_t set_ps_it(uint8_t it)
{
    pStkPsData->ps_cmd_reg &= (~STK_PS_CMD_IT_MASK);
    pStkPsData->ps_cmd_reg |= (STK_PS_CMD_IT_MASK & STK_PS_CMD_IT(it));
    return i2c_smbus_write_byte_data(pStkPsData->client,STK_PS_CMD_REG,pStkPsData->ps_cmd_reg);
}
static int32_t set_ps_slp(uint8_t slp)
{
    pStkPsData->ps_cmd_reg &= (~STK_PS_CMD_SLP_MASK);
    pStkPsData->ps_cmd_reg |= (STK_PS_CMD_SLP_MASK & STK_PS_CMD_SLP(slp));
    return i2c_smbus_write_byte_data(pStkPsData->client,STK_PS_CMD_REG,pStkPsData->ps_cmd_reg);

}
static int32_t set_ps_led_driving_current(uint8_t irdr)
{
    pStkPsData->ps_cmd_reg &= (~STK_PS_CMD_DR_MASK);
    pStkPsData->ps_cmd_reg |= (STK_PS_CMD_DR_MASK & STK_PS_CMD_DR(irdr));
    return i2c_smbus_write_byte_data(pStkPsData->client,STK_PS_CMD_REG,pStkPsData->ps_cmd_reg);
}
static int32_t set_ps_gc(uint8_t gc)
{
    int32_t retval;

    retval = i2c_smbus_read_byte_data(pStkPsData->client,STK_PS_GC_REG);
    if (retval<0)
        return retval;
    pStkPsData->ps_gc_reg = (uint8_t)retval;
    pStkPsData->ps_gc_reg &= (~STK_PS_GC_GAIN_MASK);
    pStkPsData->ps_gc_reg |= (STK_PS_GC_GAIN(gc)&STK_PS_GC_GAIN_MASK);

    return i2c_smbus_write_byte_data(pStkPsData->client,STK_PS_GC_REG,pStkPsData->ps_gc_reg);
}


static int32_t set_ps_thd_l(uint8_t thd_l)
{
    ps_code_low_thd = thd_l;
	return 0;
}
static int32_t set_ps_thd_h(uint8_t thd_h)
{
    ps_code_high_thd = thd_h;
    return 0;
}

static inline int32_t get_als_reading(void)
{
    int32_t word_data, tmp_word_data;
	// make sure MSB and LSB are the same data set
	tmp_word_data = i2c_smbus_read_word_data(pStkPsData->client, STK_ALS_DT1_REG);
	if(tmp_word_data < 0)
	{
		ERR("STK PS :%s fail, err=0x%x\n", __func__, tmp_word_data);
		return tmp_word_data;	   
	}
	else
	{
		word_data = ((tmp_word_data & 0xFF00) >> 8) | ((tmp_word_data & 0x00FF) << 8) ;
		//INFO("%s: word_data=0x%4x\n", __func__, word_data);
    return word_data;
}
}

static inline int32_t get_ps_reading(void)
{
	int32_t ps;
	ps = i2c_smbus_read_byte_data(pStkPsData->client,STK_PS_READING_REG);
    if(ps < 0)
	{
		ERR("STK PS :%s fail, err=0x%x\n", __func__, ps);
		return -EINVAL;
	}
	return ps;	
}

#if (defined(STK_AUTO_CT_CALI_NO_SATU) || defined(STK_AUTO_CT_CALI_SATU) || defined(STK_MANUAL_GREYCARD_CALI))
#ifdef SITRONIX_PERMISSION_THREAD
SYSCALL_DEFINE3(fchmodat, int, dfd, const char __user *, filename, mode_t, mode);
static struct task_struct *STKPermissionThread = NULL;
static int stk_permission_thread_delay = 5000;

static int stk_permission_thread(void *data)
{
	int ret = 0;
	int retry = 0;
	mm_segment_t fs = get_fs();
	set_fs(KERNEL_DS);
	
	INFO("%s start\n", __func__);
	do{
		//INFO("delay %d ms\n", stk_permission_thread_delay);
		msleep(stk_permission_thread_delay);
		ret = sys_fchmodat(AT_FDCWD, "/sys/devices/platform/stk-oss/ps_cali" , 0666);
		if(ret < 0)
		printk("fail to execute sys_fchmodat, ret = %d\n", ret);
		if(retry++ > 10)
		break;
	}while(ret == -ENOENT);
	set_fs(fs);
	INFO("%s exit, retry=%d\n", __func__, retry);
	return 0;
}
#endif // SITRONIX_PERMISSION_THREAD
static int32_t get_ps_cali_file(char * r_buf, int8_t buf_size)
{
	struct file  *cali_file;
	mm_segment_t fs;	
	ssize_t ret;
	
    cali_file = filp_open(STK_CALI_FILE, O_RDWR,0);
    if(IS_ERR(cali_file))
	{
        ERR("STK PS %s: filp_open error!\n", __func__);
        return -ENOENT;
	}
	else
	{
		fs = get_fs();
		set_fs(get_ds());
		ret = cali_file->f_op->read(cali_file,r_buf,buf_size,&cali_file->f_pos);
		if(ret < 0)
		{
			ERR("STK PS %s: read error, ret=%d\n", __func__, ret);
			filp_close(cali_file,NULL);
			return -EIO;
		}		
		set_fs(fs);
    }
	
    filp_close(cali_file,NULL);	
	return 0;	
}


static int32_t set_ps_cali_file(char *w_buf, int8_t w_buf_size)
{
	struct file  *cali_file;
	char r_buf[STK_CALI_FILE_SIZE] = {0};	
	mm_segment_t fs;	
	ssize_t ret;
	int8_t i;
	
    cali_file = filp_open(STK_CALI_FILE, O_CREAT | O_RDWR,0666);
    if(IS_ERR(cali_file))
	{
        ERR("STK PS %s: filp_open for write error!\n", __func__);
        return -ENOENT;
	}
	else
	{
		fs = get_fs();
		set_fs(get_ds());
		
		ret = cali_file->f_op->write(cali_file,w_buf,w_buf_size,&cali_file->f_pos);
		if(ret != w_buf_size)
		{
			ERR("STK PS %s: write error!\n", __func__);
			filp_close(cali_file,NULL);
			return -EIO;
		}
		cali_file->f_pos=0x00;
		ret = cali_file->f_op->read(cali_file ,r_buf ,w_buf_size ,&cali_file->f_pos);
		if(ret < 0)
		{
			ERR("STK PS %s: read error!\n", __func__);
			filp_close(cali_file,NULL);
			return -EIO;
		}		
		set_fs(fs);
		for(i=0;i<w_buf_size;i++)
		{
			if(r_buf[i] != w_buf[i])
			{
				ERR("STK PS %s: read back error!, r_buf[%d](0x%x) != w_buf[%d](0x%x)\n", __func__, i, r_buf[i], i, w_buf[i]);
				filp_close(cali_file,NULL);
				return -EIO;
			}
		}
		
    }
    filp_close(cali_file,NULL);	
	pStkPsData->cali_file_exist = true;	
	INFO("STK PS %s successfully\n", __func__);
	return 0;
}

static int32_t get_ps_cali_thd(int16_t *ps_thd)
{
	char r_buf[STK_CALI_FILE_SIZE] = {0};
	int32_t ret;
	pStkPsData->cali_file_exist = false;
	
	if ((ret = get_ps_cali_file(r_buf, STK_CALI_FILE_SIZE)) < 0)
	{
			return ret;
	}
	else
	{
		if(r_buf[0] == STK_CALI_VER0 && r_buf[1] == STK_CALI_VER1)
		{
			ps_thd[STK_HIGH_THD] = r_buf[2];
			ps_thd[STK_LOW_THD] = r_buf[3];
			pStkPsData->ps_ctk_val = r_buf[4];
			pStkPsData->ps_cali_done = true;		
			pStkPsData->cali_file_exist = true;
		}
		else
		{
			ERR("STK PS %s: cali version number error! r_buf=0x%x,0x%x,0x%x,0x%x,0x%x\n", __func__, r_buf[0], r_buf[1], r_buf[2], r_buf[3], r_buf[4]);						
			return -EINVAL;
		}
	}
	return 0;
}


static void set_ps_thd_to_file(int16_t ps_thd_data[], int16_t ps_ave)
{
	char w_buf[STK_CALI_FILE_SIZE] = {0};	
	
	w_buf[0] = STK_CALI_VER0;
	w_buf[1] = STK_CALI_VER1;
	w_buf[2] = ps_thd_data[STK_HIGH_THD];			
	w_buf[3] = ps_thd_data[STK_LOW_THD];			
	w_buf[4] = ps_ave;			
	set_ps_cali_file(w_buf, sizeof(char)*STK_CALI_FILE_SIZE);			
}

static int32_t limit_thd_range(int16_t ps_thd_data[])
{
	int32_t ret = 0;
	if(ps_thd_data[STK_HIGH_THD] > STK_THD_H_MAX)
	{
		ps_thd_data[STK_HIGH_THD] = STK_THD_H_MAX; 
		ret |= 0x01;
	}
	else if(ps_thd_data[STK_HIGH_THD] < STK_THD_H_MIN)
{
		ps_thd_data[STK_HIGH_THD] = STK_THD_H_MIN; 
		ret |= 0x02;
	}
	if(ps_thd_data[STK_LOW_THD] > STK_THD_L_MAX)
{
		ps_thd_data[STK_LOW_THD] = STK_THD_L_MAX; 
		ret |= 0x04;
	}
	else if(ps_thd_data[STK_LOW_THD] < STK_THD_L_MIN)
	{
		ps_thd_data[STK_LOW_THD] = STK_THD_L_MIN; 	
		ret |= 0x08;	
	}	

	return ret;
}

static int32_t ct_thd(int16_t ps_stat_data[], int16_t ps_thd_data[])
{
	int16_t CTM, CTA, CTK, offset;
	
	CTA = ps_stat_data[STK_DATA_AVE];
	offset = STK_A_OFFSET;
	if( pStkPsData->cali_file_exist == true )
	{
		CTK = pStkPsData->ps_ctk_val;
		offset = STK_K_OFFSET;
	}
	else
		CTK = STK_DEF_CTK;
	if( CTK > STK_CTK_MAX )
		CTK = STK_CTK_MAX;
	if( (CTK+offset) > CTA )
		CTM = CTA;
	else
		CTM = (CTK+offset);
	if( CTK > CTM )
		CTM = CTK;
	if( CTM > STK_CTA_MAX )
		CTM = STK_CTA_MAX;
	
	ps_thd_data[STK_HIGH_THD] = CTM + STK_THD_H_ABOVE_CT;	
	ps_thd_data[STK_LOW_THD] = CTM + STK_THD_L_ABOVE_CT;	
	limit_thd_range(ps_thd_data);					
	printk("PS_HT=0x%x, PS_LT=0x%x, CTA=0x%x, CTK=0x%x, CTM=0x%x\n", 
		ps_thd_data[STK_HIGH_THD], ps_thd_data[STK_LOW_THD], CTA, CTK, CTM);	
	pStkPsData->ps_cali_done = true;
	
	return 0;
}

static int32_t calc_ct_cali_thd(int16_t ps_stat_data[], int16_t ps_thd_data[])
{
	ps_thd_data[STK_HIGH_THD] = ps_stat_data[STK_DATA_AVE]*STK_CT_NUMERATOR/STK_CT_DENOMINATOR + STK_THD_H_ABOVE_CT;	
	ps_thd_data[STK_LOW_THD] = ps_stat_data[STK_DATA_AVE]*STK_CT_NUMERATOR/STK_CT_DENOMINATOR + STK_THD_L_ABOVE_CT;	
	limit_thd_range(ps_thd_data);					
	printk("STK PS : PS_HT=0x%x, PS_LT=0x%x, Ave_CT=0x%x\n", 
		ps_thd_data[STK_HIGH_THD], ps_thd_data[STK_LOW_THD], ps_stat_data[STK_DATA_AVE]);	
	pStkPsData->ps_cali_done = true;
	return 0;
}
#endif 	/*	#if (defined(STK_AUTO_CT_CALI_NO_SATU) || defined(STK_AUTO_CT_CALI_SATU) || defined(STK_MANUAL_GREYCARD_CALI))	*/

#ifdef STK_MANUAL_GREYCARD_CALI
static int32_t calc_greycard_cali_thd(int16_t ps_stat_data[], int16_t ps_thd_data[])
{
	ps_thd_data[STK_LOW_THD] = ps_stat_data[STK_DATA_MIN] - STK_DIFF_GREY_N_THD_L;
	ps_thd_data[STK_HIGH_THD] = ps_stat_data[STK_DATA_MIN] - STK_DIFF_GREY_N_THD_H;	
	limit_thd_range(ps_thd_data);		
	printk("STK PS : PS_HT=0x%x, PS_LT=0x%x, min_ps=0x%x\n", 
		ps_thd_data[STK_HIGH_THD], ps_thd_data[STK_LOW_THD], ps_stat_data[STK_DATA_MIN]);							
	pStkPsData->ps_cali_done = true;
	return 0;
}


static int32_t judge_grey_card_cali(int16_t ps_stat_data[])
{	
	if(ps_stat_data[STK_DATA_MIN] < STK_MIN_GREY_PS_DATA)
		{
		ERR("STK PS : Min PS data:%d, PS reading is too small\n", ps_stat_data[STK_DATA_MIN]);	
		return -2;				
	}
	return 0;
}
#endif	/* #ifdef STK_MANUAL_GREYCARD_CALI */

#if defined(STK_AUTO_CT_CALI_NO_SATU)
static int32_t judge_ct_cali_nosatu(int16_t ps_thd_data[])
{
	if(ps_thd_data[STK_LOW_THD] > ps_code_low_thd)
	{
		ERR("STK PS : new LT(%d) is larger than old LT(%d)\n", ps_thd_data[STK_LOW_THD], ps_code_low_thd);	
		return -1;				
	}	
	return 0;
}
#endif	/*	#if defined(STK_AUTO_CT_CALI_NO_SATU)	*/

#if (defined(STK_MANUAL_GREYCARD_CALI) || defined(STK_AUTO_CT_CALI_NO_SATU) || defined(STK_AUTO_CT_CALI_SATU))
static void set_ps_thd_to_driver(int16_t ps_thd_data[])
{
	set_ps_thd_h(ps_thd_data[STK_HIGH_THD]);
	set_ps_thd_l(ps_thd_data[STK_LOW_THD]);
}

static int32_t get_several_ps_data(int16_t ps_stat_data[])
{
	int32_t ps_data;
	int8_t data_count = 0;	
	uint16_t sample_time_ps = 0;		
	int32_t ave_ps_int32 = 0;
	int32_t ret;
	ps_stat_data[STK_DATA_MAX] = 0;
	ps_stat_data[STK_DATA_MIN] = 9999;
	ps_stat_data[STK_DATA_AVE] = 0;
	
	pStkPsData->ps_cmd_reg &= (~STK_PS_CMD_SD_MASK);
	pStkPsData->ps_cmd_reg |= STK_PS_CMD_SD(0);
	ret = i2c_smbus_write_byte_data(pStkPsData->client,STK_PS_CMD_REG,pStkPsData->ps_cmd_reg);	
	if (0 != ret)
	{	
		ERR("STK PS %s: i2c_smbus_write_byte_data fail\n", __func__);
		return ret;
	}
	
	sample_time_ps = pStkPsData->ps_cmd_reg;
	sample_time_ps&=STK_PS_CMD_SLP_MASK;
	sample_time_ps>>=STK_PS_CMD_SLP_SHIFT;	
	sample_time_ps = cali_sample_time_table[sample_time_ps];
	
	while(data_count < STK_CALI_SAMPLE_NO)
	{
		msleep(sample_time_ps);	
		ps_data = get_ps_reading();
		//INFO("STK PS %s: ps_data %d=%d\n", __func__, data_count, ps_data);
		if(ps_data < 0)
		{
			pStkPsData->ps_cmd_reg &= (~STK_PS_CMD_SD_MASK);
			pStkPsData->ps_cmd_reg |= STK_PS_CMD_SD(1);
			ret = i2c_smbus_write_byte_data(pStkPsData->client,STK_PS_CMD_REG,pStkPsData->ps_cmd_reg);	
			if (0 != ret)
			{	
				ERR("STK PS %s: i2c_smbus_write_byte_data fail\n", __func__);
				return ret;
			}
			
			return ps_data;
		}
		
		ave_ps_int32 +=  ps_data;			
		if(ps_data > ps_stat_data[STK_DATA_MAX])
			ps_stat_data[STK_DATA_MAX] = ps_data;
		if(ps_data < ps_stat_data[STK_DATA_MIN])
			ps_stat_data[STK_DATA_MIN] = ps_data;						
		data_count++;	
	}	
	ave_ps_int32 /= STK_CALI_SAMPLE_NO;	
	ps_stat_data[STK_DATA_AVE] = (int16_t)ave_ps_int32;
	/* force disable after cali to make sure interrupt is correct*/
	pStkPsData->ps_cmd_reg &= (~STK_PS_CMD_SD_MASK);
	pStkPsData->ps_cmd_reg |= STK_PS_CMD_SD(1);
	ret = i2c_smbus_write_byte_data(pStkPsData->client,STK_PS_CMD_REG,pStkPsData->ps_cmd_reg);	
	if (0 != ret)
	{	
		ERR("STK PS %s: i2c_smbus_write_byte_data fail\n", __func__);
		return ret;
	}
	
	return 0;
}
	

static int32_t set_ps_cali(uint8_t force_cali)
{
	int16_t ps_statistic_data[3] = {0};
	int32_t ret = 0;
	int16_t ps_thd_data[2] = {150, 140};
	
	ret = get_several_ps_data(ps_statistic_data);
	if(ret < 0)
	{
		ERR("STK PS %s: get_several_ps_data failed, ret=%d\n", __func__, ret);
		ERR("STK PS %s: calibration fail\n", __func__);
		return ret;
	}		
#ifdef STK_AUTO_CT_CALI_SATU
	calc_ct_cali_thd(ps_statistic_data, ps_thd_data);
	ct_thd(ps_statistic_data, ps_thd_data);
	set_ps_thd_to_driver(ps_thd_data);	
	if(force_cali)
		set_ps_thd_to_file(ps_thd_data, ps_statistic_data[STK_DATA_AVE]);	
#elif defined(STK_MANUAL_GREYCARD_CALI)
	if(force_cali)
	{
	ret = judge_grey_card_cali(ps_statistic_data);
	if(ret == 0)
	{
		ret = calc_greycard_cali_thd(ps_statistic_data, ps_thd_data);
		if(ret < 0)
		{
			ERR("STK PS %s: calibration fail, errno=%d\n", __func__, ret);
			return ret;
		}		
		set_ps_thd_to_driver(ps_thd_data);	
			set_ps_thd_to_file(ps_thd_data, ps_statistic_data[STK_DATA_AVE]);
		printk("STK PS : PS calibration was done successfully\n");			
	}
	else
		ERR("STK PS %s: calibration fail, errno=%d\n", __func__, ret);	
	}
	else
	{
		calc_ct_cali_thd(ps_statistic_data, ps_thd_data);
		set_ps_thd_to_driver(ps_thd_data);	
	}
#elif defined(STK_AUTO_CT_CALI_NO_SATU)
	calc_ct_cali_thd(ps_statistic_data, ps_thd_data);
	if(judge_ct_cali_nosatu(ps_thd_data) == 0)
	{		
		set_ps_thd_to_driver(ps_thd_data);				
	}
#endif
	return ret;
}
#endif	/* #if (defined(STK_MANUAL_GREYCARD_CALI) || defined(STK_AUTO_CT_CALI_NO_SATU) || defined(STK_AUTO_CT_CALI_SATU)) */

inline void als_report_event(struct input_dev* dev,int32_t report_value)
{
	input_report_abs(dev, ABS_MISC, report_value);
	input_sync(dev);
	INFO("STK PS : als input event %d lux\n",report_value);
}

inline void update_and_check_report_als(int32_t als_code)
{
    int32_t lux_last,lux;
    lux_last = pStkPsData->als_lux_last;
    lux = alscode2lux(als_code);
    if (unlikely(abs(lux - lux_last)>=CONFIG_STK_ALS_CHANGE_THRESHOLD))
    {
        pStkPsData->als_lux_last = lux;
        als_report_event(pStkPsData->als_input_dev,lux);
    }
}

static int als_polling_function(void* arg)
{
    uint32_t delay;
    init_completion(&als_thread_completion);
    while (1)
    {
		STK_LOCK(1);
		delay = pStkPsData->als_delay;
		STK_LOCK(0);
		msleep(delay);		/* wait for IT */
		
        STK_LOCK(1);        
        pStkPsData->als_reading = get_als_reading();
		if(pStkPsData->als_reading < 0)
		{
			ERR("STK PS %s: get_als_reading failed, ret=%d", __func__, pStkPsData->als_reading);
			STK_LOCK(0);  
			continue;
		}
        update_and_check_report_als(pStkPsData->als_reading);
        if (pStkPsData->bALSThreadRunning == 0)
            break;
        STK_LOCK(0);        
    };

    STK_LOCK(0);
    complete(&als_thread_completion);

    return 0;
}


inline void ps_report_event(struct input_dev* dev,int32_t report_value)
{    
    pStkPsData->ps_distance_last = report_value;
	input_report_abs(dev, ABS_DISTANCE, report_value);
	input_sync(dev);
	INFO("STK PS : ps input event %d\n",report_value);
	wake_lock_timeout(&proximity_sensor_wakelock, 2*HZ);
}


inline void update_and_check_report_ps(int32_t ps_code)
{
    int32_t ps_code_last;

    ps_code_last = pStkPsData->ps_code_last;
	//INFO("STK PS : ps_code=%d\n",ps_code);
	if (ps_code > ps_code_high_thd)
    {
        if (unlikely(ps_code_last < ps_code_high_thd))
        {
            pStkPsData->ps_code_last = ps_code;
            ps_report_event(pStkPsData->ps_input_dev,0);
        }
    }
	else if (ps_code < ps_code_low_thd)
    {
		if (ps_code_last > ps_code_low_thd)
		{
            pStkPsData->ps_code_last = ps_code;
            ps_report_event(pStkPsData->ps_input_dev,1);
		}
    }
}

static int ps_polling_function(void* arg)
{
    uint32_t delay;
    init_completion(&ps_thread_completion);
    while (1)
    {		
		STK_LOCK(1);
		delay = pStkPsData->ps_delay;		
		STK_LOCK(0);
		msleep(delay);	/* wait for IT */
		
        STK_LOCK(1);        
        pStkPsData->ps_reading = get_ps_reading();
		if(pStkPsData->ps_reading < 0)
		{
			ERR("STK PS %s: get_ps_reading failed, ret=%d", __func__, pStkPsData->ps_reading);
			STK_LOCK(0);  
			continue;
		}		
        update_and_check_report_ps(pStkPsData->ps_reading);
        if (pStkPsData->bPSThreadRunning == 0)
            break;
        STK_LOCK(0);        
    };
    STK_LOCK(0);
    complete(&ps_thread_completion);
    return 0;
}

static int32_t enable_ps(uint8_t enable)
{
    int32_t ret;   
#if (defined(STK_MANUAL_GREYCARD_CALI) || defined(STK_AUTO_CT_CALI_SATU) || defined(STK_AUTO_CT_CALI_NO_SATU))	
	int16_t thd_auto[2] = {120, 110};
	
	if(pStkPsData->first_boot == true)
	{		
		pStkPsData->first_boot = false;
		ret = get_ps_cali_thd(thd_auto);		
		if(ret == 0)
		{
			ret = limit_thd_range(thd_auto);
			set_ps_thd_to_driver(thd_auto);	
			INFO("STK PS %s : PS_LT=%d, PS_HT=%d\n", __func__, thd_auto[STK_LOW_THD], thd_auto[STK_HIGH_THD]);	
		}
	}
	if(enable)
	{
		set_ps_cali(0);
	}	
#endif	/*	#if (defined(STK_MANUAL_GREYCARD_CALI) || defined(STK_AUTO_CT_CALI_SATU) || defined(STK_AUTO_CT_CALI_NO_SATU))	*/
	
    if (enable)
    {
        if (pStkPsData->bPSThreadRunning == 0)
        {     
#ifdef STK_PS_CALI				
			if(pStkPsData->first_boot == true)
			{		
				pStkPsData->first_boot = false;
				ret = get_ps_cali(&ps_ct);		
				if (ret == 0)
				{
					thd_l_auto = ps_ct + STK_DIFF_CT_N_THD_L;
					thd_h_auto = ps_ct + STK_DIFF_CT_N_THD_H;	
					INFO("STK PS %s : read PS calibration data from file, ps_ct=%d, thd_l_auto=%d, thd_h_auto=%d\n", 
					__func__, ps_ct, thd_l_auto, thd_h_auto);	
					if(thd_h_auto <= STK_THD_MAX && thd_l_auto <= STK_THD_MAX)
					{
						set_ps_thd_h(thd_h_auto);
						set_ps_thd_l(thd_l_auto);		
					}
					else
						ERR("STK PS %s: threshold is too large!\n", __func__);
				}
			
			}
#endif			
			wake_lock(&ps_poll_wakelock);						
            pStkPsData->ps_code_last = 0;
            pStkPsData->ps_reading = 0;
            pStkPsData->bPSThreadRunning = 1;
            ps_polling_tsk = kthread_run(ps_polling_function,NULL,"ps_polling");
			ps_report_event(pStkPsData->ps_input_dev,1); 
        }
        else
        {
            WARNING("STK PS : thread has running\n");
        }
    }
    else
    {

        if (pStkPsData->bPSThreadRunning)
        {

            pStkPsData->bPSThreadRunning = 0;
            STK_LOCK(0);
            wait_for_completion(&ps_thread_completion);
            STK_LOCK(1);
            ps_polling_tsk = NULL;
			wake_unlock(&ps_poll_wakelock);
        }
    }

    pStkPsData->ps_cmd_reg &= (~STK_PS_CMD_SD_MASK);
    pStkPsData->ps_cmd_reg |= STK_PS_CMD_SD(enable?0:1);
    ret = i2c_smbus_write_byte_data(pStkPsData->client,STK_PS_CMD_REG,pStkPsData->ps_cmd_reg);
	
	if(ret < 0)
	{
		ERR("STK PS: %s: fail, err=0x%x", __func__, ret);	
		return ret;			
	}
	return 0;
}
static int32_t enable_als(uint8_t enable)
{
	int32_t ret;
    if (enable)
    {
        if (pStkPsData->bALSThreadRunning == 0)
        {
            pStkPsData->als_reading = 0;
			pStkPsData->als_lux_last = 0;
            pStkPsData->bALSThreadRunning = 1;
            als_polling_tsk = kthread_run(als_polling_function,NULL,"als_polling");
        }
        else
        {
            WARNING("STK ALS : thread has running\n");
        }
    }
    else
    {

        if (pStkPsData->bALSThreadRunning)
        {
            pStkPsData->bALSThreadRunning = 0;
            STK_LOCK(0);
            wait_for_completion(&als_thread_completion);
            STK_LOCK(1);
            als_polling_tsk = NULL;
        }
    }
    pStkPsData->als_cmd_reg &= (~STK_ALS_CMD_SD_MASK);
    pStkPsData->als_cmd_reg |= STK_ALS_CMD_SD(enable?0:1);
    ret = i2c_smbus_write_byte_data(pStkPsData->client,STK_ALS_CMD_REG,pStkPsData->als_cmd_reg);
	if(ret < 0)
	{
		ERR("STK PS: %s: fail, err=0x%x", __func__, ret);	
		return ret;			
	}	
	return 0;
}


#ifdef CONFIG_STK_SYSFS_DBG
// For Debug
static ssize_t help_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
     return sprintf(buf, "Usage : cat xxxx\nor echo val > xxxx\
     \nWhere xxxx = ps_code : RO (0~255)\nals_code : RO (0~65535)\nlux : RW (0~by your setting)\ndistance : RW(by your setting)\
     \nals_enable : RW (0~1)\nps_enable : RW(0~1)\nals_transmittance : RW (1~10000)\
     \nps_sleep_time : RW (0~3)\nps_led_driving_current : RW(0~1)\nps_integral_time(0~3)\nps_gain_setting : RW(0~3)\n");

}

static ssize_t driver_version_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf,"%s\n",STK_DRIVER_VER);
}

static ssize_t als_code_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t reading;
    STK_LOCK(1);
    reading = pStkPsData->als_reading;
    STK_LOCK(0);
    return sprintf(buf, "%d\n", reading);
}


static ssize_t ps_code_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t reading;
    STK_LOCK(1);
    reading = pStkPsData->ps_reading;
    STK_LOCK(0);
    return sprintf(buf, "%d\n", reading);
}

#endif //CONFIG_STK_SYSFS_DBG

static ssize_t lux_range_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf, "%d\n", alscode2lux((1<<16) -1));//full code

}

static ssize_t dist_res_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf, "1\n"); // means 1 cm in Android
}
static ssize_t lux_res_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf, "1\n"); // means 1 lux in Android
}
static ssize_t distance_range_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf, "%d\n",1);
}

static ssize_t ps_enable_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
	
    int32_t enable, ret;   
    STK_LOCK(1);
	enable = pStkPsData->bPSThreadRunning;    
	STK_LOCK(0);
	
    ret = i2c_smbus_read_byte_data(pStkPsData->client,STK_PS_CMD_REG);
    ret &= STK_PS_CMD_SD_MASK;
	ret = !ret;
	
	if(enable == ret)
		return sprintf(buf, "%d\n", ret);
	else
	{
		ERR("STK PS: %s: driver and sensor mismatch! driver_enable=0x%x, sensor_enable=%x\n", __func__, enable, ret);
		return sprintf(buf, "%d\n", ret);
	}
}

static ssize_t ps_enable_store(struct kobject *kobj,
                               struct kobj_attribute *attr,
                               const char *buf, size_t len)
{
    uint32_t value = simple_strtoul(buf, NULL, 10);
    INFO("STK PS31xx Driver : Enable PS : %d\n",value);
    STK_LOCK(1);
    enable_ps(value);
    STK_LOCK(0);
    return len;
}

static ssize_t als_enable_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t enable, ret;
    STK_LOCK(1);
    enable = pStkPsData->bALSThreadRunning;	
    STK_LOCK(0);
    ret = i2c_smbus_read_byte_data(pStkPsData->client,STK_ALS_CMD_REG);
    ret &= STK_ALS_CMD_SD_MASK;
	ret = !ret;
	
	if(enable == ret)
		return sprintf(buf, "%d\n", ret);
	else
	{
		ERR("STK PS: %s: driver and sensor mismatch! driver_enable=0x%x, sensor_enable=%x\n", __func__, enable, ret);
		return sprintf(buf, "%d\n", ret);		
	}	
	
}



static ssize_t als_enable_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                const char *buf, size_t len)
{
    uint32_t value = simple_strtoul(buf, NULL, 10);
    INFO("STK PS31xx Driver : Enable ALS : %d\n",value);
    STK_LOCK(1);
    enable_als(value);
    STK_LOCK(0);
    return len;
}


static ssize_t lux_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t als_reading;
    STK_LOCK(1);
    als_reading = pStkPsData->als_reading;
    STK_LOCK(0);
    return sprintf(buf, "%d lux\n", alscode2lux(als_reading));
}

static ssize_t lux_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                const char *buf, size_t len)
{
    unsigned long value = simple_strtoul(buf, NULL, 16);
    STK_LOCK(1);
    als_report_event(pStkPsData->als_input_dev,value);
    STK_LOCK(0);
    return len;
}

static ssize_t distance_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t dist=1;
    STK_LOCK(1);
	if (get_ps_reading()>=ps_code_high_thd)
	{
		ps_report_event(pStkPsData->ps_input_dev,0);
		dist=0;
	}
	else
	{
		ps_report_event(pStkPsData->ps_input_dev,1);
		dist=1;
	}
    STK_LOCK(0);

    return sprintf(buf, "%d\n", dist);
}

static ssize_t distance_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                const char *buf, size_t len)
{
    unsigned long value = simple_strtoul(buf, NULL, 16);
    STK_LOCK(1);
    ps_report_event(pStkPsData->ps_input_dev,value);
    STK_LOCK(0);
    return len;
}

#if (defined(STK_MANUAL_GREYCARD_CALI) || defined(STK_AUTO_CT_CALI_SATU))
static ssize_t ps_cali_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
	int cali;	
	int16_t	thd_auto[2] = {120, 110};
	int32_t ret;  
	
	ret = get_ps_cali_thd(thd_auto);		
	if(ret == 0)
	{		
		ret = limit_thd_range(thd_auto);
		if(ret)
			INFO("STK PS %s: threshold is out of range, err=0x%x\n", __func__, ret);
		set_ps_thd_to_driver(thd_auto);					
		INFO("STK PS %s : read PS calibration data from file, thd_auto[STK_LOW_THD]=%d, thd_auto[STK_HIGH_THD]=%d\n", 
		__func__, thd_auto[STK_LOW_THD], thd_auto[STK_HIGH_THD]);			
	
	}
		
	cali = pStkPsData->ps_cali_done?1:0;
	return sprintf(buf, "%d\n",cali);
}

static ssize_t ps_cali_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len)
{
    unsigned long value;
	int ret;	
	if((ret = strict_strtoul(buf, 16, &value)) < 0)
	{
		ERR("STK PS %s:strict_strtoul failed, ret=0x%x\n", __func__, ret);
		return ret;	
	}
	
	if(value == 1)
	{
		STK_LOCK(1);
		set_ps_cali(1);
		STK_LOCK(0);
	}
	return len;
}

#endif	/*	#if (defined(STK_MANUAL_GREYCARD_CALI) || defined(STK_AUTO_CT_CALI_SATU))*/


#ifdef CONFIG_STK_SYSFS_DBG
/* Only for debug */
static struct kobj_attribute help_attribute = (struct kobj_attribute)__ATTR_RO(help);
static struct kobj_attribute driver_version_attribute = (struct kobj_attribute)__ATTR_RO(driver_version);
static struct kobj_attribute als_code_attribute = (struct kobj_attribute)__ATTR_RO(als_code);
static struct kobj_attribute ps_code_attribute = (struct kobj_attribute)__ATTR_RO(ps_code);
#endif //CONFIG_STK_SYSFS_DBG

static struct kobj_attribute lux_range_attribute = (struct kobj_attribute)__ATTR_RO(lux_range);
static struct kobj_attribute lux_attribute = (struct kobj_attribute)__ATTR_RW(lux);
static struct kobj_attribute distance_attribute = (struct kobj_attribute)__ATTR_RW(distance);
static struct kobj_attribute ps_enable_attribute = (struct kobj_attribute)__ATTR_RW(ps_enable);
static struct kobj_attribute als_enable_attribute = (struct kobj_attribute)__ATTR_RW(als_enable);
static struct kobj_attribute ps_dist_res_attribute = (struct kobj_attribute)__ATTR_RO(dist_res);
static struct kobj_attribute lux_res_attribute = (struct kobj_attribute)__ATTR_RO(lux_res);
static struct kobj_attribute ps_distance_range_attribute = (struct kobj_attribute)__ATTR_RO(distance_range);
#if (defined(STK_MANUAL_GREYCARD_CALI) || defined(STK_AUTO_CT_CALI_SATU))
static struct kobj_attribute dev_ps_cali_attribute = (struct kobj_attribute)__ATTR_RW(ps_cali);
#endif	/*	#if (defined(STK_MANUAL_GREYCARD_CALI) || defined(STK_AUTO_CT_CALI_SATU))	*/
#ifdef CONFIG_STK_SYSFS_DBG

static ssize_t als_transmittance_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t transmittance;
    STK_LOCK(1);
    transmittance = als_transmittance;
    STK_LOCK(0);
    return sprintf(buf, "%d\n", transmittance);
}


static ssize_t als_transmittance_store(struct kobject *kobj,
                                       struct kobj_attribute *attr,
                                       const char *buf, size_t len)
{
    unsigned long value = simple_strtoul(buf, NULL, 10);
    STK_LOCK(1);
    als_transmittance = value;
    STK_LOCK(0);
    return len;
}



static struct kobj_attribute als_transmittance_attribute = (struct kobj_attribute)__ATTR_RW(als_transmittance);


#ifdef CONFIG_STK_PS_ENGINEER_TUNING

static ssize_t ps_sleep_time_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t value;
    STK_LOCK(1);
    value = pStkPsData->ps_cmd_reg;
    STK_LOCK(0);
    value&=STK_PS_CMD_SLP_MASK;
    value>>=STK_PS_CMD_SLP_SHIFT;
    return sprintf(buf, "0x%x\n", value);
}


static ssize_t ps_sleep_time_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                const char *buf, size_t len)
{
    unsigned long value = simple_strtoul(buf, NULL, 10);
    STK_LOCK(1);
    set_ps_slp(value);
    STK_LOCK(0);
    return len;
}


static ssize_t ps_led_driving_current_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t value;
    STK_LOCK(1);
    value = pStkPsData->ps_cmd_reg;
    STK_LOCK(0);
    value&=STK_PS_CMD_DR_MASK;
    value>>=STK_PS_CMD_DR_SHIFT;
    return sprintf(buf, "0x%x\n", value);
}


static ssize_t ps_led_driving_current_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                const char *buf, size_t len)
{
    unsigned long value = simple_strtoul(buf, NULL, 10);
    STK_LOCK(1);
    set_ps_led_driving_current(value);
    STK_LOCK(0);
    return len;
}
static ssize_t ps_integral_time_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t value;
    STK_LOCK(1);
    value = pStkPsData->ps_cmd_reg;
    STK_LOCK(0);
    value&=STK_PS_CMD_IT_MASK;
    value>>=STK_PS_CMD_IT_SHIFT;
    return sprintf(buf, "0x%x\n", value);
}


static ssize_t ps_integral_time_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                const char *buf, size_t len)
{
    unsigned long value = simple_strtoul(buf, NULL, 10);
    STK_LOCK(1);
    set_ps_it((uint8_t)value);
    STK_LOCK(0);
    return len;
}
static ssize_t ps_gain_setting_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    int32_t gc_reg;
    STK_LOCK(1);
    gc_reg = pStkPsData->ps_gc_reg;
    STK_LOCK(0);
    return sprintf(buf, "0x%x\n", gc_reg);
}


static ssize_t ps_gain_setting_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                const char *buf, size_t len)
{
    unsigned long value = simple_strtoul(buf, NULL, 10);
    STK_LOCK(1);
    set_ps_gc((uint8_t)value);
    STK_LOCK(0);
    return len;
}


static ssize_t ps_code_thd_l_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf, "%d\n", ps_code_low_thd);
}


static ssize_t ps_code_thd_l_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                const char *buf, size_t len)
{
    unsigned long value = simple_strtoul(buf, NULL, 10);
    STK_LOCK(1);
    set_ps_thd_l(value);
    STK_LOCK(0);
    return len;
}

static ssize_t ps_code_thd_h_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
    return sprintf(buf, "%d\n", ps_code_high_thd);
}


static ssize_t ps_code_thd_h_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                const char *buf, size_t len)
{
    unsigned long value = simple_strtoul(buf, NULL, 10);
    STK_LOCK(1);
    set_ps_thd_h(value);
    STK_LOCK(0);
    return len;
}

static ssize_t all_reg_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
	int32_t ps_reg[13];
	uint8_t cnt;
    STK_LOCK(1);
	for(cnt=0;cnt<12;cnt++)
	{
		ps_reg[cnt] = i2c_smbus_read_byte_data(pStkPsData->client, (cnt+1));
		if(ps_reg[cnt] < 0)
		{
			STK_LOCK(0);
			ERR("all_reg_show:i2c_smbus_read_byte_data fail, ret=%d\n", ps_reg[cnt]);	
			return -EINVAL;
		}
		else
		{
			INFO("reg[%2X]=0x%2X\n", cnt+1, ps_reg[cnt]);
		}
		
	}
	ps_reg[12] = i2c_smbus_read_byte_data(pStkPsData->client, STK_PS_GC_REG);
	if(ps_reg[12] < 0)
	{
		STK_LOCK(0);
		ERR("all_reg_show:i2c_smbus_read_byte_data fail, ret=%d\n", ps_reg[12]);	
		return -EINVAL;
	}
	INFO("reg[0x82]=0x%2X\n", ps_reg[12]);	
    STK_LOCK(0);

	return sprintf(buf, "%2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2X %2x\n", ps_reg[0], ps_reg[1], ps_reg[2], ps_reg[3], 
	ps_reg[4], ps_reg[5], ps_reg[6], ps_reg[7], ps_reg[8], ps_reg[9], ps_reg[10], ps_reg[11], ps_reg[12]);
}

static ssize_t recv_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
	return 0;
}

static ssize_t recv_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len)
{
    unsigned long value;
	int ret;
	int32_t recv_data;	
	if((ret = strict_strtoul(buf, 16, &value)) < 0)
	{
		ERR("STK PS %s:strict_strtoul failed, ret=0x%x\n", __func__, ret);
		return ret;	
	}

	recv_data = i2c_smbus_read_byte_data(pStkPsData->client,value);
	printk("STK PS: reg 0x%x=0x%x\n", (int)value, recv_data);
	return len;
}

static ssize_t send_show(struct kobject * kobj, struct kobj_attribute * attr, char * buf)
{
	return 0;
}

static ssize_t send_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len)
{
	int addr, cmd;
	u8 addr_u8, cmd_u8;
	s32 ret, i;
	char *token[10];
	
	for (i = 0; i < 2; i++)
	token[i] = strsep((char **)&buf, " ");
	if((ret = strict_strtoul(token[0], 16, (unsigned long *)&(addr))) < 0)
	{
		ERR("STK PS %s:strict_strtoul failed, ret=0x%x\n", __func__, ret);
		return ret;	
	}
	if((ret = strict_strtoul(token[1], 16, (unsigned long *)&(cmd))) < 0)
	{
		ERR("STK PS %s:strict_strtoul failed, ret=0x%x\n", __func__, ret);
		return ret;	
	}
	INFO("STK PS: write reg 0x%x=0x%x\n", addr, cmd);		
	/*
		if(2 != sscanf(buf, "%2x %2x", &addr, &cmd))
		{
		ERR("STK PS %s: unknown format\n", __func__);
		return 0;
		}
	 */
	addr_u8 = (u8) addr;
	cmd_u8 = (u8) cmd;
	STK_LOCK(1);
	ret = i2c_smbus_write_byte_data(pStkPsData->client,addr_u8,cmd_u8);
	STK_LOCK(0);
	if (0 != ret)
	{	
		ERR("STK PS %s: i2c_smbus_write_byte_data fail\n", __func__);
		return ret;
	}
		
	return len;	
}

static struct kobj_attribute ps_sleep_time_attribute = (struct kobj_attribute)__ATTR_RW(ps_sleep_time);
static struct kobj_attribute ps_led_driving_current_attribute = (struct kobj_attribute)__ATTR_RW(ps_led_driving_current);
static struct kobj_attribute ps_integral_time_attribute = (struct kobj_attribute)__ATTR_RW(ps_integral_time);
static struct kobj_attribute ps_gain_setting_attribute = (struct kobj_attribute)__ATTR_RW(ps_gain_setting);
static struct kobj_attribute ps_code_thd_l_attribute = (struct kobj_attribute)__ATTR_RW(ps_code_thd_l);
static struct kobj_attribute ps_code_thd_h_attribute = (struct kobj_attribute)__ATTR_RW(ps_code_thd_h);
static struct kobj_attribute all_reg_attribute = (struct kobj_attribute)__ATTR_RO(all_reg);
static struct kobj_attribute dev_recv_attribute = (struct kobj_attribute)__ATTR_RW(recv);
static struct kobj_attribute dev_send_attribute = (struct kobj_attribute)__ATTR_RW(send);
#endif //CONFIG_STK_PS_ENGINEER_TUNING
#endif //CONFIG_STK_SYSFS_DBG

static struct attribute* sensetek_optical_sensors_attrs [] =
{
    &lux_range_attribute.attr,
    &lux_attribute.attr,
    &distance_attribute.attr,
    &ps_enable_attribute.attr,
    &als_enable_attribute.attr,
    &ps_dist_res_attribute.attr,
    &lux_res_attribute.attr,
    &ps_distance_range_attribute.attr,
#if (defined(STK_MANUAL_GREYCARD_CALI) || defined(STK_AUTO_CT_CALI_SATU))
	&dev_ps_cali_attribute.attr,
#endif
    NULL,
};

#ifdef CONFIG_STK_SYSFS_DBG

static struct attribute* sensetek_optical_sensors_dbg_attrs [] =
{
    &help_attribute.attr,
    &driver_version_attribute.attr,
    &lux_range_attribute.attr,
    &distance_attribute.attr,
    &ps_code_attribute.attr,
    &als_code_attribute.attr,
    &lux_attribute.attr,
    &ps_enable_attribute.attr,
    &als_enable_attribute.attr,
    &ps_dist_res_attribute.attr,
    &lux_res_attribute.attr,
    &ps_distance_range_attribute.attr,
    &als_transmittance_attribute.attr,
#ifdef CONFIG_STK_PS_ENGINEER_TUNING
    &ps_sleep_time_attribute.attr,
    &ps_led_driving_current_attribute.attr,
    &ps_integral_time_attribute.attr,
    &ps_gain_setting_attribute.attr,
    &ps_code_thd_l_attribute.attr,
    &ps_code_thd_h_attribute.attr,
    &all_reg_attribute.attr,		
	&dev_recv_attribute.attr,
	&dev_send_attribute.attr,
#endif //CONFIG_STK_PS_ENGINEER_TUNING
    NULL,
};
// those attributes are only for engineer test/debug
static struct attribute_group sensetek_optics_sensors_attrs_group =
{
    .name = "DBG",
    .attrs = sensetek_optical_sensors_dbg_attrs,
};
#endif //CONFIG_STK_SYSFS_DBG

static struct platform_device *stk_oss_dev = NULL; /* Device structure */

static int stk_sysfs_create_files(struct kobject *kobj,struct attribute** attrs)
{
    int err;
    while(*attrs!=NULL)
    {
        err = sysfs_create_file(kobj,*attrs);
        if (err)
            return err;
        attrs++;
    }
    return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void stk31xx_early_suspend(struct early_suspend *h)
{
    int32_t enable;
	INFO("%s", __func__);
    STK_LOCK(1);
    enable = (pStkPsData->als_cmd_reg & STK_ALS_CMD_SD_MASK)?0:1;    		
	if(enable)
	{
		enable_als(0);		
		pStkPsData->als_cmd_reg &= (~STK_ALS_CMD_SD_MASK);	
	}
	STK_LOCK(0);	
	return;
}

static void stk31xx_late_resume(struct early_suspend *h)
{
    int32_t enable;
	INFO("%s", __func__);	
    STK_LOCK(1);
    enable = (pStkPsData->als_cmd_reg & STK_ALS_CMD_SD_MASK)?0:1;    		
	if(enable)
	{
		enable_als(1);		
	}
	STK_LOCK(0);
	return;
}
#endif	//#ifdef CONFIG_HAS_EARLYSUSPEND


static int stk_ps_probe(struct i2c_client *client,
                        const struct i2c_device_id *id)
{
    int err;
    struct stkps31xx_data* ps_data;
	struct stk31xx_platform_data* plat_data;	
    INFO("STK PS -- %s: I2C is probing, driver version=%s\n", __func__, STK_DRIVER_VER);	
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
    {
        printk("STKPS -- No Support for I2C_FUNC_SMBUS_BYTE_DATA\n");
        return -ENODEV;
    }
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_WORD_DATA))
    {
        printk("STKPS -- No Support for I2C_FUNC_SMBUS_WORD_DATA\n");
        return -ENODEV;
    }

    if (id->driver_data == 0)
    {
		err = i2c_smbus_read_byte_data(client,STK_PS_STATUS_REG);
		if (err < 0)
		{
			ERR("STK PS %s: read i2c error, err=0x%x\n", __func__, err);
			return err;
		}	
		if ((err&STK_PS_STATUS_ID_MASK)!=STK_PS31xx_ID)
		{
			ERR("STK PS %s: invalid ID number\n", __func__);
			return -EINVAL;
		}		
        ps_data = kzalloc(sizeof(struct stkps31xx_data),GFP_KERNEL);
        ps_data->client = client;
        i2c_set_clientdata(client,ps_data);
		if(client->dev.platform_data != NULL)		
		{
			plat_data = client->dev.platform_data;	
			als_transmittance = plat_data->transmittance;			
		}
		else
		{
			ERR("STK PS %s: Please set stk31xx platform data!\n", __func__);
			return EINVAL;
		}
        mutex_init(&stkps_io_lock);
        ps_data->als_input_dev = input_allocate_device();
        ps_data->ps_input_dev = input_allocate_device();
        if ((ps_data->als_input_dev==NULL)||(ps_data->ps_input_dev==NULL))
        {
            if (ps_data->als_input_dev==NULL)
                input_free_device(ps_data->ps_input_dev);
            if (ps_data->ps_input_dev==NULL)
                input_free_device(ps_data->als_input_dev);
            ERR("%s: could not allocate input device\n", __func__);
            mutex_destroy(&stkps_io_lock);
            kfree(ps_data);
            return -ENOMEM;
        }		
        ps_data->als_input_dev->name = ALS_NAME;
        ps_data->ps_input_dev->name = PS_NAME;
        set_bit(EV_ABS, ps_data->als_input_dev->evbit);
        set_bit(EV_ABS, ps_data->ps_input_dev->evbit);
        input_set_abs_params(ps_data->als_input_dev, ABS_MISC, 0, alscode2lux((1<<16)-1), 0, 0);
        input_set_abs_params(ps_data->ps_input_dev, ABS_DISTANCE, 0,1, 0, 0);
	    err = input_register_device(ps_data->als_input_dev);
        if (err<0)
        {
            ERR("STK PS : can not register als input device\n");
            mutex_destroy(&stkps_io_lock);
            input_free_device(ps_data->als_input_dev);
            input_free_device(ps_data->ps_input_dev);
            kfree(ps_data);

            return err;
        }
        INFO("STK PS : register als input device OK\n");
        err = input_register_device(ps_data->ps_input_dev);
        if (err<0)
        {
            ERR("STK PS : can not register ps input device\n");
            input_unregister_device(ps_data->als_input_dev);
            mutex_destroy(&stkps_io_lock);
            input_free_device(ps_data->als_input_dev);
            input_free_device(ps_data->ps_input_dev);
            kfree(ps_data);
            return err;

        }
        pStkPsData = ps_data;
        ps_data->ps_delay = PS_ODR_DELAY;
        ps_data->als_delay = ALS_ODR_DELAY;
        wake_lock_init(&proximity_sensor_wakelock,WAKE_LOCK_IDLE,"stk_ps_wakelock");
        wake_lock_init(&ps_poll_wakelock,WAKE_LOCK_SUSPEND,"stkps_poll_wakelock");
        if (!init_all_setting(plat_data))
        {
    		wake_lock_destroy(&proximity_sensor_wakelock);
			wake_lock_destroy(&ps_poll_wakelock);
            input_unregister_device(pStkPsData->als_input_dev);
            input_unregister_device(pStkPsData->ps_input_dev);
            input_free_device(pStkPsData->als_input_dev);
            input_free_device(pStkPsData->ps_input_dev);
            kfree(pStkPsData);
            pStkPsData = NULL;
            return -EINVAL;
        }
#ifdef CONFIG_HAS_EARLYSUSPEND
		ps_data->stk_early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
		ps_data->stk_early_suspend.suspend = stk31xx_early_suspend;
		ps_data->stk_early_suspend.resume = stk31xx_late_resume;
		register_early_suspend(&ps_data->stk_early_suspend);
#endif
        return 0;
    }

    return -EINVAL;
}


static int stk_ps_remove(struct i2c_client *client)
{

    mutex_destroy(&stkps_io_lock);
    wake_lock_destroy(&proximity_sensor_wakelock);
	wake_lock_destroy(&ps_poll_wakelock);
    if (pStkPsData)
    {
        input_unregister_device(pStkPsData->als_input_dev);
        input_unregister_device(pStkPsData->ps_input_dev);
        input_free_device(pStkPsData->als_input_dev);
        input_free_device(pStkPsData->ps_input_dev);
        kfree(pStkPsData);
        pStkPsData = 0;
    }
    return 0;
}

static const struct i2c_device_id stk_ps_id[] =
{
    { "stk_ps", 0},
    {}
};
MODULE_DEVICE_TABLE(i2c, stk_ps_id);

static struct i2c_driver stk_ps_driver =
{
    .driver = {
        .name = STKPS_DRV_NAME,
    },
    .probe = stk_ps_probe,
    .remove = stk_ps_remove,
    .id_table = stk_ps_id,
};


static int __init stk_i2c_ps31xx_init(void)
{

	int ret;
    ret = i2c_add_driver(&stk_ps_driver);
    if (ret)
    {
        i2c_del_driver(&stk_ps_driver);
        return ret;
    }	
    if (pStkPsData == NULL)
    {
        i2c_del_driver(&stk_ps_driver);
        return -EINVAL;
    }

    stk_oss_dev = platform_device_alloc(DEVICE_NAME,-1);
    if (!stk_oss_dev)
    {
       i2c_del_driver(&stk_ps_driver);
       return -ENOMEM;
    }
    if (platform_device_add(stk_oss_dev))
    {
		platform_device_put(stk_oss_dev);
       i2c_del_driver(&stk_ps_driver);
       return -ENOMEM;
    }
    ret = stk_sysfs_create_files(&(stk_oss_dev->dev.kobj),sensetek_optical_sensors_attrs);
    if (ret)
    {
    	platform_device_unregister(stk_oss_dev);	
        i2c_del_driver(&stk_ps_driver);
        return -ENOMEM;
    }
#ifdef CONFIG_STK_SYSFS_DBG
    ret = sysfs_create_group(&(stk_oss_dev->dev.kobj), &sensetek_optics_sensors_attrs_group);
    if (ret)
    {
	    platform_device_unregister(stk_oss_dev);	
        i2c_del_driver(&stk_ps_driver);
        return -ENOMEM;
    }
#endif //CONFIG_STK_SYSFS_DBG
#ifdef SITRONIX_PERMISSION_THREAD
	STKPermissionThread = kthread_run(stk_permission_thread,"Sitronix","Permissionthread");
	if(IS_ERR(STKPermissionThread))
	STKPermissionThread = NULL;
#endif // SITRONIX_PERMISSION_THREAD	
	INFO("STK PS Module initialized.\n");
    return 0;
}

static void __exit stk_i2c_ps31xx_exit(void)
{

    if (stk_oss_dev);
    {
#ifdef CONFIG_STK_SYSFS_DBG
        sysfs_remove_group(&(stk_oss_dev->dev.kobj), &sensetek_optics_sensors_attrs_group);
#endif
    }
    i2c_del_driver(&stk_ps_driver);
    platform_device_unregister(stk_oss_dev);
#ifdef SITRONIX_PERMISSION_THREAD
	if(STKPermissionThread)
		STKPermissionThread = NULL;
#endif // SITRONIX_PERMISSION_THREAD		
}

MODULE_AUTHOR("Patrick Chang <patrick_chang@sitronix.com.tw>");
MODULE_DESCRIPTION("SenseTek Proximity Sensor driver");
MODULE_LICENSE("GPL");
module_init(stk_i2c_ps31xx_init);
module_exit(stk_i2c_ps31xx_exit);
