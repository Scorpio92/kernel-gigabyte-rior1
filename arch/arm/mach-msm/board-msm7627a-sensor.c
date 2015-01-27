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
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <asm/mach-types.h>
#include <linux/i2c.h>
#include "devices-msm7x2xa.h"
#include <linux/platform_device.h> //add by hxh  for rpi1040 gsensor 2012.11.27
#include <linux/board-ragentek-cfg.h>

#ifdef CONFIG_BOSCH_BMA250
#include <linux/input/bma250.h>
#endif

#if (defined(CONFIG_AVAGO_APDS990X) || defined(CONFIG_ROHM_RPR400))
#include <linux/input/apds990x.h>
#endif

#if (defined(CONFIG_STK31XX_INT_MODE) || defined(CONFIG_STK31XX_POLL_MODE))                        
#include "linux/stk31xx.h"                       
#endif                                           

#if defined(CONFIG_STK3X1X_INT_MODE)                    
#include "linux/stk3x1x.h"                       
#endif  

#ifdef CONFIG_MPU_SENSORS_MPU3050
#include <linux/mpu.h>
#endif

#ifdef  CONFIG_INPUT_LTR502
#include <linux/input/ltr502.h>
#endif

#ifdef CONFIG_SENSORS_AK8975
#include <linux/akm8975.h>
#endif
#ifdef CONFIG_SENSORS_MMC328xMA_MAG
#include <linux/mmc328x.h>
#endif

#if defined(CONFIG_INPUT_KXTJ9)
#include <linux/input/kxtj9.h>
#endif

#ifdef CONFIG_INPUT_KIONIX_ACCEL 
#include <linux/input/kionix_accel.h> 
#endif /* CONFIG_INPUT_KIONIX_ACCEL */ 
#if (defined(CONFIG_AVAGO_APDS990X) || defined(CONFIG_ROHM_RPR400))
#ifndef APDS990X_IRQ_GPIO
#define APDS990X_IRQ_GPIO 17
#endif

#ifndef APDS990x_PS_DETECTION_THRESHOLD
#define APDS990x_PS_DETECTION_THRESHOLD         600
#endif

#ifndef APDS990x_PS_HSYTERESIS_THRESHOLD
#define APDS990x_PS_HSYTERESIS_THRESHOLD        500
#endif

#ifndef APDS990x_ALS_THRESHOLD_HSYTERESIS
#define APDS990x_ALS_THRESHOLD_HSYTERESIS       20
#endif


static struct msm_gpio apds990x_cfg_data[] = {
        {0,"apds990x_irq"},
};

static struct apds990x_platform_data apds990x_platformdata = {
        .ps_det_thld    = APDS990x_PS_DETECTION_THRESHOLD,
        .ps_hsyt_thld   = APDS990x_PS_HSYTERESIS_THRESHOLD,
        .als_hsyt_thld  = APDS990x_ALS_THRESHOLD_HSYTERESIS,
};

static struct i2c_board_info i2c_info_apds990x = {
        I2C_BOARD_INFO("apds990x-rpr400", 0x39),
	  .irq            = -1,//MSM_GPIO_TO_INT(gpio_num[GPIO_ALS_INT_INDEX]),
        .platform_data = &apds990x_platformdata,
};

static int apds990x_setup(void)
{
        int retval = 0;
	
	apds990x_cfg_data[0].gpio_cfg = gpio_cfg[GPIO_ALS_INT_INDEX];
        retval = msm_gpios_request_enable(apds990x_cfg_data, sizeof(apds990x_cfg_data)/sizeof(struct msm_gpio));
        if(retval) {
                printk(KERN_ERR "%s: Failed to obtain L/P sensor interrupt. Code: %d.", __func__, retval);
        }

	i2c_info_apds990x.irq = MSM_GPIO_TO_INT(gpio_num[GPIO_ALS_INT_INDEX]);

        i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
                        &i2c_info_apds990x, 1);

        return retval;
}
#endif

