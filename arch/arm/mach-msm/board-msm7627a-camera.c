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

#include <linux/i2c.h>
#include <linux/i2c/sx150x.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <asm/mach-types.h>
#include <mach/msm_iomap.h>
#include <mach/board.h>
#include <mach/irqs-7xxx.h>
#include "devices-msm7x2xa.h"
#include "board-msm7627a.h"
#include <mach/vreg.h>
#include <linux/board-ragentek-cfg.h>
#include <../../../../build/buildplus/target/QRDExt_target.h>

#define GPIO_SKU1_CAM_VGA_SHDN    18
#define GPIO_SKU1_CAM_VGA_RESET_N 29
#define GPIO_SKU3_CAM_5MP_SHDN_N   122        /* PWDN */
#define GPIO_SKU3_CAM_5MP_CAMIF_RESET  123   /* (board_is(EVT))?123:121 RESET */
#define GPIO_SKU3_CAM_5MP_CAM_DRIVER_PWDN 30
#define GPIO_NOT_CONFIGURED -1
//for qrd7 camera gpio
static int camera_gpio_rear_qrd7[] = {93, 23, 30};//PWDN, RESET, DRIVER_PWDN
static int camera_gpio_front_qrd7[] = {91, 29};//PWDN, RESET

//for skua camra gpio
#define QRD_SKUA_GPIO_CAM_5MP_SHDN_EN 8
#define QRD_SKUA_GPIO_CAM_5MP_RESET 9
#define QRD_SKUA_GPIO_CAM_3MP_PWDN 117
#define QRD_SKUA_GPIO_CAM_3MP_RESET 118 //SKUA has not this connection
#define QRD_SKUA_GPIO_CAM_FASH_LED_PWN  49
#define QRD_SKUA_GPIO_CAM_FASH_LED_PWM  94

