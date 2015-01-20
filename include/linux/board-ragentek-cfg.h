#ifndef BOARD_RAGENTEK_CFG_H
#define BOARD_RAGENTEK_CFG_H
#include <linux/module.h>
#include <asm/gpio.h>

#define INVALID_GPIO ARCH_NR_GPIOS
/*=====================Enum Const define======================*/
enum lcd_type{
	RGB_TRULY_LCD_HX8369 = 0,
	RGB_DJN_LCD_HX8369 = 1,
        MIPI_ORISE_LCD_OTM8009A = 2,
	MIPI_QHD_LCD_OTM9608A = 3,
	MIPI_QHD_LCD_DNJ_OTM9608A = 4,
        RGB_DNJ_LCD_OTM8018B = 5,
        MIPI_TCL_LCD_NT35510 = 6,
  	RGB_AZET_LCD_NT35510 = 7,
	MIPI_QHD_LCD_TRULY_OTM9608A = 8,
	RGB_DNJ_LCD_OTM8018B_IPS = 9,
	MIPI_QHD_LCD_TRULY_NT35516 = 10,
	MIPI_QHD_LCD_DIJING_NT35516 = 11,
	RGB_TS_LCD_NT35512_IPS = 12,
	MIPI_DJN_LCD_OTM8018B=13,
	MIPI_AZET_LCD_NT35510=14,
	MIPI_AZET_LCD_OTM8018B=15,
	MIPI_TRULY_LCD_OTM8018B=16,
	RGB_DNJ_LCD_OTM8018B_IPS_2 = 17,
	INVALID_LCD_TYPE,
};

enum product_type{
	RGTK_PRODUCT_Q801 = 0x08000100,
	RGTK_PRODUCT_Q802 = 0x08000200,
	RGTK_PRODUCT_Q803 = 0x08000300,
	RGTK_PRODUCT_Q203 = 0x08000400,
	RGTK_PRODUCT_Q203_NAND = 0x08000401,
	RGTK_PRODUCT_DC205 = 0x08000500,
	RGTK_PRODUCT_INVALID  = 0xFFFFFFFF,
};

enum {
	GPIO_OVP_FLAG_INDEX	= 0,
	GPIO_BT_SYS_REST_EN_INDEX,
       /*lilonghui add it for the camera flash light */
	GPIO_CAM_TRUE_FLASH_PWN_INDEX,
        GPIO_CAM_TRUE_FLASH_PWM_INDEX,
       /*end*/
	GPIO_CAM_FAKE_FLASH_EN_INDEX,
	GPIO_CAM_ID_INDEX,
    	GPIO_CAM_DRIVER_PWDN,
	GPIO_CAM_FRONT_PWDN_INDEX,	
	GPIO_CAM_FRONT_RESET_INDEX,	
	GPIO_ALS_INT_INDEX,
	GPIO_CAM_BACK_RESET_INDEX,	
	GPIO_CAM_BACK_PWDN_INDEX,    
	GPIO_FM_RDS_INT_INDEX,
	GPIO_SDC1_HW_DET_INDEX,
	GPIO_TOUCH_PANEL_RESET_INDEX,
	GPIO_TOUCH_PANEL_INT_INDEX,	
	GPIO_COMPASS_DRDY_INDEX,
	GPIO_GYRO_INT_N_INDEX,
	GPIO_ACC_INT_INDEX,
	GPIO_EXT_2V85_EN_INDEX,
	GPIO_LCD_RESET_INDEX,
	GPIO_LCD_LED_EN_INDEX,
	GPIO_KEYPAD_LED_EN_INDEX,
	GPIO_RED_LED_EN_INDEX,
	GPIO_GREEN_LED_EN_INDEX,
	KYPD_DRV0_INDEX,
	KYPD_DRV1_INDEX,
	KEYSENSE_N0_INDEX,
	KEYSENSE_N1_INDEX,
	GPIO_TOTAL_NUM,
};

extern unsigned int gpio_num[GPIO_TOTAL_NUM];
extern unsigned int gpio_cfg[GPIO_TOTAL_NUM];

/*===================Extern fuction decalre=====================*/

extern void q801_setup_gpio(void);
extern void q802_setup_gpio(void);
extern void q803_setup_gpio(void);
extern void q203_setup_gpio(void);
extern void q203_nand_setup_gpio(void);
extern void dc205_setup_gpio(void);

void set_rgtk_product(unsigned product);
unsigned get_rgtk_product(void);
bool is_rgtk_product(unsigned product);

void set_rgtk_lcd_id(int lcd_id);
unsigned get_rgtk_lcd_id(void);
unsigned is_rgtk_lcd_id(int lcd_id);

void set_cta_flag(unsigned flag);
extern unsigned get_cta_flag(void);

extern int creat_lcd_info_proc_file(void); //luke;
#endif