#if (defined(CONFIG_STK31XX_INT_MODE) || defined(CONFIG_STK31XX_POLL_MODE))                   
static struct stk31xx_platform_data stk31xx_data ={                                           
	.als_cmd = 0x4A,   //0x49                                                                          
	.ps_cmd = 0x29,     //0x21                                                                         
	.ps_gain = 0x0d,    //0x01                        
	.transmittance = 500,                       
	.ps_high_thd = 125,                         
	.ps_low_thd = 110,                          
	.int_pin = 17,                               
};                                           
#endif                                       
#if defined(CONFIG_STK31XX_POLL_MODE)                                                
static struct i2c_board_info i2c_stksensor[] __initdata = {                          
	{                                                                                   
	I2C_BOARD_INFO("stk_ps", 0x90>>1),                                                  
	//I2C_BOARD_INFO("stk_ps", 0xB0>>1),                                                
	.platform_data = &stk31xx_data,                             
	.irq = -1,                                                  
	},                                                          
};                                                           
#endif                                                       
                                                             
#ifdef CONFIG_STK31XX_INT_MODE                            
static struct i2c_board_info i2c_stksensor[] __initdata = {                          
	{                                                                                   
	I2C_BOARD_INFO("stk_ps", 0x90>>1),                                                  
	//I2C_BOARD_INFO("stk_ps", 0xB0>>1),                                                
	.platform_data = &stk31xx_data,                             
	.irq = -1,                                                  
	},                                                          
};                                        
#endif                                     

#if defined(CONFIG_STK3X1X_INT_MODE)     
static struct stk3x1x_platform_data stk3x1x_data ={                                           
	.state_reg = 0x0, /* disable all */
	.psctrl_reg = 0x71, /* ps_persistance=4, ps_gain=64X, PS_IT=0.391ms */
	.alsctrl_reg = 0x38 /* als_persistance=1, als_gain=64X, ALS_IT=50ms */,
	.ledctrl_reg = 0xFF, /* 100mA IRDR, 64/64 LED duty */
	.wait_reg = 0x07, /* 50 ms */
	.ps_thd_h =1700,
	.ps_thd_l = 1500,
	.int_pin = 17,
	.transmittance = 500,          
};                   

static struct i2c_board_info i2c_stk3x1x[] __initdata = {                          
	{                                                                                   
	I2C_BOARD_INFO("stk3310_ps", 0x47),                                                  
	//I2C_BOARD_INFO("stk_ps", 0xB0>>1),                                                
	.platform_data = &stk3x1x_data,                             
	.irq = -1,                                                  
	},                                                          
};           

#endif

#ifdef CONFIG_MPU_SENSORS_MPU3050

#define GPIO_ACC_INT 28
#define GPIO_GYRO_INT 27

/* gyro x and z axis invert for EVB*/
static struct mpu_platform_data mpu3050_data = {
	.int_config  = 0x10,
	.orientation = { -1, 0, 0,
			0, 1, 0,
			0, 0, -1 },
};

/* accel x and z axis invert for EVB */
static struct ext_slave_platform_data inv_mpu_bma250_data = {
	.bus         = EXT_SLAVE_BUS_SECONDARY,
	.orientation = { -1, 0, 0,
			0, 1, 0,
			0, 0, -1 },
};
/* compass  */
static struct ext_slave_platform_data inv_mpu_mmc328xms_data = {
	.bus         = EXT_SLAVE_BUS_PRIMARY,
	.orientation = { -1, 0, 0,
			0, 1, 0,
			0, 0, 1 },
};

/* gyro x and z axis invert for EVT*/
static struct mpu_platform_data mpu3050_data_qrd5 = {
	.int_config  = 0x10,
	.orientation = { 1, 0, 0,
			0, 1, 0,
			0, 0, 1 },
};