#ifdef CONFIG_MSM_CAMERA_V4L2
static uint32_t camera_off_gpio_table[] = {
	GPIO_CFG(15, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
};

static uint32_t camera_on_gpio_table[] = {
	GPIO_CFG(15, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
};

#ifdef CONFIG_S5K4E1
static struct gpio s5k4e1_cam_req_gpio[] = {
	{GPIO_CAM_GP_CAMIF_RESET_N, GPIOF_DIR_OUT, "CAM_RESET"},
};

static struct msm_gpio_set_tbl s5k4e1_cam_gpio_set_tbl[] = {
	{GPIO_CAM_GP_CAMIF_RESET_N, GPIOF_OUT_INIT_LOW, 1000},
	{GPIO_CAM_GP_CAMIF_RESET_N, GPIOF_OUT_INIT_HIGH, 4000},
};

static struct msm_camera_gpio_conf gpio_conf_s5k4e1 = {
	.camera_off_table = camera_off_gpio_table,
	.camera_off_table_size = ARRAY_SIZE(camera_off_gpio_table),
	.camera_on_table = camera_on_gpio_table,
	.camera_on_table_size = ARRAY_SIZE(camera_on_gpio_table),
	.cam_gpio_req_tbl = s5k4e1_cam_req_gpio,
	.cam_gpio_req_tbl_size = ARRAY_SIZE(s5k4e1_cam_req_gpio),
	.cam_gpio_set_tbl = s5k4e1_cam_gpio_set_tbl,
	.cam_gpio_set_tbl_size = ARRAY_SIZE(s5k4e1_cam_gpio_set_tbl),
	.gpio_no_mux = 1,
};
#endif

#ifdef CONFIG_MT9E013
static struct msm_camera_gpio_conf gpio_conf_mt9e013 = {
	.camera_off_table = camera_off_gpio_table,
	.camera_on_table = camera_on_gpio_table,
	.gpio_no_mux = 1,
};
#endif

#ifdef CONFIG_WEBCAM_OV9726
static struct msm_camera_gpio_conf gpio_conf_ov9726 = {
	.camera_off_table = camera_off_gpio_table,
	.camera_on_table = camera_on_gpio_table,
	.gpio_no_mux = 1,
};
#endif

#ifdef CONFIG_WEBCAM_OV7692_QRD
static struct msm_camera_gpio_conf gpio_conf_ov7692 = {
	.camera_off_table = camera_off_gpio_table,
	.camera_on_table = camera_on_gpio_table,
	.gpio_no_mux = 1,
};
#endif
/*renwei add it for the front camera at 2012-6-12*/
#ifdef CONFIG_GC0339
static struct msm_camera_gpio_conf gpio_conf_gc0339 = {
	.camera_off_table = camera_off_gpio_table,
	.camera_on_table = camera_on_gpio_table,
	.gpio_no_mux = 1,
};
#endif

#ifdef CONFIG_MT9V113
static struct msm_camera_gpio_conf gpio_conf_mt9v113 = {
	.camera_off_table = camera_off_gpio_table,
	.camera_on_table = camera_on_gpio_table,
	.gpio_no_mux = 1,
};
#endif

#ifdef CONFIG_OV5640
static struct msm_camera_gpio_conf gpio_conf_ov5640 = {
	.camera_off_table = camera_off_gpio_table,
	.camera_on_table = camera_on_gpio_table,
	.gpio_no_mux = 1,
};
#endif

/*renwei add it for s5k5ca at 2012-9-26*/
#ifdef CONFIG_S5K5CA
static struct msm_camera_gpio_conf gpio_conf_s5k5ca = {
	.camera_off_table = camera_off_gpio_table,
	.camera_on_table = camera_on_gpio_table,
	.gpio_no_mux = 1,
};
#endif
/*add end*/
#ifdef CONFIG_OV5647
static struct msm_camera_gpio_conf gpio_conf_ov5647 = {
	.camera_off_table = camera_off_gpio_table,
	.camera_on_table = camera_on_gpio_table,
	.gpio_no_mux = 1,
};
#endif
#ifdef CONFIG_OV5647_TRULY_CM6868
static struct msm_camera_gpio_conf gpio_conf_ov5647_truly_cm6868 = {
	.camera_off_table = camera_off_gpio_table,
	.camera_on_table = camera_on_gpio_table,
	.gpio_no_mux = 1,
};
#endif
#ifdef CONFIG_OV5647_SUNNY_P5V02S
static struct msm_camera_gpio_conf gpio_conf_ov5647_sunny_p5v02s = {
	.camera_off_table = camera_off_gpio_table,
	.camera_on_table = camera_on_gpio_table,
	.gpio_no_mux = 1,
};
#endif
/*lilonghui add it for the 
q203 camera 2012-9-17 */
#ifdef CONFIG_Q203
#ifdef CONFIG_OV2655
static struct msm_camera_gpio_conf gpio_conf_ov2655 = {
	.camera_off_table = camera_off_gpio_table,
	.camera_on_table = camera_on_gpio_table,
	.gpio_no_mux = 1,
};
#endif
/*add end*/
#else
/*renwei add it for ov2655 camera at 2012-8-1*/
#ifdef CONFIG_OV2655
static struct msm_camera_gpio_conf gpio_conf_ov2655 = {
	.camera_off_table = camera_off_gpio_table,
	.camera_on_table = camera_on_gpio_table,
	.gpio_no_mux = 1,
};
#endif
/*add end*/
#endif

#ifdef CONFIG_WEBCAM_OV9726
static struct msm_camera_gpio_conf gpio_conf_ov9726 = {
	.camera_off_table = camera_off_gpio_table,
	.camera_on_table = camera_on_gpio_table,
	.gpio_no_mux = 1,
};
#endif

#ifdef CONFIG_AR0543
static struct msm_camera_gpio_conf gpio_conf_ar0543 = {
	.camera_off_table = camera_off_gpio_table,
	.camera_on_table = camera_on_gpio_table,
	.gpio_no_mux = 1,
};
#endif
#if 0
#ifdef CONFIG_MSM_CAMERA_FLASH
static struct msm_camera_sensor_flash_src msm_flash_src = {
	.flash_sr_type = MSM_CAMERA_FLASH_SRC_EXT,
	._fsrc.ext_driver_src.led_en = GPIO_CAM_GP_LED_EN1,
	._fsrc.ext_driver_src.led_flash_en = GPIO_CAM_GP_LED_EN2,
};
#endif
#endif

static struct camera_vreg_t msm_cam_vreg[] = {
	{"msme1", REG_LDO, 1800000, 1800000, 0},
	{"gp2", REG_LDO, 2850000, 2850000, 0},
	{"usb2", REG_LDO, 1800000, 1800000, 0},
};

//static struct msm_camera_sensor_info msm_camera_sensor_s5k4e1_data;

struct msm_camera_device_platform_data msm_camera_device_data_csi1[] = {
	{
		.csid_core = 1,
		.is_csic = 1,
		.ioclk = {
			.vfe_clk_rate = 192000000,
		},
	},
	{
		.csid_core = 1,
		.is_csic = 1,
		.ioclk = {
			.vfe_clk_rate = 266667000,
		},
	},
};

struct msm_camera_device_platform_data msm_camera_device_data_csi0[] = {
	{
		.csid_core = 0,
		.is_csic = 1,
		.ioclk = {
			.vfe_clk_rate = 192000000,
		},
	},
	{
		.csid_core = 0,
		.is_csic = 1,
		.ioclk = {
			.vfe_clk_rate = 266667000,
		},
	},
};

static struct i2c_board_info msm_act_main_cam_i2c_info = {
	I2C_BOARD_INFO("msm_actuator", 0x18 >> 1),
};

#ifdef CONFIG_DW9712_ACT
static struct i2c_board_info s5k4e1_actuator_i2c_info = {
	I2C_BOARD_INFO("dw9712_act", 0x8C >> 1),
};

static struct msm_actuator_info s5k4e1_actuator_info = {
	.board_info     = &s5k4e1_actuator_i2c_info,
	.bus_id         = MSM_GSBI0_QUP_I2C_BUS_ID,
	.vcm_pwd        = GPIO_CAM_GP_CAM_PWDN,
	.vcm_enable     = 1,
};
#endif

#ifdef CONFIG_S5K4E1
static struct msm_actuator_info msm_act_main_cam_4_info = {
	.board_info     = &msm_act_main_cam_i2c_info,
	.cam_name   = MSM_ACTUATOR_MAIN_CAM_4,
	.bus_id         = MSM_GSBI0_QUP_I2C_BUS_ID,
	.vcm_pwd        = GPIO_CAM_GP_CAM_PWDN,
	.vcm_enable     = 1,
};

static struct msm_camera_sensor_flash_data flash_s5k4e1 = {
	.flash_type             = MSM_CAMERA_FLASH_LED,
	.flash_src              = &msm_flash_src
	};

static struct msm_camera_sensor_platform_info sensor_board_info_s5k4e1 = {
	.mount_angle	= 90,
	.cam_vreg = msm_cam_vreg,
	.num_vreg = ARRAY_SIZE(msm_cam_vreg),
	.gpio_conf = &gpio_conf_s5k4e1,
};

static struct msm_camera_sensor_info msm_camera_sensor_s5k4e1_data = {
	.sensor_name    = "s5k4e1",
	.sensor_reset_enable = 1,
	.pmic_gpio_enable    = 0,
	.pdata                  = &msm_camera_device_data_csi1[0],
	.flash_data             = &flash_s5k4e1,
	.sensor_platform_info   = &sensor_board_info_s5k4e1,
	.csi_if                 = 1,
	.camera_type = BACK_CAMERA_2D,
	.sensor_type = BAYER_SENSOR,
	.actuator_info = &msm_act_main_cam_4_info,
};
#endif

#ifdef CONFIG_WEBCAM_OV7692_QRD
static struct msm_camera_sensor_platform_info sensor_board_info_ov7692 = {
	.mount_angle = 90,//renwei modify it for the front camera at 2012-5-31
	.cam_vreg = msm_cam_vreg,
	.num_vreg = ARRAY_SIZE(msm_cam_vreg),
	.gpio_conf = &gpio_conf_ov7692,
};

static struct msm_camera_sensor_flash_data flash_ov7692 = {
	.flash_type     = MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_sensor_info msm_camera_sensor_ov7692_data = {
	.sensor_name	    = "ov7692",
	.sensor_reset_enable    = 0,
	.pmic_gpio_enable  = 1,
	.sensor_reset	   = GPIO_SKU1_CAM_VGA_RESET_N,
	.sensor_pwd	     = GPIO_SKU1_CAM_VGA_SHDN,
	.pdata			= &msm_camera_device_data_csi0[0],
	.flash_data	     = &flash_ov7692,
	.sensor_platform_info   = &sensor_board_info_ov7692,
	.csi_if		 = 1,
	.camera_type = FRONT_CAMERA_2D,
	.sensor_type = YUV_SENSOR,
};
#endif
/*renwei add it for the front camera at 2012-6-12*/
#ifdef CONFIG_GC0339

static struct msm_camera_sensor_platform_info sensor_board_info_gc0339 = {
	//.mount_angle = 270,
	.cam_vreg = msm_cam_vreg,
	.num_vreg = ARRAY_SIZE(msm_cam_vreg),
	.gpio_conf = &gpio_conf_gc0339,
};

static struct msm_camera_sensor_flash_data flash_gc0339 = {
	.flash_type     = MSM_CAMERA_FLASH_NONE,
};
#ifdef CONFIG_DC205_YL
static struct msm_camera_sensor_info msm_camera_sensor_gc0339_data = {
	.sensor_name	    = "gc0339",
	.sensor_reset_enable    = 0,
	.pmic_gpio_enable  = 1,
	.sensor_reset	   = GPIO_NOT_CONFIGURED,
	.sensor_pwd	     = GPIO_NOT_CONFIGURED,
	.pdata			= &msm_camera_device_data_csi1[1],
	.flash_data	     = &flash_gc0339,
	.sensor_platform_info   = &sensor_board_info_gc0339,
	.csi_if		 = 1,
	.camera_type = BACK_CAMERA_2D,
	.sensor_type = BAYER_SENSOR,
};
#else
static struct msm_camera_sensor_info msm_camera_sensor_gc0339_data = {
	.sensor_name	    = "gc0339",
	.sensor_reset_enable    = 0,
	.pmic_gpio_enable  = 1,
	.sensor_reset	   = GPIO_NOT_CONFIGURED,
	.sensor_pwd	     = GPIO_NOT_CONFIGURED,
	.pdata			= &msm_camera_device_data_csi0[0],
	.flash_data	     = &flash_gc0339,
	.sensor_platform_info   = &sensor_board_info_gc0339,
	.csi_if		 = 1,
	.camera_type = FRONT_CAMERA_2D,
	.sensor_type = BAYER_SENSOR,
};
#endif
#endif


#ifdef CONFIG_MT9V113
static struct msm_camera_sensor_platform_info sensor_board_info_mt9v113 = {
	.mount_angle = 90,
	.cam_vreg = msm_cam_vreg,
	.num_vreg = ARRAY_SIZE(msm_cam_vreg),
	.gpio_conf = &gpio_conf_mt9v113,
};

static struct msm_camera_sensor_flash_data flash_mt9v113 = {
	.flash_type     = MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9v113_data = {
	.sensor_name	    = "mt9v113",
	.sensor_reset_enable    = 0,
	.pmic_gpio_enable  = 1,
	.sensor_reset	   = GPIO_SKU1_CAM_VGA_RESET_N,
	.sensor_pwd	     = GPIO_SKU1_CAM_VGA_SHDN,
	.pdata			= &msm_camera_device_data_csi0[0],
	.flash_data	     = &flash_mt9v113,
	.sensor_platform_info   = &sensor_board_info_mt9v113,
	.csi_if		 = 1,
	.camera_type = FRONT_CAMERA_2D,
	.sensor_type = YUV_SENSOR,
};
#endif

#ifdef CONFIG_OV5640
static struct msm_camera_sensor_platform_info sensor_board_info_ov5640 = {
	.mount_angle = 90,
	.cam_vreg = msm_cam_vreg,
	.num_vreg = ARRAY_SIZE(msm_cam_vreg),
	.gpio_conf = &gpio_conf_ov5640,
};

static struct msm_camera_sensor_flash_src msm_flash_src_ov5640 = {
	.flash_sr_type = MSM_CAMERA_FLASH_SRC_LED,
	._fsrc.led_src.led_name = "flashlight",
	._fsrc.led_src.led_name_len = 10,
};

static struct msm_camera_sensor_flash_data flash_ov5640 = {
	.flash_type             = MSM_CAMERA_FLASH_LED,
	.flash_src              = &msm_flash_src_ov5640,
};

static struct msm_camera_sensor_info msm_camera_sensor_ov5640_data = {
	.sensor_name    = "ov5640",
	.sensor_reset_enable = 1,
	.pmic_gpio_enable  = 1,
	.sensor_reset   = GPIO_SKU3_CAM_5MP_CAMIF_RESET,
	.sensor_pwd     _= GPIO_SKU3_CAM_5MP_SHDN_N,
	.pdata          = &msm_camera_device_data_csi1[0],
	.flash_data     = &flash_ov5640,
	.sensor_platform_info   = &sensor_board_info_ov5640,
	.csi_if                 = 1,
	.camera_type	= BACK_CAMERA_2D,
	.sensor_type = YUV_SENSOR,
};
#endif


#ifdef CONFIG_OV5647

static struct msm_actuator_info msm_act_main_cam_6_info = {
	.board_info     = &msm_act_main_cam_i2c_info,
	.cam_name   = MSM_ACTUATOR_MAIN_CAM_6,
	.bus_id         = MSM_GSBI0_QUP_I2C_BUS_ID,
	.vcm_pwd        = GPIO_NOT_CONFIGURED,
	.vcm_enable     = 1,
};


#ifdef CONFIG_AD5046_ACT
static struct i2c_board_info ad5046_actuator_i2c_info = {
	I2C_BOARD_INFO("ad5046_act", 0x18 >> 1),
};

static struct msm_actuator_info ad5046_actuator_info = {
	.board_info     = &ad5046_actuator_i2c_info,
	.bus_id         = MSM_GSBI0_QUP_I2C_BUS_ID,
	.vcm_pwd        = GPIO_SKU3_CAM_5MP_CAM_DRIVER_PWDN,
	.vcm_enable     = 1,
};
#endif

static struct msm_camera_sensor_platform_info sensor_board_info_ov5647 = {
	//.mount_angle = 270,//renwei modify it for the back camera at 2012-5-13
	.cam_vreg = msm_cam_vreg,
	.num_vreg = ARRAY_SIZE(msm_cam_vreg),
	.gpio_conf = &gpio_conf_ov5647,
};
/*lilonghui modify it for the camera flashlight 2012-6-29  if you use the camera flash light for Q801,you can open the CONFIG*/
#ifdef CAMERA_FLASH_LED
static struct msm_camera_sensor_flash_src msm_flash_src_ov5647 = {
	.flash_sr_type = MSM_CAMERA_FLASH_SRC_LED1,
	._fsrc.ext_driver_src.led_en = QRD_SKUA_GPIO_CAM_FASH_LED_PWM,
	._fsrc.ext_driver_src.led_flash_en = QRD_SKUA_GPIO_CAM_FASH_LED_PWN,
};

static struct msm_camera_sensor_flash_data flash_ov5647 = {
	.flash_type             = MSM_CAMERA_FLASH_LED,
	.flash_src              = &msm_flash_src_ov5647,
};
#else
static struct msm_camera_sensor_flash_data flash_ov5647 = {
	.flash_type     = MSM_CAMERA_FLASH_NONE,
};
#endif
/*end*/
static struct msm_camera_sensor_info msm_camera_sensor_ov5647_data = {
	.sensor_name    = "ov5647",
	.sensor_reset_enable = 1,
	.pmic_gpio_enable  = 1,
	.sensor_reset   = GPIO_SKU3_CAM_5MP_CAMIF_RESET,
	.sensor_pwd     = GPIO_SKU3_CAM_5MP_SHDN_N,
	.pdata          = &msm_camera_device_data_csi1[0],
	.flash_data     = &flash_ov5647,
	.sensor_platform_info   = &sensor_board_info_ov5647,
	.csi_if                 = 1,
	.camera_type	= BACK_CAMERA_2D,
	.sensor_type = BAYER_SENSOR,
	#ifdef CONFIG_AD5046_ACT
	.actuator_info = &ad5046_actuator_info
	#else
	.actuator_info = &msm_act_main_cam_6_info,
	#endif
};
#endif

#ifdef CONFIG_OV5647_TRULY_CM6868
static struct msm_actuator_info msm_act_main_cam_6_info = {
	.board_info     = &msm_act_main_cam_i2c_info,
	.cam_name   = MSM_ACTUATOR_MAIN_CAM_6,
	.bus_id         = MSM_GSBI0_QUP_I2C_BUS_ID,
	.vcm_pwd        = GPIO_NOT_CONFIGURED,
	.vcm_enable     = 0,
};

static struct msm_camera_sensor_platform_info sensor_board_info_ov5647_truly_cm6868 = {
	.mount_angle = 90,
	.cam_vreg = msm_cam_vreg,
	.num_vreg = ARRAY_SIZE(msm_cam_vreg),
	.gpio_conf = &gpio_conf_ov5647_truly_cm6868,
};

static struct msm_camera_sensor_flash_src msm_flash_src_ov5647_truly_cm6868 = {
	.flash_sr_type = MSM_CAMERA_FLASH_SRC_LED1,
	._fsrc.ext_driver_src.led_en = 13,
	._fsrc.ext_driver_src.led_flash_en = 32,
};

static struct msm_camera_sensor_flash_data flash_ov5647_truly_cm6868 = {
	.flash_type             = MSM_CAMERA_FLASH_LED,
	.flash_src              = &msm_flash_src_ov5647_truly_cm6868,
};

static struct msm_camera_sensor_info msm_camera_sensor_ov5647_truly_cm6868_data = {
	.sensor_name    = "ov5647_truly_cm6868",
	.sensor_reset_enable = 1,
	.pmic_gpio_enable  = 1,
	.sensor_reset   = GPIO_NOT_CONFIGURED,
	.sensor_pwd     = GPIO_NOT_CONFIGURED,
	.pdata          = &msm_camera_device_data_csi1[0],
	.flash_data     = &flash_ov5647_truly_cm6868,
	.sensor_platform_info   = &sensor_board_info_ov5647_truly_cm6868,
	.csi_if                 = 1,
	.camera_type	= BACK_CAMERA_2D,
	.sensor_type = BAYER_SENSOR,
	.actuator_info = &msm_act_main_cam_6_info,
};

#endif
#ifdef CONFIG_OV5647_SUNNY_P5V02S
static struct msm_actuator_info msm_act_main_cam_7_info = {
	.board_info     = &msm_act_main_cam_i2c_info,
	.cam_name   = MSM_ACTUATOR_MAIN_CAM_7,
	.bus_id         = MSM_GSBI0_QUP_I2C_BUS_ID,
	.vcm_pwd        = GPIO_NOT_CONFIGURED,
	.vcm_enable     = 0,
};

static struct msm_camera_sensor_platform_info sensor_board_info_ov5647_sunny_p5v02s = {
	.mount_angle = 90,
	.cam_vreg = msm_cam_vreg,
	.num_vreg = ARRAY_SIZE(msm_cam_vreg),
	.gpio_conf = &gpio_conf_ov5647_sunny_p5v02s,
};

static struct msm_camera_sensor_flash_src msm_flash_src_ov5647_sunny_p5v02s = {
	.flash_sr_type = MSM_CAMERA_FLASH_SRC_LED1,
	._fsrc.ext_driver_src.led_en = 13,
	._fsrc.ext_driver_src.led_flash_en = 32,
};

static struct msm_camera_sensor_flash_data flash_ov5647_sunny_p5v02s = {
	.flash_type             = MSM_CAMERA_FLASH_NONE,
	.flash_src              = &msm_flash_src_ov5647_sunny_p5v02s,
};

static struct msm_camera_sensor_info msm_camera_sensor_ov5647_sunny_p5v02s_data = {
	.sensor_name    = "ov5647_sunny_p5v02s",
	.sensor_reset_enable = 1,
	.pmic_gpio_enable  = 1,
	.sensor_reset   = GPIO_NOT_CONFIGURED,
	.sensor_pwd     = GPIO_NOT_CONFIGURED,
	.pdata          = &msm_camera_device_data_csi1[0],
	.flash_data     = &flash_ov5647_sunny_p5v02s,
	.sensor_platform_info   = &sensor_board_info_ov5647_sunny_p5v02s,
	.csi_if                 = 1,
	.camera_type	= BACK_CAMERA_2D,
	.sensor_type = BAYER_SENSOR,
	.actuator_info = &msm_act_main_cam_7_info,
};

#endif
/*lilonghui add it for the  q203 camera  2012-9-17*/
#ifdef CONFIG_Q203
#ifdef CONFIG_OV2655
#ifdef CAMERA_FLASH_LED
static struct msm_camera_sensor_platform_info sensor_board_info_ov2655 = {
	//.mount_angle = 90,
	.cam_vreg = msm_cam_vreg,
	.num_vreg = ARRAY_SIZE(msm_cam_vreg),
	.gpio_conf = &gpio_conf_ov2655,
};

static struct msm_camera_sensor_flash_src msm_flash_src_ov2655 = {
	.flash_sr_type = MSM_CAMERA_FLASH_SRC_LED2,
	._fsrc.ext_driver_src.led_en = GPIO_NOT_CONFIGURED,
	
};

static struct msm_camera_sensor_flash_data flash_ov2655 = {
	.flash_type             = MSM_CAMERA_FLASH_LED,
	.flash_src              = &msm_flash_src_ov2655,
};
#else
static struct msm_camera_sensor_flash_data flash_ov2655 = {
	.flash_type     = MSM_CAMERA_FLASH_NONE,
};
#endif

static struct msm_camera_sensor_info msm_camera_sensor_ov2655_data = {
	.sensor_name    = "ov2655",
	.sensor_reset_enable = 1,
	.pmic_gpio_enable  = 1,
	.sensor_reset   = GPIO_NOT_CONFIGURED,
	.sensor_pwd     = GPIO_NOT_CONFIGURED,
	.pdata          = &msm_camera_device_data_csi1[1],
	.flash_data     = &flash_ov2655,
	.sensor_platform_info   = &sensor_board_info_ov2655,
	.csi_if                 = 1,
	.camera_type	= BACK_CAMERA_2D,
	.sensor_type = YUV_SENSOR,
};
#endif
#elif defined(CONFIG_Q801_VOBIS)
/*renwei add it for ov2655 camera at 2012-8-1*/
#ifdef CONFIG_OV2655
static struct msm_camera_sensor_platform_info sensor_board_info_ov2655 = {
	//.mount_angle = 90,
	.cam_vreg = msm_cam_vreg,
	.num_vreg = ARRAY_SIZE(msm_cam_vreg),
	.gpio_conf = &gpio_conf_ov2655,
};
/*lilonghui modify all for the front camera can not support the flash led ,delete it 2012-8-*/
static struct msm_camera_sensor_flash_data flash_ov2655 = {
	.flash_type     = MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_sensor_info msm_camera_sensor_ov2655_data = {
	.sensor_name    = "ov2655",
	.sensor_reset_enable = 1,
	.pmic_gpio_enable  = 1,
	.sensor_reset   = GPIO_NOT_CONFIGURED,
	.sensor_pwd     = GPIO_NOT_CONFIGURED,
	.pdata          = &msm_camera_device_data_csi0[0],
	.flash_data     = &flash_ov2655,
	.sensor_platform_info   = &sensor_board_info_ov2655,
	.csi_if                 = 1,
	.camera_type	= FRONT_CAMERA_2D,
	.sensor_type = YUV_SENSOR,
};
#endif
/*add end*/
#endif
/*renwei add it for the main camera at 2012-5-31*/
#ifdef CONFIG_OV8825
static struct msm_camera_gpio_conf gpio_conf_ov8825 = {
	.camera_off_table = camera_off_gpio_table,
	.camera_on_table = camera_on_gpio_table,
	.gpio_no_mux = 1,
};

/*lilonghui modify it for the camera flashlight 2012-8-15  if you use the camera flash light for Q801,you can open the CONFIG*/
#ifdef CAMERA_FLASH_LED
static struct msm_camera_sensor_flash_src msm_flash_src_ov8825 = {
	.flash_sr_type = MSM_CAMERA_FLASH_SRC_LED1,
	._fsrc.ext_driver_src.led_en = QRD_SKUA_GPIO_CAM_FASH_LED_PWM,
	._fsrc.ext_driver_src.led_flash_en = QRD_SKUA_GPIO_CAM_FASH_LED_PWN,
};

static struct msm_camera_sensor_flash_data flash_ov8825 = {
	.flash_type             = MSM_CAMERA_FLASH_LED,
	.flash_src              = &msm_flash_src_ov8825,
};
#else
static struct msm_camera_sensor_flash_data flash_ov8825 = {
	.flash_type     = MSM_CAMERA_FLASH_NONE,
};
#endif
/*end*/
static struct msm_camera_sensor_platform_info sensor_board_info_ov8825 = {
	//.mount_angle  = 90,
	.cam_vreg = msm_cam_vreg,
	.num_vreg = ARRAY_SIZE(msm_cam_vreg),
	.gpio_conf = &gpio_conf_ov8825,
};

static struct msm_actuator_info msm_act_main_cam_3_info = {
	.board_info     = &msm_act_main_cam_i2c_info,
	.cam_name   = MSM_ACTUATOR_MAIN_CAM_3,
	.bus_id         = MSM_GSBI0_QUP_I2C_BUS_ID,
	.vcm_pwd        = GPIO_SKU3_CAM_5MP_CAM_DRIVER_PWDN,
	.vcm_enable     = 0,
};

static struct msm_camera_sensor_info msm_camera_sensor_ov8825_data = {
	.sensor_name    = "ov8825",
	.sensor_reset_enable    = 1,
	.pmic_gpio_enable = 1,
	.sensor_reset           = GPIO_SKU3_CAM_5MP_CAMIF_RESET,
	.sensor_pwd     = GPIO_SKU3_CAM_5MP_SHDN_N,
	.pdata  = &msm_camera_device_data_csi1[1],
	.flash_data     = &flash_ov8825,
	.sensor_platform_info = &sensor_board_info_ov8825,
	.csi_if = 1,
	.camera_type = BACK_CAMERA_2D,
	.sensor_type = BAYER_SENSOR,
	.actuator_info = &msm_act_main_cam_3_info,
};
#endif
#ifdef CONFIG_AR0543
static struct msm_camera_sensor_platform_info sensor_board_info_ar0543 = {
 	.mount_angle    = 90,
 	.cam_vreg       = msm_cam_vreg,
 	.num_vreg       = ARRAY_SIZE(msm_cam_vreg),
 	.gpio_conf      = &gpio_conf_ar0543,
};

static struct msm_camera_sensor_flash_src msm_flash_src_ar0543 = {
	.flash_sr_type                     = MSM_CAMERA_FLASH_SRC_LED2,
	._fsrc.ext_driver_src.led_en       = GPIO_NOT_CONFIGURED,
	
};

static struct msm_camera_sensor_flash_data flash_ar0543 = {
	.flash_type             = MSM_CAMERA_FLASH_LED,
	.flash_src              = &msm_flash_src_ar0543,
};

static struct msm_camera_sensor_info msm_camera_sensor_ar0543_data = {
	.sensor_name            = "ar0543",
	.sensor_reset_enable    = 1,
	.pmic_gpio_enable       = 1,
	.sensor_reset           = GPIO_NOT_CONFIGURED,
	.sensor_pwd             = GPIO_NOT_CONFIGURED,
	.pdata			= &msm_camera_device_data_csi1[0],
	.flash_data             = &flash_ar0543,
	.sensor_platform_info   = &sensor_board_info_ar0543,
	.csi_if                 = 1,
	.camera_type	          = BACK_CAMERA_2D,
	.sensor_type            = BAYER_SENSOR,
};
#endif
/*renwei add it for s5k5ca at 2012-9-26*/
#ifdef CONFIG_S5K5CA
static struct msm_camera_sensor_platform_info sensor_board_info_s5k5ca = {
	.mount_angle = 90,
	.cam_vreg = msm_cam_vreg,
	.num_vreg = ARRAY_SIZE(msm_cam_vreg),
	.gpio_conf = &gpio_conf_s5k5ca,
};

static struct msm_camera_sensor_flash_src msm_flash_src_s5k5ca = {
	.flash_sr_type                     = MSM_CAMERA_FLASH_SRC_LED2,
	._fsrc.ext_driver_src.led_en       = GPIO_NOT_CONFIGURED,
};

static struct msm_camera_sensor_flash_data flash_s5k5ca = {
	.flash_type     = MSM_CAMERA_FLASH_LED,
	.flash_src      = &msm_flash_src_s5k5ca,
};
 
static struct msm_camera_sensor_info msm_camera_sensor_s5k5ca_data = {
	.sensor_name    = "s5k5ca",
	.sensor_reset_enable = 1,
	.pmic_gpio_enable  = 1,
	.sensor_reset   = GPIO_NOT_CONFIGURED,
	.sensor_pwd     = GPIO_NOT_CONFIGURED,
	.pdata          = &msm_camera_device_data_csi1[0],
	.flash_data     = &flash_s5k5ca,
	.sensor_platform_info   = &sensor_board_info_s5k5ca,
	.csi_if                 = 1,
	.camera_type	= BACK_CAMERA_2D,
	.sensor_type = YUV_SENSOR,
};
 #endif
 /*add end*/
#ifdef CONFIG_MT9E013
static struct msm_camera_sensor_flash_data flash_mt9e013 = {
	.flash_type             = MSM_CAMERA_FLASH_LED,
	.flash_src              = &msm_flash_src
};

static struct msm_camera_sensor_platform_info sensor_board_info_mt9e013 = {
	.mount_angle	= 90,
	.cam_vreg = msm_cam_vreg,
	.num_vreg = ARRAY_SIZE(msm_cam_vreg),
	.gpio_conf = &gpio_conf_mt9e013,
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9e013_data = {
	.sensor_name    = "mt9e013",
	.sensor_reset_enable = 1,
	.pmic_gpio_enable    = 0,
	.pdata                  = &msm_camera_device_data_csi1[1],
	.flash_data             = &flash_mt9e013,
	.sensor_platform_info   = &sensor_board_info_mt9e013,
	.csi_if                 = 1,
	.camera_type = BACK_CAMERA_2D,
	.sensor_type = BAYER_SENSOR,
};
#endif

#ifdef CONFIG_IMX072
static struct msm_camera_sensor_platform_info imx072_sensor_7627a_info = {
	.mount_angle = 90
};

static struct msm_camera_sensor_flash_data flash_imx072 = {
	.flash_type             = MSM_CAMERA_FLASH_LED,
	.flash_src              = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_imx072_data = {
	.sensor_name    = "imx072",
	.sensor_reset_enable = 1,
	.sensor_reset   = GPIO_CAM_GP_CAMIF_RESET_N, /* TODO 106,*/
	.pmic_gpio_enable  = 0,
	.sensor_pwd             = 85,
	.vcm_pwd                = GPIO_CAM_GP_CAM_PWDN,
	.vcm_enable             = 1,
	.pdata                  = &msm_camera_device_data_csi0[0],
	.flash_data             = &flash_imx072,
	.sensor_platform_info = &imx072_sensor_7627a_info,
	.csi_if                 = 1
};

static struct platform_device msm_camera_sensor_imx072 = {
	.name   = "msm_camera_imx072",
	.dev    = {
		.platform_data = &msm_camera_sensor_imx072_data,
	},
};
#endif

#ifdef CONFIG_WEBCAM_OV9726
static struct msm_camera_sensor_flash_data flash_ov9726 = {
	.flash_type             = MSM_CAMERA_FLASH_LED,
	.flash_src              = &msm_flash_src
};

static struct msm_camera_sensor_platform_info sensor_board_info_ov9726 = {
	.mount_angle	= 90,
	.cam_vreg = msm_cam_vreg,
	.num_vreg = ARRAY_SIZE(msm_cam_vreg),
	.gpio_conf = &gpio_conf_ov9726,
};

static struct msm_camera_sensor_info msm_camera_sensor_ov9726_data = {
	.sensor_name    = "ov9726",
	.sensor_reset_enable = 0,
	.pmic_gpio_enable  = 0,
	.pdata                  = &msm_camera_device_data_csi0[0],
	.flash_data             = &flash_ov9726,
	.sensor_platform_info   = &sensor_board_info_ov9726,
	.csi_if                 = 1,
	.camera_type = FRONT_CAMERA_2D,
	.sensor_type = BAYER_SENSOR,
};
#endif

static struct platform_device msm_camera_server = {
	.name = "msm_cam_server",
	.id = 0,
};

//Start === Allen
#ifdef CAMERA_FLASH_LED
void camera_flash_setup(void)
{
	int rc;
	if(is_rgtk_product(RGTK_PRODUCT_Q801)){
		rc = gpio_tlmm_config(gpio_cfg[GPIO_CAM_TRUE_FLASH_PWM_INDEX], GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("%s: gpio_tlmm_config for %d failed\n",
				__func__, gpio_num[GPIO_CAM_TRUE_FLASH_PWM_INDEX]);			
		}

		rc = gpio_tlmm_config(gpio_cfg[GPIO_CAM_TRUE_FLASH_PWN_INDEX], GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("%s: gpio_tlmm_config for %d failed\n",
				__func__, gpio_num[GPIO_CAM_TRUE_FLASH_PWN_INDEX]);
		}

		#ifdef CONFIG_OV5647
		msm_flash_src_ov5647.flash_sr_type = MSM_CAMERA_FLASH_SRC_LED1;
		msm_flash_src_ov5647._fsrc.ext_driver_src.led_en = gpio_num[GPIO_CAM_TRUE_FLASH_PWM_INDEX];
		msm_flash_src_ov5647._fsrc.ext_driver_src.led_flash_en = gpio_num[GPIO_CAM_TRUE_FLASH_PWN_INDEX];
		#endif
		#ifdef CONFIG_OV8825
		msm_flash_src_ov8825.flash_sr_type = MSM_CAMERA_FLASH_SRC_LED1;
		msm_flash_src_ov8825._fsrc.ext_driver_src.led_en = gpio_num[GPIO_CAM_TRUE_FLASH_PWM_INDEX];
		msm_flash_src_ov8825._fsrc.ext_driver_src.led_flash_en = gpio_num[GPIO_CAM_TRUE_FLASH_PWN_INDEX];
		#endif
	}

	if(is_rgtk_product(RGTK_PRODUCT_Q802) || is_rgtk_product(RGTK_PRODUCT_Q803) || is_rgtk_product(RGTK_PRODUCT_Q203)){
		rc = gpio_tlmm_config(gpio_cfg[GPIO_CAM_FAKE_FLASH_EN_INDEX], GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("%s: gpio_tlmm_config for %d failed\n",
				__func__, gpio_num[GPIO_CAM_FAKE_FLASH_EN_INDEX]);
		}
		
		#ifdef CONFIG_OV5647
		msm_flash_src_ov5647.flash_sr_type = MSM_CAMERA_FLASH_SRC_LED2;
		msm_flash_src_ov5647._fsrc.ext_driver_src.led_en = gpio_num[GPIO_CAM_FAKE_FLASH_EN_INDEX];
		#endif

		#ifdef CONFIG_OV8825
		msm_flash_src_ov8825.flash_sr_type = MSM_CAMERA_FLASH_SRC_LED2;
		msm_flash_src_ov8825._fsrc.ext_driver_src.led_en = gpio_num[GPIO_CAM_FAKE_FLASH_EN_INDEX];	
		#endif

		#ifdef CONFIG_Q203
		#ifdef CONFIG_OV2655
		msm_flash_src_ov2655.flash_sr_type = MSM_CAMERA_FLASH_SRC_LED2;
		msm_flash_src_ov2655._fsrc.ext_driver_src.led_en = gpio_num[GPIO_CAM_FAKE_FLASH_EN_INDEX];	
		#endif
                #ifdef CONFIG_AR0543
                msm_flash_src_ar0543.flash_sr_type = MSM_CAMERA_FLASH_SRC_LED2;
		msm_flash_src_ar0543._fsrc.ext_driver_src.led_en = gpio_num[GPIO_CAM_FAKE_FLASH_EN_INDEX];
                #endif
                #ifdef CCONFIG_S5K5CA
                msm_flash_src_s5k5ca.flash_sr_type = MSM_CAMERA_FLASH_SRC_LED2;
		msm_flash_src_s5k5ca._fsrc.ext_driver_src.led_en = gpio_num[GPIO_CAM_FAKE_FLASH_EN_INDEX];
                #endif
		#endif

	}
}
#endif
//End === Allen
/*lilonghui add it for the camera gpio init 2012-9-21*/
#ifdef CONFIG_Q203
static void init_camera_gpio(void){
      int rc;
     if(is_rgtk_product(RGTK_PRODUCT_Q203)){
		rc = gpio_tlmm_config(gpio_num[GPIO_CAM_FRONT_RESET_INDEX], GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("%s: gpio_tlmm_config for %d failed\n",
				__func__, gpio_num[GPIO_CAM_FRONT_RESET_INDEX]);
		}
		rc = gpio_tlmm_config(gpio_num[GPIO_CAM_FRONT_PWDN_INDEX], GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("%s: gpio_tlmm_config for %d failed\n",
				__func__, gpio_num[GPIO_CAM_FRONT_PWDN_INDEX]);
		}
		rc = gpio_tlmm_config(gpio_num[GPIO_CAM_BACK_RESET_INDEX], GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("%s: gpio_tlmm_config for %d failed\n",
				__func__, gpio_num[GPIO_CAM_FRONT_RESET_INDEX]);
		}
		rc = gpio_tlmm_config(gpio_num[GPIO_CAM_BACK_PWDN_INDEX], GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("%s: gpio_tlmm_config for %d failed\n",
				__func__, gpio_num[GPIO_CAM_FRONT_PWDN_INDEX]);
		}

  }
}
#endif
static void __init msm7x27a_init_cam(void)
{

	if (!(machine_is_msm7x27a_ffa() || 
			machine_is_msm7625a_ffa() ||
			machine_is_msm7627a_qrd1() ||
			machine_is_msm8625_qrd7() ||
			machine_is_msm8625_evb() ||
			machine_is_msm8625_qrd5() ||
		   	machine_is_msm7x27a_qrd5a() ||
			machine_is_msm8625_skua()) ||
			machine_is_msm8625_ffa()) {
#ifdef CONFIG_S5K4E1
		sensor_board_info_s5k4e1.cam_vreg = NULL;
		sensor_board_info_s5k4e1.num_vreg = 0;
#endif
#ifdef CONFIG_MT9E013
		sensor_board_info_mt9e013.cam_vreg = NULL;
		sensor_board_info_mt9e013.num_vreg = 0;
#endif
#ifdef CONFIG_WEBCAM_OV9726
		sensor_board_info_ov9726.cam_vreg = NULL;
		sensor_board_info_ov9726.num_vreg = 0;
#endif
#ifdef CONFIG_WEBCAM_OV7692_QRD
		sensor_board_info_ov7692.cam_vreg = NULL;
		sensor_board_info_ov7692.num_vreg = 0;
#endif
#ifdef CONFIG_OV5647
		sensor_board_info_ov5647.cam_vreg = NULL;
		sensor_board_info_ov5647.num_vreg = 0;
#endif
/*renwei add it for ov2655 camera at 2012-8-1*/
#ifdef CONFIG_OV2655
		sensor_board_info_ov2655.cam_vreg = NULL;
		sensor_board_info_ov2655.num_vreg = 0;
#endif
/*add end*/
#ifdef CONFIG_OV8825
                sensor_board_info_ov8825.cam_vreg = NULL;
                sensor_board_info_ov8825.num_vreg = 0;
#endif

	}else if (machine_is_msm8625_qrd7())
	{
		//Add SKU7 specific settings
#ifdef CONFIG_OV5647
		msm_camera_sensor_ov5647_data.sensor_reset = camera_gpio_rear_qrd7[1];
		msm_camera_sensor_ov5647_data.sensor_pwd = camera_gpio_rear_qrd7[0];
		msm_camera_sensor_ov5647_data.vcm_pwd = 0;
		msm_camera_sensor_ov5647_data.vcm_enable = 0;
	
#endif
/*renwei add it for ov2655 camera at 2012-8-1*/
#ifdef CONFIG_OV2655
		msm_camera_sensor_ov2655_data.sensor_reset = camera_gpio_rear_qrd7[1];
		msm_camera_sensor_ov2655_data.sensor_pwd = camera_gpio_rear_qrd7[0];
		msm_camera_sensor_ov2655_data.vcm_pwd = 0;
		msm_camera_sensor_ov2655_data.vcm_enable = 0;
		//msm_flash_src_ov2655.flash_sr_type = MSM_CAMERA_FLASH_SRC_LED2;
		//msm_flash_src_ov2655._fsrc.ext_driver_src.led_en = 96;
#endif
/*add end */
#ifdef CONFIG_WEBCAM_OV7692_QRD
		msm_camera_sensor_ov7692_data.sensor_reset = camera_gpio_front_qrd7[1];
		msm_camera_sensor_ov7692_data.sensor_pwd = camera_gpio_front_qrd7[0];
#endif
/*renwei add it for the front camera at 2012-6-12*/
#ifdef CONFIG_GC0339
		sensor_board_info_gc0339.mount_angle = 270;
		msm_camera_sensor_gc0339_data.sensor_reset = camera_gpio_front_qrd7[1];
		msm_camera_sensor_gc0339_data.sensor_pwd = camera_gpio_front_qrd7[0];
#endif
	}else if(machine_is_msm8625_evb()){
		//Add EVB &8x25 specific settings
#ifdef CONFIG_OV5647
		msm_camera_sensor_ov5647_data.sensor_reset = GPIO_SKU3_CAM_5MP_CAMIF_RESET;
		msm_camera_sensor_ov5647_data.sensor_pwd = GPIO_SKU3_CAM_5MP_SHDN_N;
		sensor_board_info_ov5647.mount_angle = 90;
#endif
/*renwei add it for ov2655 camera at 2012-8-1*/
#ifdef CONFIG_OV2655
		msm_camera_sensor_ov2655_data.sensor_reset = GPIO_SKU1_CAM_VGA_RESET_N;
		msm_camera_sensor_ov2655_data.sensor_pwd = GPIO_SKU3_CAM_5MP_SHDN_N;
		sensor_board_info_ov2655.mount_angle = 90;
#endif
/*add end*/
#ifdef CONFIG_WEBCAM_OV7692_QRD
		sensor_board_info_ov7692.mount_angle = 270;
		msm_camera_sensor_ov7692_data.sensor_reset = GPIO_SKU1_CAM_VGA_RESET_N;
		msm_camera_sensor_ov7692_data.sensor_pwd = GPIO_SKU1_CAM_VGA_SHDN;
#endif
/*renwei add it for the front camera at 2012-6-12*/
#ifdef CONFIG_GC0339
		sensor_board_info_gc0339.mount_angle = 90;
		msm_camera_sensor_gc0339_data.sensor_reset = GPIO_SKU1_CAM_VGA_RESET_N;
		msm_camera_sensor_gc0339_data.sensor_pwd = GPIO_SKU1_CAM_VGA_SHDN;
#endif

	}else if(machine_is_msm8625_qrd5() || machine_is_msm7x27a_qrd5a()){
/*lilonghui add it for the camera 2012-9-21*/
#ifdef CONFIG_Q203
                init_camera_gpio();
#endif
//Add SKU5 &8x25 specific settings
#ifdef CONFIG_OV5647_TRULY_CM6868
		msm_camera_sensor_ov5647_truly_cm6868_data.sensor_reset = gpio_num[GPIO_CAM_BACK_RESET_INDEX];
		msm_camera_sensor_ov5647_truly_cm6868_data.sensor_pwd = gpio_num[GPIO_CAM_BACK_PWDN_INDEX];
		sensor_board_info_ov5647_truly_cm6868.mount_angle = 90;
		msm_flash_src_ov5647_truly_cm6868.flash_sr_type = MSM_CAMERA_FLASH_SRC_LED2;
#endif
/*lilonghui add it for all the project camera if you add a new camera pls add it according as below 2012-8-13*/
#ifdef CONFIG_OV5647

#ifdef CONFIG_Q801
                msm_camera_sensor_ov5647_data.camera_product_type =CAMERA_PRODUCT_ID_Q801;
                msm_camera_sensor_ov5647_data.customer_name = CAMERA_CUSTOMER_QC;
	        msm_camera_sensor_ov5647_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_ov5647_data.camera_sensor_ic_name=CAMERA_IC_NAME_OV5647;
                sensor_board_info_ov5647.mount_angle = 90;
#elif defined(CONFIG_Q802_35)
                msm_camera_sensor_ov5647_data.camera_product_type =CAMERA_PRODUCT_ID_Q802;
                msm_camera_sensor_ov5647_data.customer_name = CAMERA_CUSTOMER_35;
	        msm_camera_sensor_ov5647_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_ov5647_data.camera_sensor_ic_name=CAMERA_IC_NAME_OV5647;
               sensor_board_info_ov5647.mount_angle = 90;
#elif defined(CONFIG_Q802_QC)
                msm_camera_sensor_ov5647_data.camera_product_type =CAMERA_PRODUCT_ID_Q802;
                msm_camera_sensor_ov5647_data.customer_name = CAMERA_CUSTOMER_QC;
	        msm_camera_sensor_ov5647_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_ov5647_data.camera_sensor_ic_name=CAMERA_IC_NAME_OV5647;
               sensor_board_info_ov5647.mount_angle = 90;
#elif defined(CONFIG_Q802_CY)          
                msm_camera_sensor_ov5647_data.camera_product_type =CAMERA_PRODUCT_ID_Q802;
                msm_camera_sensor_ov5647_data.customer_name =CAMERA_CUSTOMER_CY;
	        msm_camera_sensor_ov5647_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_ov5647_data.camera_sensor_ic_name=CAMERA_IC_NAME_OV5647;
                sensor_board_info_ov5647.mount_angle = 90;   
#elif  defined(CONFIG_Q802_S21)
                msm_camera_sensor_ov5647_data.camera_product_type =CAMERA_PRODUCT_ID_Q802;
                msm_camera_sensor_ov5647_data.customer_name = CAMERA_CUSTOMER_S21;
	        msm_camera_sensor_ov5647_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_ov5647_data.camera_sensor_ic_name=CAMERA_IC_NAME_OV5647;
                sensor_board_info_ov5647.mount_angle = 90;
#elif  defined(CONFIG_Q802_S22)
                msm_camera_sensor_ov5647_data.camera_product_type =CAMERA_PRODUCT_ID_Q802;
                msm_camera_sensor_ov5647_data.customer_name = CAMERA_CUSTOMER_S22;
	        msm_camera_sensor_ov5647_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_ov5647_data.camera_sensor_ic_name=CAMERA_IC_NAME_OV5647;
                sensor_board_info_ov5647.mount_angle = 90; 
#elif  defined(CONFIG_Q803_AMOI)
                msm_camera_sensor_ov5647_data.camera_product_type =CAMERA_PRODUCT_ID_Q803;
                msm_camera_sensor_ov5647_data.customer_name = CAMERA_CUSTOMER_AMOI;
	        msm_camera_sensor_ov5647_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_ov5647_data.camera_sensor_ic_name=CAMERA_IC_NAME_OV5647;
                sensor_board_info_ov5647.mount_angle = 270; 
#else         
                msm_camera_sensor_ov5647_data.camera_product_type =CAMERA_PRODUCT_ID_NON;
                msm_camera_sensor_ov5647_data.customer_name = CAMERA_CUSTOMER_NON;
	        msm_camera_sensor_ov5647_data.module_sensor_name =CAMERA_MODULE_NON;
                msm_camera_sensor_ov5647_data.camera_sensor_ic_name=CAMERA_IC_NAME_OV5647;
                sensor_board_info_ov5647.mount_angle = 90;

#endif
                msm_camera_sensor_ov5647_data.sensor_reset = GPIO_SKU3_CAM_5MP_CAMIF_RESET;
		msm_camera_sensor_ov5647_data.sensor_pwd = GPIO_SKU3_CAM_5MP_SHDN_N;
		msm_act_main_cam_6_info.vcm_pwd = gpio_num[GPIO_CAM_DRIVER_PWDN];
#endif
#ifdef CONFIG_OV8825
#ifdef CONFIG_Q801
               printk("%s:lilonghui call CONFIG_Q801 %d \n",__func__,__LINE__);
               msm_camera_sensor_ov8825_data.camera_product_type =CAMERA_PRODUCT_ID_Q801;
               msm_camera_sensor_ov8825_data.customer_name = CAMERA_CUSTOMER_QC;
	       msm_camera_sensor_ov8825_data.module_sensor_name =CAMERA_MODULE_TRULY;
               msm_camera_sensor_ov8825_data.camera_sensor_ic_name=CAMERA_IC_NAME_OV8825;
               sensor_board_info_ov8825.mount_angle = 90;
#elif defined(CONFIG_Q802_35)
               msm_camera_sensor_ov8825_data.camera_product_type =CAMERA_PRODUCT_ID_Q802;
               msm_camera_sensor_ov8825_data.customer_name = CAMERA_CUSTOMER_35;
	       msm_camera_sensor_ov8825_data.module_sensor_name =CAMERA_MODULE_TRULY;
               msm_camera_sensor_ov8825_data.camera_sensor_ic_name=CAMERA_IC_NAME_OV8825;
               sensor_board_info_ov8825.mount_angle = 90;
#elif defined(CONFIG_Q802_QC)
               msm_camera_sensor_ov8825_data.camera_product_type =CAMERA_PRODUCT_ID_Q802;
               msm_camera_sensor_ov8825_data.customer_name = CAMERA_CUSTOMER_QC;
	       msm_camera_sensor_ov8825_data.module_sensor_name =CAMERA_MODULE_TRULY;
               msm_camera_sensor_ov8825_data.camera_sensor_ic_name=CAMERA_IC_NAME_OV8825;
               sensor_board_info_ov8825.mount_angle = 90;
#elif  defined(CONFIG_Q802_CY)
         
               msm_camera_sensor_ov8825_data.camera_product_type =CAMERA_PRODUCT_ID_Q802;
               msm_camera_sensor_ov8825_data.customer_name = CAMERA_CUSTOMER_CY;
	       msm_camera_sensor_ov8825_data.module_sensor_name =CAMERA_MODULE_TRULY;
               msm_camera_sensor_ov8825_data.camera_sensor_ic_name=CAMERA_IC_NAME_OV8825;
               sensor_board_info_ov8825.mount_angle = 90;   
#elif defined(CONFIG_Q802_S21)
               msm_camera_sensor_ov8825_data.camera_product_type =CAMERA_PRODUCT_ID_Q802;
               msm_camera_sensor_ov8825_data.customer_name = CAMERA_CUSTOMER_S21;
	       msm_camera_sensor_ov8825_data.module_sensor_name =CAMERA_MODULE_TRULY;
               msm_camera_sensor_ov8825_data.camera_sensor_ic_name=CAMERA_IC_NAME_OV8825;
              sensor_board_info_ov8825.mount_angle = 90;
#elif  defined(CONFIG_Q802_S22)
               msm_camera_sensor_ov8825_data.camera_product_type =CAMERA_PRODUCT_ID_Q802;
               msm_camera_sensor_ov8825_data.customer_name = CAMERA_CUSTOMER_S22;
	       msm_camera_sensor_ov8825_data.module_sensor_name =CAMERA_MODULE_TRULY;
               msm_camera_sensor_ov8825_data.camera_sensor_ic_name=CAMERA_IC_NAME_OV8825;
               sensor_board_info_ov8825.mount_angle = 90; 
#elif defined(CONFIG_Q803_AMOI)
               msm_camera_sensor_ov8825_data.camera_product_type =CAMERA_PRODUCT_ID_Q803;
               msm_camera_sensor_ov8825_data.customer_name = CAMERA_CUSTOMER_AMOI;
	       msm_camera_sensor_ov8825_data.module_sensor_name =CAMERA_MODULE_TRULY;
               msm_camera_sensor_ov8825_data.camera_sensor_ic_name=CAMERA_IC_NAME_OV8825;
               sensor_board_info_ov8825.mount_angle = 90;
#else
	       msm_camera_sensor_ov8825_data.camera_product_type =CAMERA_PRODUCT_ID_NON;
               msm_camera_sensor_ov8825_data.customer_name = CAMERA_CUSTOMER_NON;
	       msm_camera_sensor_ov8825_data.module_sensor_name =CAMERA_MODULE_NON;
               msm_camera_sensor_ov8825_data.camera_sensor_ic_name=CAMERA_IC_NAME_OV8825;
               sensor_board_info_ov8825.mount_angle = 90;
#endif 
#endif

#ifdef CONFIG_GC0339       
#ifdef CONFIG_Q801
                msm_camera_sensor_gc0339_data.camera_product_type =CAMERA_PRODUCT_ID_Q801;
                msm_camera_sensor_gc0339_data.customer_name = CAMERA_CUSTOMER_QC;
	        msm_camera_sensor_gc0339_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_gc0339_data.camera_sensor_ic_name=CAMERA_IC_NAME_GC0339;
                sensor_board_info_gc0339.mount_angle = 90;
#elif defined(CONFIG_Q802_35)
                msm_camera_sensor_gc0339_data.camera_product_type =CAMERA_PRODUCT_ID_Q802;
                msm_camera_sensor_gc0339_data.customer_name = CAMERA_CUSTOMER_35;
	        msm_camera_sensor_gc0339_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_gc0339_data.camera_sensor_ic_name=CAMERA_IC_NAME_GC0339;
                sensor_board_info_gc0339.mount_angle = 90;
#elif defined(CONFIG_Q802_QC)
                msm_camera_sensor_gc0339_data.camera_product_type =CAMERA_PRODUCT_ID_Q802;
                msm_camera_sensor_gc0339_data.customer_name = CAMERA_CUSTOMER_QC;
	        msm_camera_sensor_gc0339_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_gc0339_data.camera_sensor_ic_name=CAMERA_IC_NAME_GC0339;
                sensor_board_info_gc0339.mount_angle = 90;
#elif defined(CONFIG_Q802_CY)
                msm_camera_sensor_gc0339_data.camera_product_type =CAMERA_PRODUCT_ID_Q802;
                msm_camera_sensor_gc0339_data.customer_name = CAMERA_CUSTOMER_CY;
	        msm_camera_sensor_gc0339_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_gc0339_data.camera_sensor_ic_name=CAMERA_IC_NAME_GC0339;
                sensor_board_info_gc0339.mount_angle = 90;   
#elif  defined(CONFIG_Q802_S21)
                msm_camera_sensor_gc0339_data.camera_product_type =CAMERA_PRODUCT_ID_Q802;
                msm_camera_sensor_gc0339_data.customer_name = CAMERA_CUSTOMER_S21;
	        msm_camera_sensor_gc0339_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_gc0339_data.camera_sensor_ic_name=CAMERA_IC_NAME_GC0339;
                sensor_board_info_gc0339.mount_angle = 90;
#elif  defined(CONFIG_Q802_S22)
                msm_camera_sensor_gc0339_data.camera_product_type =CAMERA_PRODUCT_ID_Q802;
                msm_camera_sensor_gc0339_data.customer_name =CAMERA_CUSTOMER_S22;
	        msm_camera_sensor_gc0339_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_gc0339_data.camera_sensor_ic_name=CAMERA_IC_NAME_GC0339;
                sensor_board_info_gc0339.mount_angle = 90;
#elif  defined(CONFIG_Q803_AMOI)
                printk("%s:lilonghui call the camera %d \n",__func__,__LINE__);
                msm_camera_sensor_gc0339_data.camera_product_type =CAMERA_PRODUCT_ID_Q803;
                msm_camera_sensor_gc0339_data.customer_name = CAMERA_CUSTOMER_AMOI;
	        msm_camera_sensor_gc0339_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_gc0339_data.camera_sensor_ic_name=CAMERA_IC_NAME_GC0339;
                sensor_board_info_gc0339.mount_angle = 90;
#elif  defined(CONFIG_Q803_CY)
		printk("%s:lilonghui call the camera %d \n",__func__,__LINE__);
                msm_camera_sensor_gc0339_data.camera_product_type =CAMERA_PRODUCT_ID_Q803;
                msm_camera_sensor_gc0339_data.customer_name = CAMERA_CUSTOMER_CY;
	        msm_camera_sensor_gc0339_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_gc0339_data.camera_sensor_ic_name=CAMERA_IC_NAME_GC0339;
                sensor_board_info_gc0339.mount_angle = 90;
#elif  defined(CONFIG_Q203)
                msm_camera_sensor_gc0339_data.camera_product_type =CAMERA_PRODUCT_ID_NON;
                msm_camera_sensor_gc0339_data.customer_name = CAMERA_CUSTOMER_NON;
	        msm_camera_sensor_gc0339_data.module_sensor_name =CAMERA_MODULE_NON;
                msm_camera_sensor_gc0339_data.camera_sensor_ic_name=CAMERA_IC_NAME_GC0339; 
                sensor_board_info_gc0339.mount_angle = 270;
                msm_camera_sensor_gc0339_data.sensor_reset = gpio_num[GPIO_CAM_FRONT_RESET_INDEX];
		msm_camera_sensor_gc0339_data.sensor_pwd = gpio_num[GPIO_CAM_FRONT_PWDN_INDEX];
#elif  defined(CONFIG_DC205_YL)
                msm_camera_sensor_gc0339_data.camera_product_type =CAMERA_PRODUCT_ID_NON;
                msm_camera_sensor_gc0339_data.customer_name = CAMERA_CUSTOMER_NON;
	        msm_camera_sensor_gc0339_data.module_sensor_name =CAMERA_MODULE_NON;
                msm_camera_sensor_gc0339_data.camera_sensor_ic_name=CAMERA_IC_NAME_GC0339; 
                sensor_board_info_gc0339.mount_angle = 90;
                msm_camera_sensor_gc0339_data.sensor_reset = gpio_num[GPIO_CAM_BACK_RESET_INDEX];
		msm_camera_sensor_gc0339_data.sensor_pwd = gpio_num[GPIO_CAM_BACK_PWDN_INDEX];
                 
#else
                printk("%s:lilonghui call the camera %d\n",__func__,__LINE__);
                msm_camera_sensor_gc0339_data.camera_product_type =CAMERA_PRODUCT_ID_NON;
                msm_camera_sensor_gc0339_data.customer_name = CAMERA_CUSTOMER_NON;
	        msm_camera_sensor_gc0339_data.module_sensor_name =CAMERA_MODULE_NON;
                msm_camera_sensor_gc0339_data.camera_sensor_ic_name=CAMERA_IC_NAME_GC0339; 
                sensor_board_info_gc0339.mount_angle = 90;
#endif
                msm_camera_sensor_gc0339_data.sensor_reset = GPIO_SKU1_CAM_VGA_RESET_N;
		msm_camera_sensor_gc0339_data.sensor_pwd = GPIO_SKU1_CAM_VGA_SHDN;
#endif
#ifdef CONFIG_OV2655
#ifdef CONFIG_Q801

		msm_camera_sensor_ov2655_data.camera_product_type =CAMERA_PRODUCT_ID_Q801;
                msm_camera_sensor_ov2655_data.customer_name = CAMERA_CUSTOMER_QC;
	        msm_camera_sensor_ov2655_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_ov2655_data.camera_sensor_ic_name=CAMERA_IC_NAME_OV2655;
                sensor_board_info_ov2655.mount_angle = 90;
                msm_camera_sensor_ov2655_data.sensor_reset = GPIO_SKU1_CAM_VGA_RESET_N;
	      	msm_camera_sensor_ov2655_data.sensor_pwd = GPIO_SKU1_CAM_VGA_SHDN;
#elif defined(CONFIG_Q203)
		msm_camera_sensor_ov2655_data.camera_product_type =CAMERA_PRODUCT_ID_Q203;
                msm_camera_sensor_ov2655_data.customer_name = CAMERA_CUSTOMER_QC;
	        msm_camera_sensor_ov2655_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_ov2655_data.camera_sensor_ic_name=CAMERA_IC_NAME_OV2655;
                sensor_board_info_ov2655.mount_angle = 90;
		msm_camera_sensor_ov2655_data.sensor_reset =gpio_num[GPIO_CAM_BACK_RESET_INDEX];
		msm_camera_sensor_ov2655_data.sensor_pwd = gpio_num[GPIO_CAM_BACK_PWDN_INDEX];
#elif defined(CONFIG_Q801_VOBIS)
		msm_camera_sensor_ov2655_data.camera_product_type =CAMERA_PRODUCT_ID_Q203;
                msm_camera_sensor_ov2655_data.customer_name = CAMERA_CUSTOMER_QC;
	        msm_camera_sensor_ov2655_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_ov2655_data.camera_sensor_ic_name=CAMERA_IC_NAME_OV2655;
                sensor_board_info_ov2655.mount_angle = 270;
		msm_camera_sensor_ov2655_data.sensor_reset =gpio_num[GPIO_CAM_FRONT_RESET_INDEX];
		msm_camera_sensor_ov2655_data.sensor_pwd = gpio_num[GPIO_CAM_FRONT_PWDN_INDEX];
#else
		msm_camera_sensor_ov2655_data.camera_product_type =CAMERA_PRODUCT_ID_NON;
                msm_camera_sensor_ov2655_data.customer_name =CAMERA_CUSTOMER_NON;
	        msm_camera_sensor_ov2655_data.module_sensor_name =CAMERA_MODULE_NON;
                msm_camera_sensor_ov2655_data.camera_sensor_ic_name=CAMERA_MODULE_NON;
                sensor_board_info_ov2655.mount_angle = 90;
		msm_camera_sensor_ov2655_data.sensor_reset = GPIO_SKU1_CAM_VGA_RESET_N;
		msm_camera_sensor_ov2655_data.sensor_pwd = GPIO_SKU1_CAM_VGA_SHDN;
#endif

#endif
#ifdef CONFIG_AR0543
#ifdef CONFIG_Q203
		msm_camera_sensor_ar0543_data.camera_product_type =CAMERA_PRODUCT_ID_Q203;
                msm_camera_sensor_ar0543_data.customer_name = CAMERA_CUSTOMER_QC;
	        msm_camera_sensor_ar0543_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_ar0543_data.camera_sensor_ic_name=CAMERA_IC_NAME_AR0543;
                sensor_board_info_ar0543.mount_angle = 90;
		msm_camera_sensor_ar0543_data.sensor_reset =gpio_num[GPIO_CAM_BACK_RESET_INDEX];
		msm_camera_sensor_ar0543_data.sensor_pwd = gpio_num[GPIO_CAM_BACK_PWDN_INDEX];
#else
		msm_camera_sensor_ar0543_data.camera_product_type =CAMERA_PRODUCT_ID_NON;
                msm_camera_sensor_ar0543_data.customer_name = CAMERA_CUSTOMER_QC;
	        msm_camera_sensor_ar0543_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_ar0543_data.camera_sensor_ic_name=CAMERA_MODULE_NON;
                sensor_board_info_ar0543.mount_angle = 90;
		msm_camera_sensor_ar0543_data.sensor_reset =gpio_num[GPIO_CAM_BACK_RESET_INDEX];
		msm_camera_sensor_ar0543_data.sensor_pwd = gpio_num[GPIO_CAM_BACK_PWDN_INDEX];
#endif
#endif

#ifdef CONFIG_S5K5CA
#ifdef CONFIG_Q203
		msm_camera_sensor_s5k5ca_data.camera_product_type =CAMERA_PRODUCT_ID_Q203;
                msm_camera_sensor_s5k5ca_data.customer_name = CAMERA_CUSTOMER_QC;
	        msm_camera_sensor_s5k5ca_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_s5k5ca_data.camera_sensor_ic_name=CAMERA_IC_NAME_S5K5CA;
        	msm_camera_sensor_s5k5ca_data.sensor_reset = gpio_num[GPIO_CAM_BACK_RESET_INDEX];
		msm_camera_sensor_s5k5ca_data.sensor_pwd =gpio_num[GPIO_CAM_BACK_PWDN_INDEX];
		sensor_board_info_s5k5ca.mount_angle = 270;
#else
		msm_camera_sensor_s5k5ca_data.camera_product_type =CAMERA_PRODUCT_ID_NON;
                msm_camera_sensor_s5k5ca_data.customer_name = CAMERA_CUSTOMER_QC;
	        msm_camera_sensor_s5k5ca_data.module_sensor_name =CAMERA_MODULE_TRULY;
                msm_camera_sensor_s5k5ca_data.camera_sensor_ic_name=CAMERA_MODULE_NON;
		msm_camera_sensor_s5k5ca_data.sensor_reset = gpio_num[GPIO_CAM_BACK_RESET_INDEX];
		msm_camera_sensor_s5k5ca_data.sensor_pwd = gpio_num[GPIO_CAM_BACK_PWDN_INDEX];
		sensor_board_info_s5k5ca.mount_angle = 270;
#endif 
#endif 
/*end lilonghui 2012-8-13*/

/*renwei add it for the camera AF at 2012-5-25*/
#ifdef CONFIG_AD5046_ACT
		ad5046_actuator_info.vcm_pwd = gpio_num[GPIO_CAM_DRIVER_PWDN];
#endif
/*add end*/
#ifdef CONFIG_WEBCAM_OV7692_QRD
		sensor_board_info_ov7692.mount_angle = 90;//renwei modify it for the front camera at 2012-5-31
		msm_camera_sensor_ov7692_data.sensor_reset = GPIO_SKU1_CAM_VGA_RESET_N;
		msm_camera_sensor_ov7692_data.sensor_pwd = GPIO_SKU1_CAM_VGA_SHDN;
#endif

	        //Start === Allen
		#if defined(CAMERA_FLASH_LED)
		camera_flash_setup();
		#endif
		//End === Allen
	}else if(machine_is_msm8625_skua())
	{
#ifdef CONFIG_OV5647_SUNNY_P5V02S
		//Add SKUA specific settings, ov5647/Mp9v113 GPIOs
		msm_camera_sensor_ov5647_sunny_p5v02s_data.sensor_reset = QRD_SKUA_GPIO_CAM_5MP_RESET;
		msm_camera_sensor_ov5647_sunny_p5v02s_data.sensor_pwd = QRD_SKUA_GPIO_CAM_5MP_SHDN_EN;
		msm_camera_sensor_ov5647_sunny_p5v02s_data.vcm_pwd = 0;
		msm_camera_sensor_ov5647_sunny_p5v02s_data.vcm_enable = 0;
		sensor_board_info_ov5647_sunny_p5v02s.mount_angle = 90;
#endif

/*renwei add it for the ov2655 camera at 2012-8-1*/
#ifdef CONFIG_OV2655
		msm_camera_sensor_ov2655_data.sensor_reset = QRD_SKUA_GPIO_CAM_5MP_RESET;
		msm_camera_sensor_ov2655_data.sensor_pwd = QRD_SKUA_GPIO_CAM_5MP_SHDN_EN;
		msm_camera_sensor_ov2655_data.vcm_pwd = 0;
		msm_camera_sensor_ov2655_data.vcm_enable = 0;
		sensor_board_info_ov2655.mount_angle = 270;
#endif
/*add end*/
#ifdef CONFIG_OV5640
		msm_camera_sensor_ov5640_data.sensor_reset = QRD_SKUA_GPIO_CAM_5MP_RESET;
		msm_camera_sensor_ov5640_data.sensor_pwd = QRD_SKUA_GPIO_CAM_5MP_SHDN_EN;
		msm_camera_sensor_ov5640_data.vcm_pwd = 0;
		msm_camera_sensor_ov5640_data.vcm_enable = 0;
		sensor_board_info_ov5640.mount_angle = 270;
#endif

#ifdef CONFIG_MT9V113
		msm_camera_sensor_mt9v113_data.sensor_reset = QRD_SKUA_GPIO_CAM_3MP_RESET;
		msm_camera_sensor_mt9v113_data.sensor_pwd = QRD_SKUA_GPIO_CAM_3MP_PWDN;
#endif

	}

	platform_device_register(&msm_camera_server);
	if (machine_is_msm8625_surf() || machine_is_msm8625_evb() || 
		machine_is_msm8625_qrd7() || machine_is_msm8625_qrd5() || 
		machine_is_msm7x27a_qrd5a() || machine_is_msm8625_skua() || 
		machine_is_msm8625_evt()
		) {
		platform_device_register(&msm8625_device_csic0);
		platform_device_register(&msm8625_device_csic1);
	} else {
		platform_device_register(&msm7x27a_device_csic0);
		platform_device_register(&msm7x27a_device_csic1);
	}
	if (machine_is_msm8625_evb() || machine_is_msm8625_evt())
		*(int *) msm7x27a_device_clkctl.dev.platform_data = 1;
	platform_device_register(&msm7x27a_device_clkctl);
	platform_device_register(&msm7x27a_device_vfe);
}

static struct i2c_board_info i2c_camera_devices[] = {
	#if 0 //snow we only have the two drivers compatiable for V4l2, close other first.
	#ifdef CONFIG_S5K4E1
	{
		I2C_BOARD_INFO("s5k4e1", 0x36),
		.platform_data = &msm_camera_sensor_s5k4e1_data,
	},
	#endif
	#ifdef CONFIG_WEBCAM_OV9726
	{
		I2C_BOARD_INFO("ov9726", 0x10),
		.platform_data = &msm_camera_sensor_ov9726_data,
	},
	#endif
	#ifdef CONFIG_IMX072
	{
		I2C_BOARD_INFO("imx072", 0x34),
	},
	#endif
	#ifdef CONFIG_MT9E013
	{
		I2C_BOARD_INFO("mt9e013", 0x6C >> 2),
		.platform_data = &msm_camera_sensor_mt9e013_data,
	},CONFIG_AR0543
	#endif
	{
		I2C_BOARD_INFO("sc628a", 0x6E),
	},

#endif


#ifdef CONFIG_OV5647
	{
		I2C_BOARD_INFO("ov5647", 0x36 << 1),//original
		.platform_data = &msm_camera_sensor_ov5647_data,
	},
#endif

/*renwei add it for the main camera at 2012-5-31*/
#ifdef CONFIG_OV8825
	{
		I2C_BOARD_INFO("ov8825", 0x6C >> 3),
		.platform_data = &msm_camera_sensor_ov8825_data,
	},
#endif
/*lilonghui add it or the camera 2012-9-24*/
#ifdef CONFIG_AR0543
	{
		I2C_BOARD_INFO("ar0543", 0x6c+2),
		.platform_data = &msm_camera_sensor_ar0543_data,
	},
#endif
/*renwei add it for the s5k5ca at 2012-9-26*/
#ifdef CONFIG_S5K5CA
	{
		I2C_BOARD_INFO("s5k5ca", 0x78),
		.platform_data = &msm_camera_sensor_s5k5ca_data,
	},
#endif
/*add end*/
#ifdef CONFIG_WEBCAM_OV7692_QRD
	{
		I2C_BOARD_INFO("ov7692", 0x78),
		.platform_data = &msm_camera_sensor_ov7692_data,
	},
#endif
/*renwei add it for ov2655 camera at 2012-8-1*/
#ifdef CONFIG_OV2655
	{
		I2C_BOARD_INFO("ov2655", (0x60)), //0x30
		.platform_data = &msm_camera_sensor_ov2655_data,
	},
#endif
/*renwei add it for the front camera at 2012-6-12*/
#ifdef CONFIG_GC0339
	{
		I2C_BOARD_INFO("gc0339", 0x36),
		.platform_data = &msm_camera_sensor_gc0339_data,
	},
#endif

#ifdef CONFIG_OV5640
	{
		I2C_BOARD_INFO("ov5640", 0x78 - 2),//original78, sub 2 to avoid i2c conflicts
		.platform_data = &msm_camera_sensor_ov5640_data,
	},
#endif


#ifdef CONFIG_MT9V113
{
		I2C_BOARD_INFO("mt9v113", 0x7a), //0x45
		.platform_data = &msm_camera_sensor_mt9v113_data,
},
#endif

};

#if 0
static struct i2c_board_info i2c_camera_devices_qrd5[] = {
#ifdef CONFIG_OV5647_TRULY_CM6868
	{
		I2C_BOARD_INFO("ov5647_truly_cm6868", 0x36 << 1),//original
		.platform_data = &msm_camera_sensor_ov5647_truly_cm6868_data,
	},
#endif
#ifdef CONFIG_WEBCAM_OV7692_QRD
	{
		I2C_BOARD_INFO("ov7692", 0x78),
		.platform_data = &msm_camera_sensor_ov7692_data,
	},
#endif
#ifdef CONFIG_AR0543
	{
		I2C_BOARD_INFO("ar0543", 0x6c+2),
		.platform_data = &msm_camera_sensor_ar0543_data,
	},
#endif
};
#endif

static struct i2c_board_info i2c_camera_devices_skua[] = {
#ifdef CONFIG_OV5647_SUNNY_P5V02S
	{
		I2C_BOARD_INFO("ov5647_sunny_p5v02s", 0x36 << 1),//original
		.platform_data = &msm_camera_sensor_ov5647_sunny_p5v02s_data,
	},
#endif

#ifdef CONFIG_OV5647_SUNNY_P5V02S
	{
		I2C_BOARD_INFO("ov5647_sunny_p5v02s", 0x36 << 1),//original
		.platform_data = &msm_camera_sensor_ov5647_sunny_p5v02s_data,
	},
#endif
/*renwei add it for ov2566 camera at 2012-8-1*/
#ifdef CONFIG_OV2655
	{
		I2C_BOARD_INFO("ov2655", (0x60)), //0x30
		.platform_data = &msm_camera_sensor_ov2655_data,
	},
#endif
/*add end*/
#ifdef CONFIG_OV5640
	{
		I2C_BOARD_INFO("ov5640", 0x78 - 2),//original78, sub 2 to avoid i2c conflicts
		.platform_data = &msm_camera_sensor_ov5640_data,
	},
#endif
#ifdef CONFIG_OV8825
	{
		I2C_BOARD_INFO("ov8825", 0x6C >> 3),
		.platform_data = &msm_camera_sensor_ov8825_data,
	},
#endif

#ifdef CONFIG_MT9V113
{
		I2C_BOARD_INFO("mt9v113", 0x7a), //0x45
		.platform_data = &msm_camera_sensor_mt9v113_data,
},
#endif
};
#else //none-v4l2 below
static uint32_t camera_off_gpio_table[] = {
	GPIO_CFG(15, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
};

static uint32_t camera_on_gpio_table[] = {
	GPIO_CFG(15, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
};

#ifdef CONFIG_MSM_CAMERA_FLASH
static struct msm_camera_sensor_flash_src msm_flash_src = {
	.flash_sr_type = MSM_CAMERA_FLASH_SRC_EXT,
	._fsrc.ext_driver_src.led_en = GPIO_CAM_GP_LED_EN1,
	._fsrc.ext_driver_src.led_flash_en = GPIO_CAM_GP_LED_EN2,
};
#endif

static struct regulator_bulk_data regs_camera[] = {
	{ .supply = "msme1", .min_uV = 1800000, .max_uV = 1800000 },
	{ .supply = "gp2",   .min_uV = 2850000, .max_uV = 2850000 },
	{ .supply = "usb2",  .min_uV = 1800000, .max_uV = 1800000 },
};

static void qrd1_camera_gpio_cfg(void)
{

	int rc = 0;

	rc = gpio_request(QRD_GPIO_CAM_5MP_SHDN_EN, "ov5640");
	if (rc < 0)
		pr_err("%s: gpio_request---GPIO_CAM_5MP_SHDN_EN failed!",
				__func__);


	rc = gpio_tlmm_config(GPIO_CFG(QRD_GPIO_CAM_5MP_SHDN_EN, 0,
				GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP,
				GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (rc < 0) {
		pr_err("%s: unable to enable Power Down gpio for main"
				"camera!\n", __func__);
		gpio_free(QRD_GPIO_CAM_5MP_SHDN_EN);
	}


	rc = gpio_request(QRD_GPIO_CAM_5MP_RESET, "ov5640");
	if (rc < 0) {
		pr_err("%s: gpio_request---GPIO_CAM_5MP_RESET failed!",
				__func__);
		gpio_free(QRD_GPIO_CAM_5MP_SHDN_EN);
	}


	rc = gpio_tlmm_config(GPIO_CFG(QRD_GPIO_CAM_5MP_RESET, 0,
				GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP,
				GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (rc < 0) {
		pr_err("%s: unable to enable reset gpio for main camera!\n",
				__func__);
		gpio_free(QRD_GPIO_CAM_5MP_RESET);
	}

	rc = gpio_request(QRD_GPIO_CAM_3MP_PWDN, "ov7692");
	if (rc < 0)
		pr_err("%s: gpio_request---GPIO_CAM_3MP_PWDN failed!",
				__func__);

	rc = gpio_tlmm_config(GPIO_CFG(QRD_GPIO_CAM_3MP_PWDN, 0,
				GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP,
				GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (rc < 0) {
		pr_err("%s: unable to enable Power Down gpio for front"
				"camera!\n", __func__);
		gpio_free(QRD_GPIO_CAM_3MP_PWDN);
	}

	gpio_direction_output(QRD_GPIO_CAM_5MP_SHDN_EN, 1);
	gpio_direction_output(QRD_GPIO_CAM_5MP_RESET, 1);
	gpio_direction_output(QRD_GPIO_CAM_3MP_PWDN, 1);
}
#endif

//#ifndef CONFIG_MSM_CAMERA_V4L2
static void skua_camera_gpio_cfg(void)
{
	int rc = 0;
    pr_err("%s IN\r\n", __func__);

	rc = gpio_request(QRD_SKUA_GPIO_CAM_5MP_SHDN_EN, "ov5647");
	if (rc < 0)
		pr_err("%s: gpio_request---GPIO_CAM_5MP_SHDN_EN failed!",
				__func__);


	rc = gpio_tlmm_config(GPIO_CFG(QRD_SKUA_GPIO_CAM_5MP_SHDN_EN, 0,
				GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP,
				GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (rc < 0) {
		pr_err("%s: unable to enable Power Down gpio for main"
				"camera!\n", __func__);
		gpio_free(QRD_SKUA_GPIO_CAM_5MP_SHDN_EN);
	}


	rc = gpio_request(QRD_SKUA_GPIO_CAM_5MP_RESET, "ov5647");
	if (rc < 0) {
		pr_err("%s: gpio_request---GPIO_CAM_5MP_RESET failed!",
				__func__);
		gpio_free(QRD_SKUA_GPIO_CAM_5MP_SHDN_EN);
	}


	rc = gpio_tlmm_config(GPIO_CFG(QRD_SKUA_GPIO_CAM_5MP_RESET, 0,
				GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP,
				GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (rc < 0) {
		pr_err("%s: unable to enable reset gpio for main camera!\n",
				__func__);
		gpio_free(QRD_SKUA_GPIO_CAM_5MP_RESET);
	}

	rc = gpio_request(QRD_SKUA_GPIO_CAM_3MP_PWDN, "mt9v113");
	if (rc < 0)
		pr_err("%s: gpio_request---GPIO_CAM_3MP_PWDN failed!",
				__func__);

	rc = gpio_tlmm_config(GPIO_CFG(QRD_SKUA_GPIO_CAM_3MP_PWDN, 0,
				GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP,
				GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (rc < 0) {
		pr_err("%s: unable to enable Power Down gpio for front"
				"camera!\n", __func__);
		gpio_free(QRD_SKUA_GPIO_CAM_3MP_PWDN);
	}

	rc = gpio_request(QRD_SKUA_GPIO_CAM_3MP_RESET, "mt9v113");
	if (rc < 0) {
		pr_err("%s: gpio_request---GPIO_CAM_5MP_RESET failed!",
				__func__);
		gpio_free(QRD_SKUA_GPIO_CAM_3MP_PWDN);
	}

	rc = gpio_tlmm_config(GPIO_CFG(QRD_SKUA_GPIO_CAM_3MP_RESET, 0,
				GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP,
				GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (rc < 0) {
		pr_err("%s: unable to enable reset gpio for main camera!\n",
				__func__);
		gpio_free(QRD_SKUA_GPIO_CAM_3MP_RESET);
    }

	gpio_direction_output(QRD_SKUA_GPIO_CAM_5MP_SHDN_EN, 1);
	gpio_direction_output(QRD_SKUA_GPIO_CAM_5MP_RESET, 1);
	gpio_direction_output(QRD_SKUA_GPIO_CAM_3MP_PWDN, 1);
	gpio_direction_output(QRD_SKUA_GPIO_CAM_3MP_RESET, 1);
}
//#endif

#define GPIO_SKU3_CAM_5MP_SHDN_N   122        /* PWDN */
#define GPIO_SKU3_CAM_5MP_CAMIF_RESET   123    /* (board_is(EVT))?123:121 RESET */
#define GPIO_SKU3_CAM_5MP_CAM_DRIVER_PWDN 30

#define GPIO_SKU1_CAM_VGA_SHDN    18
#define GPIO_SKU1_CAM_VGA_RESET_N 29

static void evb_camera_gpio_cfg(void)
{
	int rc = 0;

	rc = gpio_request(gpio_num[GPIO_CAM_BACK_PWDN_INDEX], "ov5647");
	if (rc < 0)
		pr_err("%s: gpio_request GPIO_SKU3_CAM_5MP_SHDN_N failed!",
			 __func__);

	pr_debug("gpio_tlmm_config %d\r\n", gpio_num[GPIO_CAM_BACK_PWDN_INDEX]);
	rc = gpio_tlmm_config(gpio_cfg[GPIO_CAM_BACK_PWDN_INDEX], GPIO_CFG_ENABLE);
	if (rc < 0) {
		pr_err("%s:unable to enable Powr Dwn gpio for main camera!\n",
			 __func__);
		gpio_free(gpio_num[GPIO_CAM_BACK_PWDN_INDEX]);
	}

	gpio_direction_output(gpio_num[GPIO_CAM_BACK_PWDN_INDEX], 1);

	rc = gpio_request(gpio_num[GPIO_CAM_BACK_RESET_INDEX], "ov5647");
	if (rc < 0)
		pr_err("%s: gpio_request GPIO_SKU3_CAM_5MP_CAMIF_RESET failed!",
			 __func__);

	pr_debug("gpio_tlmm_config %d\r\n", gpio_num[GPIO_CAM_BACK_RESET_INDEX]);
	rc = gpio_tlmm_config(gpio_cfg[GPIO_CAM_BACK_RESET_INDEX], GPIO_CFG_ENABLE);
	if (rc < 0) {
		pr_err("%s: unable to enable reset gpio for main camera!\n",
			 __func__);
		gpio_free(gpio_num[GPIO_CAM_BACK_RESET_INDEX]);
	}

	gpio_direction_output(gpio_num[GPIO_CAM_BACK_RESET_INDEX], 1);

	rc = gpio_request(gpio_num[GPIO_CAM_FRONT_PWDN_INDEX], "ov7692");
	if (rc < 0)
		pr_err("%s: gpio_request---GPIO_SKU1_CAM_VGA_SHDN failed!",
			 __func__);

	rc = gpio_tlmm_config(gpio_cfg[GPIO_CAM_FRONT_PWDN_INDEX], GPIO_CFG_ENABLE);
	if (rc < 0) {
		pr_err("%s:unable to enable Powr Dwn gpio for frnt camera!\n",
			 __func__);
		gpio_free(gpio_num[GPIO_CAM_FRONT_PWDN_INDEX]);
	}

	gpio_direction_output(gpio_num[GPIO_CAM_FRONT_PWDN_INDEX], 1);

	rc = gpio_request(gpio_num[GPIO_CAM_FRONT_RESET_INDEX] , "ov7692");
	if (rc < 0)
		pr_err("%s: gpio_request---GPIO_SKU1_CAM_VGA_RESET_N failed!",
			 __func__);

	rc = gpio_tlmm_config(gpio_cfg[GPIO_CAM_FRONT_RESET_INDEX] , GPIO_CFG_ENABLE);

	if (rc < 0) {
		pr_err("%s: unable to enable reset gpio for front camera!\n",
			 __func__);
		gpio_free(gpio_num[GPIO_CAM_FRONT_RESET_INDEX] );
	}
	gpio_direction_output(gpio_num[GPIO_CAM_FRONT_RESET_INDEX] , 1);


	rc = gpio_request(gpio_num[GPIO_CAM_ID_INDEX], "cam_id");
	if (rc < 0)
		pr_err("%s: gpio_request---CAM_ID failed!",
			 __func__);

	rc = gpio_tlmm_config(gpio_cfg[GPIO_CAM_ID_INDEX], GPIO_CFG_ENABLE);

	if (rc < 0) {
		pr_err("%s: unable to enable reset gpio for front camera!\n",
			 __func__);
		gpio_free(GPIO_SKU1_CAM_VGA_RESET_N);
	}

}

static void qrd7_camera_gpio_cfg(void)
{
    int i = 0, rc = 0;
	
    for(i = 0; i < sizeof(camera_gpio_rear_qrd7)/sizeof(int); i++)
    {
        rc = gpio_request(camera_gpio_rear_qrd7[i], "ov5647");
        if (rc < 0)
            pr_err("%s: gpio_request %d failed!\n", __func__, camera_gpio_rear_qrd7[i]);
        pr_debug("gpio_tlmm_config %d\n", camera_gpio_rear_qrd7[i]);
        rc = gpio_tlmm_config(GPIO_CFG(camera_gpio_rear_qrd7[i], 0,
                                GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL,
                                GPIO_CFG_2MA), GPIO_CFG_ENABLE);
        if (rc < 0) {
            pr_err("%s:unable to enable Powr Dwn gpio for main camera!\n", __func__);
            gpio_free(camera_gpio_rear_qrd7[i]);
            continue;
        }
        gpio_direction_output(camera_gpio_rear_qrd7[i], 1);
    }
	
    for(i = 0; i < sizeof(camera_gpio_front_qrd7)/sizeof(int); i++)
    {
        rc = gpio_request(camera_gpio_front_qrd7[i], "ov7692");
        if (rc < 0)
            pr_err("%s: gpio_request %d failed!\n", __func__, camera_gpio_front_qrd7[i]);
        pr_debug("gpio_tlmm_config %d\n", camera_gpio_front_qrd7[i]);
        rc = gpio_tlmm_config(GPIO_CFG(camera_gpio_front_qrd7[i], 0,
                                GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL,
                                GPIO_CFG_2MA), GPIO_CFG_ENABLE);
        if (rc < 0) {
            pr_err("%s:unable to enable Powr Dwn gpio for main camera!\n", __func__);
            gpio_free(camera_gpio_front_qrd7[i]);
            continue;
        }
        gpio_direction_output(camera_gpio_front_qrd7[i], 1);
    }
}

#ifndef CONFIG_MSM_CAMERA_V4L2

static void msm_camera_vreg_config(int vreg_en)
{
	int rc = vreg_en ?
		regulator_bulk_enable(ARRAY_SIZE(regs_camera), regs_camera) :
		regulator_bulk_disable(ARRAY_SIZE(regs_camera), regs_camera);

	if (rc)
		pr_err("%s: could not %sable regulators: %d\n",
				__func__, vreg_en ? "en" : "dis", rc);
}

static int config_gpio_table(uint32_t *table, int len)
{
	int rc = 0, i = 0;

	for (i = 0; i < len; i++) {
		rc = gpio_tlmm_config(table[i], GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("%s not able to get gpio\n", __func__);
			for (i--; i >= 0; i--)
				gpio_tlmm_config(camera_off_gpio_table[i],
							GPIO_CFG_ENABLE);
			break;
		}
	}
	return rc;
}

static int config_camera_on_gpios_rear(void)
{
	int rc = 0;

	if (machine_is_msm7x27a_ffa() || machine_is_msm7625a_ffa()
				|| machine_is_msm7627a_qrd1()
				|| machine_is_msm8625_skua()
				|| machine_is_msm8625_ffa())
		msm_camera_vreg_config(1);

	rc = config_gpio_table(camera_on_gpio_table,
			ARRAY_SIZE(camera_on_gpio_table));
	if (rc < 0) {
		pr_err("%s: CAMSENSOR gpio table request"
		"failed\n", __func__);
		return rc;
	}

	return rc;
}

static void config_camera_off_gpios_rear(void)
{
	if (machine_is_msm7x27a_ffa() || machine_is_msm7625a_ffa()
				|| machine_is_msm7627a_qrd1()
				|| machine_is_msm8625_skua()
				|| machine_is_msm8625_ffa())
		msm_camera_vreg_config(0);

	config_gpio_table(camera_off_gpio_table,
			ARRAY_SIZE(camera_off_gpio_table));
}

static int config_camera_on_gpios_front(void)
{
	int rc = 0;

	if (machine_is_msm7x27a_ffa() || machine_is_msm7625a_ffa()
				|| machine_is_msm7627a_qrd1()
				|| machine_is_msm8625_skua()
				|| machine_is_msm8625_ffa())
		msm_camera_vreg_config(1);

	rc = config_gpio_table(camera_on_gpio_table,
			ARRAY_SIZE(camera_on_gpio_table));
	if (rc < 0) {
		pr_err("%s: CAMSENSOR gpio table request"
			"failed\n", __func__);
		return rc;
	}

	return rc;
}

static void config_camera_off_gpios_front(void)
{
	if (machine_is_msm7x27a_ffa() || machine_is_msm7625a_ffa()
				|| machine_is_msm7627a_qrd1()
				|| machine_is_msm8625_skua()
				|| machine_is_msm8625_ffa())
		msm_camera_vreg_config(0);

	config_gpio_table(camera_off_gpio_table,
			ARRAY_SIZE(camera_off_gpio_table));
}

struct msm_camera_device_platform_data msm_camera_device_data_rear = {
	.camera_gpio_on		= config_camera_on_gpios_rear,
	.camera_gpio_off	= config_camera_off_gpios_rear,
	.ioext.csiphy		= 0xA1000000,
	.ioext.csisz		= 0x00100000,
	.ioext.csiirq		= INT_CSI_IRQ_1,
	.ioclk.mclk_clk_rate	= 24000000,
	.ioclk.vfe_clk_rate	= 192000000,
	.ioext.appphy		= MSM7XXX_CLK_CTL_PHYS,
	.ioext.appsz		= MSM7XXX_CLK_CTL_SIZE,
};

struct msm_camera_device_platform_data msm_camera_device_data_front = {
	.camera_gpio_on		= config_camera_on_gpios_front,
	.camera_gpio_off	= config_camera_off_gpios_front,
	.ioext.csiphy		= 0xA0F00000,
	.ioext.csisz		= 0x00100000,
	.ioext.csiirq		= INT_CSI_IRQ_0,
	.ioclk.mclk_clk_rate	= 24000000,
	.ioclk.vfe_clk_rate	= 192000000,
	.ioext.appphy		= MSM7XXX_CLK_CTL_PHYS,
	.ioext.appsz		= MSM7XXX_CLK_CTL_SIZE,
};

#ifdef CONFIG_OV5647

static struct msm_camera_sensor_platform_info ov5647_sensor_7627a_info = {
	.mount_angle = 270
//	.mount_angle = 270
};

static struct msm_camera_sensor_flash_src msm_flash_src_ov5647 = {
	.flash_sr_type = MSM_CAMERA_FLASH_SRC_LED,
	._fsrc.led_src.led_name = "flashlight",
	._fsrc.led_src.led_name_len = 10,
};

static struct msm_camera_sensor_flash_data flash_ov5647 = {
	.flash_type             = MSM_CAMERA_FLASH_LED,
	.flash_src              = &msm_flash_src_ov5647,
};

static struct msm_camera_sensor_info msm_camera_sensor_ov5647_data = {
	.sensor_name    = "ov5647",
	.sensor_reset_enable = 1,
	.sensor_reset   = GPIO_SKU3_CAM_5MP_CAMIF_RESET,
	.pmic_gpio_enable  = 1,
	.sensor_pwd     = GPIO_SKU3_CAM_5MP_SHDN_N,
	.vcm_pwd        = GPIO_SKU3_CAM_5MP_CAM_DRIVER_PWDN,
	.vcm_enable     = 1,
	.pdata          = &msm_camera_device_data_rear,
	.flash_data     = &flash_ov5647,
	.sensor_platform_info   = &ov5647_sensor_7627a_info,
	.csi_if                 = 1
};

static struct platform_device msm_camera_sensor_ov5647 = {
	.name      = "msm_camera_ov5647",
	.dev       = {
		.platform_data = &msm_camera_sensor_ov5647_data,
	},
};
#endif

/*renwei add it for ov2655 camera at 2012-8-1*/
#ifdef CONFIG_OV2655
static struct msm_camera_sensor_platform_info ov2655_sensor_7627a_info = {
	.mount_angle = 90
};

static struct msm_camera_sensor_flash_src msm_flash_src_ov2655 = {
	.flash_sr_type = MSM_CAMERA_FLASH_SRC_LED,
	._fsrc.led_src.led_name = "flashlight",
	._fsrc.led_src.led_name_len = 10,
};

static struct msm_camera_sensor_flash_data flash_ov2655 = {
	.flash_type             = MSM_CAMERA_FLASH_LED,
	.flash_src              = &msm_flash_src_ov2655,
};

static struct msm_camera_sensor_info msm_camera_sensor_ov2655_data = {
	.sensor_name    = "ov2655",
	.sensor_reset_enable = 1,
	.sensor_reset   = GPIO_SKU3_CAM_5MP_CAMIF_RESET,
	.pmic_gpio_enable  = 1,
	.sensor_pwd     = GPIO_SKU3_CAM_5MP_SHDN_N,
	.vcm_pwd        = GPIO_SKU3_CAM_5MP_CAM_DRIVER_PWDN,
	.vcm_enable     = 1,
	.pdata          = &msm_camera_device_data_rear,
	.flash_data     = &flash_ov2655,
	.sensor_platform_info   = &ov2655_sensor_7627a_info,
	.csi_if                 = 1
};

static struct platform_device msm_camera_sensor_ov2655 = {
	.name      = "msm_camera_ov2655",
	.dev       = {
		.platform_data = &msm_camera_sensor_ov2655_data,
	},
};
#endif
/*add end*/
#ifdef CONFIG_S5K4E1
static struct msm_camera_sensor_platform_info s5k4e1_sensor_7627a_info = {
	.mount_angle = 90
};

static struct msm_camera_sensor_flash_data flash_s5k4e1 = {
	.flash_type	     = MSM_CAMERA_FLASH_LED,
	.flash_src	      = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_s5k4e1_data = {
	.sensor_name		= "s5k4e1",
	.sensor_reset_enable	= 1,
	.sensor_reset		= GPIO_CAM_GP_CAMIF_RESET_N,
	.pmic_gpio_enable       = 0,
	.sensor_pwd	     = 85,
	.vcm_pwd		= GPIO_CAM_GP_CAM_PWDN,
	.vcm_enable	     = 1,
	.pdata			= &msm_camera_device_data_rear,
	.flash_data	     = &flash_s5k4e1,
	.sensor_platform_info   = &s5k4e1_sensor_7627a_info,
	.csi_if			= 1
};

static struct platform_device msm_camera_sensor_s5k4e1 = {
	.name   = "msm_camera_s5k4e1",
	.dev    = {
		.platform_data = &msm_camera_sensor_s5k4e1_data,
	},
};
#endif

#ifdef CONFIG_IMX072
static struct msm_camera_sensor_platform_info imx072_sensor_7627a_info = {
	.mount_angle = 90
};

static struct msm_camera_sensor_flash_data flash_imx072 = {
	.flash_type	     = MSM_CAMERA_FLASH_LED,
	.flash_src	      = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_imx072_data = {
	.sensor_name		= "imx072",
	.sensor_reset_enable	= 1,
	.sensor_reset		= GPIO_CAM_GP_CAMIF_RESET_N, /* TODO 106,*/
	.pmic_gpio_enable       = 0,
	.sensor_pwd	     = 85,
	.vcm_pwd		= GPIO_CAM_GP_CAM_PWDN,
	.vcm_enable	     = 1,
	.pdata			= &msm_camera_device_data_rear,
	.flash_data	     = &flash_imx072,
	.sensor_platform_info	= &imx072_sensor_7627a_info,
	.csi_if			= 1
};

static struct platform_device msm_camera_sensor_imx072 = {
	.name   = "msm_camera_imx072",
	.dev    = {
		.platform_data = &msm_camera_sensor_imx072_data,
	},
};
#endif

#ifdef CONFIG_WEBCAM_OV9726
static struct msm_camera_sensor_info msm_camera_sensor_ov9726_data;
static struct msm_camera_sensor_platform_info ov9726_sensor_7627a_info = {
	.mount_angle = 90
};

static struct msm_camera_sensor_flash_data flash_ov9726 = {
	.flash_type	     = MSM_CAMERA_FLASH_NONE,
	.flash_src	      = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_ov9726_data = {
	.sensor_name		= "ov9726",
	.sensor_reset_enable	= 0,
	.sensor_reset		= GPIO_CAM_GP_CAM1MP_XCLR,
	.pmic_gpio_enable       = 0,
	.sensor_pwd	     = 85,
	.vcm_pwd		= 1,
	.vcm_enable	     = 0,
	.pdata			= &msm_camera_device_data_front,
	.flash_data	     = &flash_ov9726,
	.sensor_platform_info   = &ov9726_sensor_7627a_info,
	.csi_if			= 1
};

static struct platform_device msm_camera_sensor_ov9726 = {
	.name   = "msm_camera_ov9726",
	.dev    = {
		.platform_data = &msm_camera_sensor_ov9726_data,
	},
};
#else
static inline void msm_camera_vreg_init(void) { }
#endif

#ifdef CONFIG_MT9E013
static struct msm_camera_sensor_platform_info mt9e013_sensor_7627a_info = {
	.mount_angle = 90
};

static struct msm_camera_sensor_flash_data flash_mt9e013 = {
	.flash_type = MSM_CAMERA_FLASH_LED,
	.flash_src  = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9e013_data = {
	.sensor_name		= "mt9e013",
	.sensor_reset		= 0,
	.sensor_reset_enable	= 1,
	.pmic_gpio_enable       = 0,
	.sensor_pwd		= 85,
	.vcm_pwd		= 1,
	.vcm_enable		= 0,
	.pdata		= &msm_camera_device_data_rear,
	.flash_data		= &flash_mt9e013,
	.sensor_platform_info   = &mt9e013_sensor_7627a_info,
	.csi_if		= 1
};

static struct platform_device msm_camera_sensor_mt9e013 = {
	.name      = "msm_camera_mt9e013",
	.dev       = {
		.platform_data = &msm_camera_sensor_mt9e013_data,
	},
};
#endif

#ifdef CONFIG_OV5640
static struct msm_camera_sensor_platform_info ov5640_sensor_info = {
	.mount_angle    = 90
};

static struct msm_camera_sensor_flash_src msm_flash_src_ov5640 = {
	.flash_sr_type = MSM_CAMERA_FLASH_SRC_LED,
	._fsrc.led_src.led_name = "flashlight",
	._fsrc.led_src.led_name_len = 10,
};

static struct msm_camera_sensor_flash_data flash_ov5640 = {
	.flash_type     = MSM_CAMERA_FLASH_LED,
	.flash_src      = &msm_flash_src_ov5640,
};

static struct msm_camera_sensor_info msm_camera_sensor_ov5640_data = {
	.sensor_name	    = "ov5640",
	.sensor_reset_enable    = 1,
	.pmic_gpio_enable  = 0,
	.sensor_reset	   = QRD_GPIO_CAM_5MP_RESET,
	.sensor_pwd	     = QRD_GPIO_CAM_5MP_SHDN_EN,
	.vcm_pwd		= 0,
	.vcm_enable	     = 0,
	.pdata			= &msm_camera_device_data_rear,
	.flash_data	     = &flash_ov5640,
	.sensor_platform_info   = &ov5640_sensor_info,
	.csi_if		 = 1,
};

static struct platform_device msm_camera_sensor_ov5640 = {
	.name   = "msm_camera_ov5640",
	.dev    = {
		.platform_data = &msm_camera_sensor_ov5640_data,
	},
};
#endif

#ifdef CONFIG_MT9V113
static struct msm_camera_sensor_platform_info mt9v113_sensor_7627a_info = {
	.mount_angle = 90
};

static struct msm_camera_sensor_flash_data flash_mt9v113 = {
    .flash_type             = MSM_CAMERA_FLASH_NONE,
	//.flash_src              = &msm_flash_src
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9v113_data = {
	.sensor_name            = "mt9v113",
	.sensor_reset_enable    = 1,
	.sensor_reset			= QRD_SKUA_GPIO_CAM_3MP_RESET,
	                        //config this gpio pin as your hardware design
    .sensor_pwd             = QRD_SKUA_GPIO_CAM_3MP_PWDN,
	                        //config this gpio pin as your hardware design
	.vcm_pwd                = 0,
	.vcm_enable             = 0,
	.pdata                  = &msm_camera_device_data_front,
//	.pdata                  = &msm_camera_data_front,  //should be front
	.flash_data             = &flash_mt9v113,
	.sensor_platform_info   = &mt9v113_sensor_7627a_info,
	.csi_if                 = 1
};

static struct platform_device msm_camera_sensor_mt9v113 = {
	.name   = "msm_camera_mt9v113",
	.dev    = {
		.platform_data = &msm_camera_sensor_mt9v113_data,
	},
};
#endif


#ifdef CONFIG_WEBCAM_OV7692_QRD
static struct msm_camera_sensor_platform_info ov7692_sensor_7627a_info = {
	.mount_angle = 90
};

static struct msm_camera_sensor_flash_data flash_ov7692 = {
	.flash_type     = MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_sensor_info msm_camera_sensor_ov7692_data = {
	.sensor_name	    = "ov7692",
	.sensor_reset_enable    = 0,
	.pmic_gpio_enable  = 1,
	.sensor_reset	   = GPIO_SKU1_CAM_VGA_RESET_N,
	.sensor_pwd	     = GPIO_SKU1_CAM_VGA_SHDN,
	.vcm_pwd		= 0,
	.vcm_enable	     = 0,
	.pdata			= &msm_camera_device_data_front,
	.flash_data	     = &flash_ov7692,
	.sensor_platform_info   = &ov7692_sensor_7627a_info,
	.csi_if		 = 1,
};

static struct platform_device msm_camera_sensor_ov7692 = {
	.name   = "msm_camera_ov7692",
	.dev    = {
		.platform_data = &msm_camera_sensor_ov7692_data,
	},
};
#endif

static struct i2c_board_info i2c_camera_devices[] = {
	#ifdef CONFIG_S5K4E1
	{
		I2C_BOARD_INFO("s5k4e1", 0x36),
	},
	{
		I2C_BOARD_INFO("s5k4e1_af", 0x8c >> 1),
	},
	#endif
	#ifdef CONFIG_WEBCAM_OV9726
	{
		I2C_BOARD_INFO("ov9726", 0x10),
	},
	#endif
	#ifdef CONFIG_IMX072
	{
		I2C_BOARD_INFO("imx072", 0x34),
	},
	#endif
	#ifdef CONFIG_MT9E013
	{
		I2C_BOARD_INFO("mt9e013", 0x6C >> 2),
	},
	#endif
	{
		I2C_BOARD_INFO("sc628a", 0x6E),
	},
};

static struct i2c_board_info i2c_camera_devices_qrd[] = {
	#ifdef CONFIG_OV5640
	{
		I2C_BOARD_INFO("ov5640", 0x78 >> 1),
	},
	#endif
	#ifdef CONFIG_WEBCAM_OV7692_QRD
	{
		I2C_BOARD_INFO("ov7692", 0x78),
	},
	#endif
};

static struct i2c_board_info i2c_camera_devices_skua[] = {
    #ifdef CONFIG_OV5647
	{
		I2C_BOARD_INFO("ov5647", 0x36 << 1),
	},
	{
		I2C_BOARD_INFO("ov5647_af", 0x18 >> 1),
	},
    #endif
    #ifdef CONFIG_OV5640
	{
		I2C_BOARD_INFO("ov5640", 0x78 >> 1),
	},
    #endif
/*renwei add it for ov2566 camera at 2012-8-1*/
    #ifdef CONFIG_OV2655
	{
		I2C_BOARD_INFO("ov2655", (0x60)), //0x30
	},
    #endif
/*add end*/
    #ifdef CONFIG_MT9V113
	{
		I2C_BOARD_INFO("mt9v113", (0x7a >> 1)), //0x45
	},
    #endif
};
static struct i2c_board_info i2c_camera_devices_evb[] = {
	#ifdef CONFIG_OV5647
	{
		I2C_BOARD_INFO("ov5647", 0x36 << 1),
	},
	{
		I2C_BOARD_INFO("ov5647_af", 0x18 >> 1),
	},
	#endif
/*renwei add it for ov2566 camera at 2012-8-1*/
	#ifdef CONFIG_OV2655
	{
		I2C_BOARD_INFO("ov2655", (0x60)), //0x30
	},
	#endif
/*add end*/
	#ifdef CONFIG_WEBCAM_OV7692_QRD
	{
		I2C_BOARD_INFO("ov7692", 0x78),
	},
	#endif
};

static struct platform_device *camera_devices_msm[] __initdata = {
#ifdef CONFIG_S5K4E1
	&msm_camera_sensor_s5k4e1,
#endif
#ifdef CONFIG_IMX072
	&msm_camera_sensor_imx072,
#endif
#ifdef CONFIG_WEBCAM_OV9726
	&msm_camera_sensor_ov9726,
#endif
#ifdef CONFIG_MT9E013
	&msm_camera_sensor_mt9e013,
#endif
};

static struct platform_device *camera_devices_qrd[] __initdata = {
#ifdef CONFIG_OV5640
	&msm_camera_sensor_ov5640,
#endif
#ifdef CONFIG_WEBCAM_OV7692_QRD
	&msm_camera_sensor_ov7692,
#endif
};

static struct platform_device *camera_devices_skua[] __initdata = {
#ifdef CONFIG_OV5647
	&msm_camera_sensor_ov5647,
#endif
/*renwei add it for ov2566 camera at 2012-8-1*/
#ifdef CONFIG_OV2655
	&msm_camera_sensor_ov2655,
#endif
/*add end*/
#ifdef CONFIG_OV5640
	&msm_camera_sensor_ov5640,
#endif
#ifdef CONFIG_MT9V113
	&msm_camera_sensor_mt9v113,
#endif
};

static struct platform_device *camera_devices_evb[] __initdata = {
#ifdef CONFIG_OV5647
	&msm_camera_sensor_ov5647,
#endif
/*renwei add it for ov2566 camera at 2012-8-1*/
#ifdef CONFIG_OV2655
	&msm_camera_sensor_ov2655,
#endif
/*add end*/
#ifdef CONFIG_WEBCAM_OV7692_QRD
	&msm_camera_sensor_ov7692,
#endif
/*renwei add it for the main camera at 2012-5-31*/
#ifdef CONFIG_OV8825
	&msm_camera_sensor_ov8825,
#endif
};
#endif

enum {
	SX150X_CAM,
};

static struct sx150x_platform_data sx150x_data[] __initdata = {
	[SX150X_CAM]    = {
		.gpio_base	      = GPIO_CAM_EXPANDER_BASE,
		.oscio_is_gpo	   = false,
		.io_pullup_ena	  = 0,
		.io_pulldn_ena	  = 0,
		.io_open_drain_ena      = 0x23,
		.irq_summary	    = -1,
	},
};

static struct i2c_board_info cam_exp_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO("sx1508q", 0x22),
		.platform_data  = &sx150x_data[SX150X_CAM],
	},
};

static void __init register_i2c_devices(void)
{
	i2c_register_board_info(MSM_GSBI0_QUP_I2C_BUS_ID,
				cam_exp_i2c_info,
				ARRAY_SIZE(cam_exp_i2c_info));
}

/* TODO: sensor specific? */
static struct regulator *reg_ext_2v8;
/*renwei add it for the main camera AF at 2012-7-21*/
static struct vreg *lcd_camera_vreg = NULL;

//static struct regulator *reg_ext_1v8;

int lcd_camera_power_onoff(int on)
{
	int rc = 0;

	printk("%s: on = %d\n", __func__, on);

	/* ext_2v8 and ext_1v8 control */
	if (!reg_ext_2v8) {
		reg_ext_2v8 = regulator_get(NULL, "ext_2v8");
		if (IS_ERR(reg_ext_2v8)) {
			pr_err("'%s' regulator not found, rc=%ld\n",
				"ext_2v8", IS_ERR(reg_ext_2v8));
			reg_ext_2v8 = NULL;
			return -ENODEV;
		}
	}

#if 0
	if (!reg_ext_1v8) {
		reg_ext_1v8 = regulator_get(NULL, "ext_1v8");
		if (IS_ERR(reg_ext_1v8)) {
			pr_err("'%s' regulator not found, rc=%ld\n",
				"ext_1v8", IS_ERR(reg_ext_1v8));
			reg_ext_1v8 = NULL;
			return -ENODEV;
		}
	}
#endif
	if (on) {
		rc = regulator_enable(reg_ext_2v8);
		if (rc) {
			pr_err("'%s' regulator enable failed, rc=%d\n",
				"reg_ext_2v8", rc);
			return rc;
		}

		/*renwei add it for Q802 main camera AF at 2012-7-11*/
		rc = vreg_enable(lcd_camera_vreg);
  		if (rc) {
	 		 pr_err("vreg_enable: lcd_camera_vreg vreg failed\n");
 		}
		/*add end*/
#if 0
		rc = regulator_enable(reg_ext_1v8);
		if (rc) {
			pr_err("'%s' regulator enable failed, rc=%d\n",
				"reg_ext_1v8", rc);
			return rc;
		}
#endif
		pr_debug("%s(on): success\n", __func__);
	} else {
		//rc = regulator_disable(reg_ext_2v8);
		if (rc)
			pr_warning("'%s' regulator disable failed, rc=%d\n",
				"reg_ext_2v8", rc);
#if 0
		rc = regulator_disable(reg_ext_1v8);
		if (rc)
			pr_warning("'%s' regulator disable failed, rc=%d\n",
				"reg_ext_1v8", rc);
#endif
		pr_debug("%s(off): success\n", __func__);
	}

	return rc;
}
EXPORT_SYMBOL(lcd_camera_power_onoff);

void camera_af_software_powerdown(struct i2c_client *client)
{
	int rc = 0;
	unsigned char txdata[] = {0x80, 0x0};
	struct i2c_msg msg[] = {
		{
			.addr = 0x18>>1,
			.flags = 0,
			.len = 2,
			.buf = txdata,
		 },
	};
	printk("camera_af_software_powerdown IN...\n");
	/* send software powerdown cmd to AF motor, avoid current leak */
	if (machine_is_msm8625_skua()){
		printk("SKUA AF Motor software powerdown, write i2c saddr:0x18, waddr:0x80, wdata:0x00\n");
		rc = i2c_transfer(client->adapter, msg, 1);
		if (rc < 0)
			printk("AF software powerdown faild\n");
		return;
	}
}
EXPORT_SYMBOL(camera_af_software_powerdown);

void __init msm7627a_camera_init(void)
{
#ifndef CONFIG_MSM_CAMERA_V4L2
	int rc;
#endif

/*renwei add it for Q802 main camera AF at 2012-7-21*/
	if (machine_is_msm8625_qrd5()){
		int rc = 0;
		lcd_camera_vreg = vreg_get(NULL, "rfrx1");
   		 rc = vreg_set_level(lcd_camera_vreg, 2850);
    		if (rc < 0) {
      			pr_err("%s: set regulator level failed "
        			"with :(%d)\n", __func__, rc);
		}
	}
/*add end */

	pr_debug("########msm7627a_camera_init Entered\n");
	/* LCD and camera power (VREG & LDO) init */
	if (machine_is_msm7627a_evb() || machine_is_msm8625_evb() || 
		machine_is_msm8625_qrd5()|| machine_is_msm7x27a_qrd5a() || 
		machine_is_msm8625_evt()) {
//		lcd_camera_power_init();
		evb_camera_gpio_cfg();
	}else if (machine_is_msm8625_qrd7()) {
		qrd7_camera_gpio_cfg();
	}else if(machine_is_msm8625_skua()){
		skua_camera_gpio_cfg();
	}

#ifndef CONFIG_MSM_CAMERA_V4L2
	if (machine_is_msm7627a_qrd1()) {
		qrd1_camera_gpio_cfg();
		platform_add_devices(camera_devices_qrd,
				ARRAY_SIZE(camera_devices_qrd));
	} else if (machine_is_msm7627a_evb() || machine_is_msm8625_evb() || 
		machine_is_msm8625_qrd5()|| machine_is_msm7x27a_qrd5a() || 
		machine_is_msm8625_evt()) {
		evb_camera_gpio_cfg();
		platform_add_devices(camera_devices_evb,
				ARRAY_SIZE(camera_devices_evb));
	} else if(machine_is_msm8625_skua())
    {
#ifdef CONFIG_OV5647
		//Add SKUA specific settings, ov5647/Mp9v113 GPIOs
		msm_camera_sensor_ov5647_data.sensor_reset = QRD_SKUA_GPIO_CAM_5MP_RESET;
		msm_camera_sensor_ov5647_data.sensor_pwd = QRD_SKUA_GPIO_CAM_5MP_SHDN_EN;
		msm_camera_sensor_ov5647_data.vcm_pwd = 0;
		msm_camera_sensor_ov5647_data.vcm_enable = 0;
		ov5647_sensor_7627a_info.mount_angle = 90;
#endif
/*renwei add it for ov2655 camera at 2012-8-1*/
#ifdef CONFIG_OV2655
		msm_camera_sensor_ov2655_data.sensor_reset = QRD_SKUA_GPIO_CAM_5MP_RESET;
		msm_camera_sensor_ov2655_data.sensor_pwd = QRD_SKUA_GPIO_CAM_5MP_SHDN_EN;
		msm_camera_sensor_ov2655_data.vcm_pwd = 0;
		msm_camera_sensor_ov2655_data.vcm_enable = 0;
		ov5647_sensor_7627a_info.mount_angle = 270;
#endif
/*add end*/
#ifdef CONFIG_OV5640
		msm_camera_sensor_ov5640_data.sensor_reset = QRD_SKUA_GPIO_CAM_5MP_RESET;
		msm_camera_sensor_ov5640_data.sensor_pwd = QRD_SKUA_GPIO_CAM_5MP_SHDN_EN;
		msm_camera_sensor_ov5640_data.vcm_pwd = 0;
		msm_camera_sensor_ov5640_data.vcm_enable = 0;
		ov5640_sensor_info.mount_angle = 270;
#endif
#ifdef CONFIG_MT9V113
		msm_camera_sensor_mt9v113_data.sensor_reset = QRD_SKUA_GPIO_CAM_3MP_RESET;
		msm_camera_sensor_mt9v113_data.sensor_pwd = QRD_SKUA_GPIO_CAM_3MP_PWDN;
#endif
		//Add 8x25 specific settings, CSI INT Number
		msm_camera_device_data_rear.ioext.csiirq = MSM8625_INT_CSI_IRQ_1;
		msm_camera_device_data_front.ioext.csiirq = MSM8625_INT_CSI_IRQ_0;

		skua_camera_gpio_cfg();
		platform_add_devices(camera_devices_skua,
				ARRAY_SIZE(camera_devices_skua));
    }
    else if (machine_is_msm8625_qrd7())
    {
#ifdef CONFIG_OV5647
        //Add SKUA specific settings, ov5647/Mp9v113 GPIOs
        msm_camera_sensor_ov5647_data.sensor_reset = camera_gpio_rear_qrd7[1];
        msm_camera_sensor_ov5647_data.sensor_pwd = camera_gpio_rear_qrd7[0];
        msm_camera_sensor_ov5647_data.vcm_pwd = 0;
        msm_camera_sensor_ov5647_data.vcm_enable = 0;
#endif
/*renwei add it for ov2655 camera at 2012-8-1*/
#ifdef CONFIG_OV2655
        msm_camera_sensor_ov2655_data.sensor_reset = camera_gpio_rear_qrd7[1];
        msm_camera_sensor_ov2655_data.sensor_pwd = camera_gpio_rear_qrd7[0];
        msm_camera_sensor_ov2655_data.vcm_pwd = 0;
        msm_camera_sensor_ov2655_data.vcm_enable = 0;
#endif
/*add end*/
#ifdef CONFIG_WEBCAM_OV7692_QRD
        msm_camera_sensor_ov7692_data.sensor_reset = camera_gpio_front_qrd7[1];
        msm_camera_sensor_ov7692_data.sensor_pwd = camera_gpio_front_qrd7[0];
#endif
        //Add 8x25 specific settings, CSI INT Number
        msm_camera_device_data_rear.ioext.csiirq = MSM8625_INT_CSI_IRQ_1;
        msm_camera_device_data_front.ioext.csiirq = MSM8625_INT_CSI_IRQ_0;

        platform_add_devices(camera_devices_evb,
                ARRAY_SIZE(camera_devices_evb));
    }
    else if (machine_is_msm7627a_qrd3())
		return;
	else
		platform_add_devices(camera_devices_msm,
				ARRAY_SIZE(camera_devices_msm));
#endif
	if (!machine_is_msm7627a_qrd1() && !machine_is_msm7627a_evb() && 
		!machine_is_msm8625_evb() && !machine_is_msm8625_qrd5() && 
		!machine_is_msm7x27a_qrd5a() && !machine_is_msm8625_skua() && 
		!machine_is_msm8625_qrd7() &&!machine_is_msm8625_evt())
		register_i2c_devices();
#ifndef CONFIG_MSM_CAMERA_V4L2
	rc = regulator_bulk_get(NULL, ARRAY_SIZE(regs_camera), regs_camera);

	if (rc) {
		pr_err("%s: could not get regulators: %d\n", __func__, rc);
		return;
	}

	rc = regulator_bulk_set_voltage(ARRAY_SIZE(regs_camera), regs_camera);

	if (rc) {
		pr_err("%s: could not set voltages: %d\n", __func__, rc);
		return;
	}
#endif

#if defined(CONFIG_MSM_CAMERA_V4L2)
	msm7x27a_init_cam();
#endif

#ifndef CONFIG_MSM_CAMERA_V4L2
	if (machine_is_msm7627a_qrd1()) {
		i2c_register_board_info(MSM_GSBI0_QUP_I2C_BUS_ID,
				i2c_camera_devices_qrd,
				ARRAY_SIZE(i2c_camera_devices_qrd));
    } else if (machine_is_msm8625_skua()){
		i2c_register_board_info(MSM_GSBI0_QUP_I2C_BUS_ID,
				i2c_camera_devices_skua,
				ARRAY_SIZE(i2c_camera_devices_skua));
	} else if (machine_is_msm7627a_evb() || machine_is_msm8625_evb() ||
		machine_is_msm8625_qrd5() || machine_is_msm7x27a_qrd5a() || 
		machine_is_msm8625_evt()) {
		pr_debug("machine_is_msm7627a_evb i2c_register_board_info\n");
		i2c_register_board_info(MSM_GSBI0_QUP_I2C_BUS_ID,
				i2c_camera_devices_evb,
				ARRAY_SIZE(i2c_camera_devices_evb));
	}
#else
	if(machine_is_msm8625_qrd7() || machine_is_msm8625_evb() || machine_is_msm8625_qrd5())
	{
		printk("i2c_register_board_info\n");

		i2c_register_board_info(MSM_GSBI0_QUP_I2C_BUS_ID,
				i2c_camera_devices,
				ARRAY_SIZE(i2c_camera_devices));
	}else if(machine_is_msm8625_skua())
	{
		printk("skua i2c_register_board_info\n");
		i2c_register_board_info(MSM_GSBI0_QUP_I2C_BUS_ID,
				i2c_camera_devices_skua,
				ARRAY_SIZE(i2c_camera_devices_skua));
	}
#endif
}