/* accel x and z axis invert for EVT */
static struct ext_slave_platform_data inv_mpu_bma250_data_qrd5 = {
	.bus         = EXT_SLAVE_BUS_SECONDARY,
	.orientation = { 1, 0, 0,
			0, 1, 0,
			0, 0, 1 },
};
/* compass for EVT  */
static struct ext_slave_platform_data inv_mpu_mmc328xms_data_qrd5 = {
	.bus         = EXT_SLAVE_BUS_PRIMARY,
	.orientation = { 1, 0, 0,
			0, 1, 0,
			0, 0, -1 },
};

static struct i2c_board_info __initdata mpu3050_boardinfo[] = {
	{
		I2C_BOARD_INFO("mpu3050", 0x68),
		.irq = MSM_GPIO_TO_INT(GPIO_GYRO_INT),
		.platform_data = &mpu3050_data,
	},
	{
		I2C_BOARD_INFO("bma250", 0x18),
		//.irq = MSM_GPIO_TO_INT(GPIO_ACC_INT),
		.platform_data = &inv_mpu_bma250_data,
	},
	{
		I2C_BOARD_INFO("mmc328xms", 0x30),
		//.irq = (IH_GPIO_BASE + COMPASS_IRQ_GPIO),
		.platform_data = &inv_mpu_mmc328xms_data,
	},
};

static struct i2c_board_info __initdata mpu3050_boardinfo_qrd5[] = {
	{
		I2C_BOARD_INFO("mpu3050", 0x68),
		.irq = MSM_GPIO_TO_INT(GPIO_GYRO_INT),
		.platform_data = &mpu3050_data_qrd5,
	},
	{
		I2C_BOARD_INFO("bma250", 0x18),
		//.irq = MSM_GPIO_TO_INT(GPIO_ACC_INT),
		.platform_data = &inv_mpu_bma250_data_qrd5,
	},
	{
		I2C_BOARD_INFO("mmc328xms", 0x30),
		//.irq = (IH_GPIO_BASE + COMPASS_IRQ_GPIO),
		.platform_data = &inv_mpu_mmc328xms_data_qrd5,
	},
};

static struct msm_gpio mpu3050_gpio_cfg_data[] = {
	{ GPIO_CFG(GPIO_GYRO_INT, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_6MA),
		"mpu3050_gyroint" },
	{ GPIO_CFG(GPIO_ACC_INT, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_6MA),
		"mpu3050_accint" },
};

static int mpu3050_gpio_setup(void) {
	int ret = 0;
	ret = msm_gpios_request_enable(mpu3050_gpio_cfg_data,
				 sizeof(mpu3050_gpio_cfg_data)/sizeof(struct msm_gpio));
	if( ret<0 )
		printk(KERN_ERR "Failed to obtain mpu3050 int GPIO!\n");
	else
		printk("mpu3050 int GPIO request!\n");
	if(machine_is_msm8625_qrd5() || machine_is_msm7x27a_qrd5a() ) {
		if (ARRAY_SIZE(mpu3050_boardinfo_qrd5))
			i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
						mpu3050_boardinfo_qrd5,
						ARRAY_SIZE(mpu3050_boardinfo_qrd5));
	} else {
		if (ARRAY_SIZE(mpu3050_boardinfo))
			i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
						mpu3050_boardinfo,
						ARRAY_SIZE(mpu3050_boardinfo));
	}
	printk("i2c_register_board_info for MPU3050\n");

	return ret;
}
#endif


#ifdef CONFIG_BOSCH_BMA250
static struct bma250_platform_data bma250_platformdata;

static struct i2c_board_info bma250_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO("bma250", 0x18),
		.platform_data = &bma250_platformdata,
		
	},
};
#endif


#ifdef CONFIG_INPUT_KIONIX_ACCEL 
 
struct kionix_accel_platform_data kionix_accel_pdata = { 
  .min_interval   = 5, 
  .poll_interval  = 200,  
  .accel_direction = 8, 
  .accel_irq_use_drdy = 0, 
  .accel_res = KIONIX_ACCEL_RES_12BIT, 
  .accel_g_range = KIONIX_ACCEL_G_2G,}; 

static struct i2c_board_info kionix_accel_i2c_info[] __initdata = {
  	{
		I2C_BOARD_INFO("acc", KIONIX_ACCEL_I2C_ADDR), 
    		.platform_data = &kionix_accel_pdata,  
    		.irq = -1, // Replace with appropriate GPIO setup 
  	}, 
};
#endif 
#if defined(CONFIG_I2C) && defined(CONFIG_INPUT_LTR502)

static struct ltr502_platform_data ltr502_pdata = {
	.int_gpio = -1,
};

static struct i2c_board_info ltr502_light_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO("ltr502", 0x1c),
		.platform_data =  &ltr502_pdata,
	},
};

static struct msm_gpio ltr502_light_gpio_cfg_data[] = {
	{GPIO_CFG(-1, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_6MA), "ltr502_light_int"},
};

static int ltr502_light_gpio_setup(void) {
	int ret = 0;
	ltr502_pdata.int_gpio = 17;
	ltr502_light_gpio_cfg_data[0].gpio_cfg =
                                GPIO_CFG(ltr502_pdata.int_gpio, 0,
                                GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_6MA);
	ret = msm_gpios_request_enable(ltr502_light_gpio_cfg_data, 1);
	if(ret < 0)
		printk(KERN_ERR "%s: Failed to obtain acc int GPIO %d. Code: %d\n",
				__func__, ltr502_pdata.int_gpio, ret);

	return ret;
}
#endif

#ifdef CONFIG_SENSORS_AK8975

static struct msm_gpio akm_gpio_cfg_data[] = {
	{
		GPIO_CFG(-1, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_6MA),
		"akm_int"
	},
};


static int akm_gpio_setup(void) {
	int ret = 0;
	akm_gpio_cfg_data[0].gpio_cfg = gpio_cfg[GPIO_COMPASS_DRDY_INDEX];
	ret = msm_gpios_request_enable(akm_gpio_cfg_data,
				 sizeof(akm_gpio_cfg_data)/sizeof(struct msm_gpio));
	return ret;
}


static struct akm8975_platform_data akm_platform_data_8975 = {
		.gpio_DRDY = -1,
		.layout = 6,
};

static struct i2c_board_info akm8975_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO("akm8975", 0x0e),
		.platform_data =  &akm_platform_data_8975,
		.flags = I2C_CLIENT_WAKE,
		.irq = -1,//MSM_GPIO_TO_INT(GPIO_COMPASS_DRDY_INDEX),
	},
};
#endif

#if defined(CONFIG_INPUT_KXTJ9)
#define KXTJ9_DEVICE_MAP	7
#define KXTJ9_MAP_X		((KXTJ9_DEVICE_MAP-1)%2)
#define KXTJ9_MAP_Y		(KXTJ9_DEVICE_MAP%2)
#define KXTJ9_NEG_X		(((KXTJ9_DEVICE_MAP+1)/2)%2)
#define KXTJ9_NEG_Y		(((KXTJ9_DEVICE_MAP+5)/4)%2)
#define KXTJ9_NEG_Z		((KXTJ9_DEVICE_MAP-1)/4)

struct kxtj9_platform_data kxtj9_pdata = {
	.min_interval 	= 5,
	.poll_interval 	= 200,

	.axis_map_x 	= KXTJ9_MAP_X,
	.axis_map_y 	= KXTJ9_MAP_Y,
	.axis_map_z 	= 2,

	.negate_x 		= KXTJ9_NEG_X,
	.negate_y 		= KXTJ9_NEG_Y,
	.negate_z 		= KXTJ9_NEG_Z,

	.res_12bit 		= RES_12BIT,
	.g_range  		= KXTJ9_G_2G,
};

static struct i2c_board_info accel_kxtj9_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO("kxtik", 0x0F),
		.platform_data = &kxtj9_pdata,
	},
};
#endif // CONFIG_INPUT_KXTJ9


/*MMC328*/
#ifdef CONFIG_SENSORS_MMC328xMA_MAG
static struct i2c_board_info mmc328_i2c_info[] __initdata = {
	{		I2C_BOARD_INFO(MMC328X_I2C_NAME, MMC328X_I2C_ADDR),
	},
};
#endif

#ifdef CONFIG_MXC_MMA8452
static struct i2c_board_info mxc_i2c0_board_info[]__initdata={
       {
             .type="mma8452",
             .addr=0x1C,
       },
};
#endif

#ifdef CONFIG_INPUT_RPI1040  //add by hxh  for rpi1040 gsensor 2012.11.27 start
#define GPIO_ACCL_INT 28
#define GPIO_SENSOR_EINT 109

static struct msm_gpio rpi1040_gpio_cfg_data[] = {
	{
		GPIO_CFG(GPIO_ACCL_INT, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_4MA),
		"rpi1040-e2"
	},{
		GPIO_CFG(GPIO_SENSOR_EINT, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_4MA),
		"rpi1040-e1"
	},{
		GPIO_CFG(27, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_4MA),
		"rpi1040-enable"
	},
};
static struct platform_device msm_device_rpi1040_sensor = {
	.name	= "rpi1040",
	.id	= 0
};
static int rpi1040_setup(void) {
	int ret = 0;
	printk("###wm rpi1040_setup\n");
	ret = msm_gpios_request_enable(rpi1040_gpio_cfg_data,
				 sizeof(rpi1040_gpio_cfg_data)/sizeof(struct msm_gpio));
	if( ret<0 )
		printk(KERN_ERR "### %s: Failed to obtain rpi1040 sensor GPIO . Code: %d\n",
				__func__, ret);
	platform_device_register(&msm_device_rpi1040_sensor);
	return ret;
}


#endif//add by hxh  for rpi1040 gsensor 2012.11.27 end
void __init msm7627a_sensor_init(void)
{
	//int rc = 0;
#if (defined(CONFIG_AVAGO_APDS990X) || defined(CONFIG_ROHM_RPR400))
	if (machine_is_msm8625_skua() || machine_is_msm7627a_evb() || machine_is_msm8625_evb() || machine_is_msm8625_qrd5() || machine_is_msm7x27a_qrd5a() ) {
		apds990x_setup();
	}
#endif
#if (defined(CONFIG_STK31XX_INT_MODE) || defined(CONFIG_STK31XX_POLL_MODE))        
	printk("i2c_register_board_info stk31xx\n");                                      
	i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,                                 
				i2c_stksensor,                          
				ARRAY_SIZE(i2c_stksensor));             
#endif  
                                       
#if defined(CONFIG_STK3X1X_INT_MODE)     
printk("i2c_register_board_info stk3x1x\n");                                      
	i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,                                 
				i2c_stk3x1x,                          
				ARRAY_SIZE(i2c_stk3x1x));         
#endif

#ifdef CONFIG_MXC_MMA8452
       pr_info("i2c_register_board_info_mma8452\n");
		 rc = gpio_tlmm_config(gpio_cfg[GPIO_ACC_INT_INDEX], GPIO_CFG_ENABLE);
		 if (rc < 0) {
			printk("sym sensor gpio_tlmm_config(gpio_cfg[GPIO_ACC_INT_INDEX] error,");
		
	}
               i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
                       mxc_i2c0_board_info,
                       ARRAY_SIZE(mxc_i2c0_board_info));
#endif
#ifdef CONFIG_MPU_SENSORS_MPU3050
	if (machine_is_msm7627a_evb() || machine_is_msm8625_evb() || machine_is_msm8625_qrd5() || machine_is_msm7x27a_qrd5a()) {
		mpu3050_gpio_setup();
	}
#endif

#ifdef CONFIG_BOSCH_BMA250
	if (machine_is_msm8625_qrd5()) {
		pr_info("i2c_register_board_info BMA250 ACC\n");
		if(is_rgtk_product(RGTK_PRODUCT_Q803) || is_rgtk_product(RGTK_PRODUCT_Q203)) 
			bma250_platformdata.project_id = 803;
		else if(is_rgtk_product(RGTK_PRODUCT_Q802))
			bma250_platformdata.project_id = 802;
		else if(is_rgtk_product(RGTK_PRODUCT_Q801))
			bma250_platformdata.project_id = 801;
		else		
			bma250_platformdata.project_id = 802;   //default as 202

		i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
					bma250_i2c_info,
					ARRAY_SIZE(bma250_i2c_info));
	}
#endif
#ifdef CONFIG_INPUT_RPI1040
	rpi1040_setup();////add by hxh  for rpi1040 gsensor 2012.11.27
#endif


#ifdef CONFIG_INPUT_KIONIX_ACCEL 
	kionix_accel_i2c_info[0].irq = MSM_GPIO_TO_INT(gpio_num[GPIO_ACC_INT_INDEX]);	
 // gpio_free(MSM_GPIO_TO_INT(gpio_num[GPIO_ACC_INT_INDEX]));
 // gpio_request(MSM_GPIO_TO_INT(gpio_num[GPIO_ACC_INT_INDEX]), "kionix-irq"); // Replace with appropriate GPIO setup 
 // gpio_direction_input(MSM_GPIO_TO_INT(gpio_num[GPIO_ACC_INT_INDEX])); // Replace with appropriate GPIO setup 
 		pr_info("i2c_register_board_info kionix_accel_\n");
		i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
					kionix_accel_i2c_info,
					ARRAY_SIZE(kionix_accel_i2c_info));
	
#endif 
  


#ifdef CONFIG_INPUT_LTR502
	if (machine_is_msm8625_qrd7()) {
		pr_info("i2c_register_board_info LTR502\n");
		ltr502_light_gpio_setup();
		i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
				ltr502_light_i2c_info,
				ARRAY_SIZE(ltr502_light_i2c_info));
	}
#endif

#ifdef CONFIG_SENSORS_AK8975
	if (machine_is_msm8625_qrd5()) {
		pr_info("i2c_register_board_info AKM8975\n");
		if(is_rgtk_product(RGTK_PRODUCT_Q803) || is_rgtk_product(RGTK_PRODUCT_Q802) ||is_rgtk_product(RGTK_PRODUCT_Q203) ) {
			akm_platform_data_8975.layout = 6;
		}else if(is_rgtk_product(RGTK_PRODUCT_Q801) ){
			akm_platform_data_8975.layout = 5;
		}else{
			printk("sym project ID error\n");
			akm_platform_data_8975.layout = 6;
		}	
			akm_platform_data_8975.gpio_DRDY = gpio_num[GPIO_COMPASS_DRDY_INDEX];
		akm_gpio_setup();
		akm8975_i2c_info[0].irq = gpio_to_irq(gpio_num[GPIO_COMPASS_DRDY_INDEX]);
		i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
				akm8975_i2c_info,
				ARRAY_SIZE(akm8975_i2c_info));
	}
#endif

#ifdef CONFIG_INPUT_KXTJ9
	if(machine_is_msm8625_skua()) {
	pr_info("i2c_register_board_info KXTJ9\n");
		i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
			accel_kxtj9_i2c_info,
			ARRAY_SIZE(accel_kxtj9_i2c_info));
	}
#endif

/*MMC328*/
#ifdef CONFIG_SENSORS_MMC328xMA_MAG
if(machine_is_msm8625_qrd5())
{
i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
				mmc328_i2c_info,
				ARRAY_SIZE(mmc328_i2c_info));
}
#endif


}
