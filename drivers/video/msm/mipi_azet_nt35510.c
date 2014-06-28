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
#include "mipi_azet_nt35510.h"
#include <linux/leds.h>  // add by hanxiaohui
#include <linux/board-ragentek-cfg.h>

//#define DRVIE_BACKLIGHT_BY_HOST
#define GPIO_AZET_LCD_RESET 85

static struct msm_panel_common_pdata *mipi_nt35510_pdata;
static struct dsi_buf nt35510_tx_buf;
static struct dsi_buf nt35510_rx_buf;

#define BACKLIGHT_BRIGHTNESS_LEVEL 32
static int prev_bl = BACKLIGHT_BRIGHTNESS_LEVEL + 1 ;
static int gpio_backlight_en = 0xff;
static int gpio_keybaord_en = 0xff;

/* common setting */
static char exit_sleep[] = {0x11};
static char display_on[] = {0x29};
static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};
static char write_ram[2] = {0x2c, 0x00}; /* write ram */
static char sw_reset[] = {0x01, 0x00}; 
//static char AZET_CUSTOMER_0X52[] = {0x53,0x00};  //write ctrl display //modify for QELS-632 by hxh 2012.11.12 
static struct dsi_cmd_desc nt35510_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 30, sizeof(display_off), display_off},//modify (0->30)for QELS-632 by hxh 2012.11.12
	{DTYPE_DCS_WRITE, 1, 0, 0, 30, sizeof(enter_sleep), enter_sleep},//modify (0->30)for QELS-632 by hxh 2012.11.12
       // {DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(AZET_CUSTOMER_0X52), AZET_CUSTOMER_0X52}  //modify for QELS-632by hxh 2012.11.12
};
static char AZET_CUSTOMER_0X51[] = {0x51,0xFF};  //write brightness
static char AZET_CUSTOMER_0X53[] = {0x53,0x24};  //write ctrl display

static char AZET_CUSTOMER_0X55[] = {0x55,0x00};  //write content adaptive brightness ctrl

static struct dsi_cmd_desc azet_cmd_drive_backlight_cmds[] = {
	{DTYPE_GEN_WRITE1, 1, 0, 0, 0, sizeof(AZET_CUSTOMER_0X51), AZET_CUSTOMER_0X51},
	{DTYPE_GEN_WRITE1, 1, 0, 0, 0, sizeof(AZET_CUSTOMER_0X53), AZET_CUSTOMER_0X53},
	{DTYPE_GEN_WRITE1, 1, 0, 0, 0, sizeof(AZET_CUSTOMER_0X55), AZET_CUSTOMER_0X55},
};

//static char ddd[]={0xd0, 0x20};
#if 1
//LV2 Page 1 enable
static char cmd0[] = {
	0xF0, 
0x55,
0xAA,
0x52,	
0x08, 
0x01
};
//AVDD Set AVDD 5.2V 
static char cmd1[] = {
	0xB0,
0x0C,
0x0C,
0x0C
};
//AVDD ratio
static char cmd2[] = {
	0xB6,
0x34,
0x34,
0x34

};
//AVEE  -5.2V
static char cmd3[] = {
	0xB1,
0x0C, 
0x0C, 
0x0C 
};
//AVEE ratio
static char cmd4[] = {
	0xB7, 
0x35,
0x35,
0x35
};

//VCL  -2.5V
static char cmd5[] = {
	0xB2, 
0x00,
0x00,
0x00

};
//VCL ratio
static char cmd6[] = {
	0xB8, 
0x24,
0x24,
0x24
};

//VGH 15V  (Free pump)
static char cmd7[] = {
	0xBF,
0x01 
};
static char cmd71[] = {
	0xB3,
0x08,
0x08,
0x08
};
//VGH ratio
static char cmd8[] = {
	0xB9,
0x37,
0x37,
0x37
};
//VGL_REG -10V
static char cmd9[] = {
	0xB5, 
0x08,
0x08,
0x08
};
static char cmd10[] = {
	0xC2,
0x03
};
//VGLX ratio
static char cmd11[] = {
	0xBA, 
0x24,
0x24,
0x24
};
//VGMP/VGSP 4.5V/0V
static char cmd12[] = {
	0xBC,
0x00,
0xA0,
0x00
};
//VGMN/VGSN -4.5V/0V
static char cmd13[] = {
	0xBD, 
0x00,
0xA0,
0x00
};
//VCOM  -1.25V
static char cmd14[] = {
	0xBE, 
0x00,
0xA3
};

//Gamma Setting   //change the gamma value by hxh 2012.11.14 start
static char cmd15[] = {
	0xD1, 
0x00,
0x00,
0x00,
0x09,
0x00,
0x1A,
0x00,
0x49,
0x00,
0x73,
0x00,
0xB1,
0x00,
0xDE,
0x01,
0x1E,
0x01,
0x4C,
0x01,
0x8C,
0x01,
0xB9,
0x01,
0xFB,
0x02,
0x2F,
0x02,
0x31,
0x02,
0x5D,
0x02,
0x89,
0x02,
0xA1,
0x02,
0xC0,
0x02,
0xD5,
0x02,
0xF5,
0x03,
0x14,
0x03,
0x1F,
0X03,
0x29,
0x03,
0x38,
0x03,
0x5B,
0x03,
0x9F
};
static char cmd16[] = {			             	                                                                 
        0xD2,
0x00,
0x00,
0x00,
0x09,
0x00,
0x1A,
0x00,
0x49,
0x00,
0x73,
0x00,
0xB1,
0x00,
0xDE,
0x01,
0x1E,
0x01,
0x4C,
0x01,
0x8C,
0x01,
0xB9,
0x01,
0xFB,
0x02,
0x2F,
0x02,
0x31,
0x02,
0x5D,
0x02,
0x89,
0x02,
0xA1,
0x02,
0xC0,
0x02,
0xD5,
0x02,
0xF5,
0x03,
0x14,
0x03,
0x1F,
0X03,
0x29,
0x03,
0x38,
0x03,
0x5B,
0x03,
0x9F

};
static char cmd17[] = {
	0xD3, 
0x00,
0x00,
0x00,
0x09,
0x00,
0x1A,
0x00,
0x49,
0x00,
0x73,
0x00,
0xB1,
0x00,
0xDE,
0x01,
0x1E,
0x01,
0x4C,
0x01,
0x8C,
0x01,
0xB9,
0x01,
0xFB,
0x02,
0x2F,
0x02,
0x31,
0x02,
0x5D,
0x02,
0x89,
0x02,
0xA1,
0x02,
0xC0,
0x02,
0xD5,
0x02,
0xF5,
0x03,
0x14,
0x03,
0x1F,
0X03,
0x29,
0x03,
0x38,
0x03,
0x5B,
0x03,
0x9F

};
static char cmd18[] = {	
	0xD4,
0x00,
0x00,
0x00,
0x09,
0x00,
0x1A,
0x00,
0x49,
0x00,
0x73,
0x00,
0xB1,
0x00,
0xDE,
0x01,
0x1E,
0x01,
0x4C,
0x01,
0x8C,
0x01,
0xB9,
0x01,
0xFB,
0x02,
0x2F,
0x02,
0x31,
0x02,
0x5D,
0x02,
0x89,
0x02,
0xA1,
0x02,
0xC0,
0x02,
0xD5,
0x02,
0xF5,
0x03,
0x14,
0x03,
0x1F,
0X03,
0x29,
0x03,
0x38,
0x03,
0x5B,
0x03,
0x9F
};
static char cmd19[] = {
	0xD5,
0x00,
0x00,
0x00,
0x09,
0x00,
0x1A,
0x00,
0x49,
0x00,
0x73,
0x00,
0xB1,
0x00,
0xDE,
0x01,
0x1E,
0x01,
0x4C,
0x01,
0x8C,
0x01,
0xB9,
0x01,
0xFB,
0x02,
0x2F,
0x02,
0x31,
0x02,
0x5D,
0x02,
0x89,
0x02,
0xA1,
0x02,
0xC0,
0x02,
0xD5,
0x02,
0xF5,
0x03,
0x14,
0x03,
0x1F,
0X03,
0x29,
0x03,
0x38,
0x03,
0x5B,
0x03,
0x9F
};
static char cmd20[] = {		
	0xD6,
0x00,
0x00,
0x00,
0x09,
0x00,
0x1A,
0x00,
0x49,
0x00,
0x73,
0x00,
0xB1,
0x00,
0xDE,
0x01,
0x1E,
0x01,
0x4C,
0x01,
0x8C,
0x01,
0xB9,
0x01,
0xFB,
0x02,
0x2F,
0x02,
0x31,
0x02,
0x5D,
0x02,
0x89,
0x02,
0xA1,
0x02,
0xC0,
0x02,
0xD5,
0x02,
0xF5,
0x03,
0x14,
0x03,
0x1F,
0X03,
0x29,
0x03,
0x38,
0x03,
0x5B,
0x03,
0x9F
};
//change the gamma value by hxh 2012.11.14 end
//LV2 Page 0 enable
static char cmd21[] = {
	0xF0,
0x55,
0xAA,
0x52,
0x08,
0x00
};
//480x800
static char cmd22[] = {
	0xB5,
0x6B
};
//Display control
static char cmd23[] = {
	0xB1,
 0xFC,
 0x00 
};

//Source hold time
static char cmd24[] = {
	0xB6,
 0x05
};

//Gate EQ control
static char cmd25[] = {
	0xB7,
0x70,
0x70
};
//Source EQ control (Mode 2)
static char cmd26[] = {
	0xB8,
 0x01,
 0x05,
 0x05,
 0x05
};
//Inversion mode  (2-dot)
static char cmd27[] = {
	0xBC,
0x02,//modify from 00->02 by hxh for QELS-146 2012.11.12
0x00,
0x00
};
//Frame rate
static char cmd28[] = {
        0xBD,
0x01,
0x84,
0x2F, //1C
0x2F, //1C
0x00
};

//Timing control 4H w/ 4-delay
static char cmd29[] = {
        0xC9,
0xD0,
0x82,
0x50,
0x50,
0x50
};
static char cmd30[] = {
	0x36, 0x00
};

static char cmd31[] = {
        0x35, 0x00
};

static char cmd32[] = {
        0x3a, 0x77
};
#endif
static struct dsi_cmd_desc nt35510_cmd_display_on_cmds[] = {


        {DTYPE_DCS_WRITE, 1, 0, 0, 20, sizeof(sw_reset), sw_reset},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd0),  cmd0},   //
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd1),  cmd1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd2),  cmd2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd3),  cmd3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd4),  cmd4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd5),  cmd5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd6),  cmd6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd7),  cmd7},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd71), cmd71},  //
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd8),  cmd8},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd9),  cmd9},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd10), cmd10},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd11), cmd11},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd12), cmd12},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd13), cmd13},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd14), cmd14},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd15), cmd15},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd16), cmd16},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd17), cmd17},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd18), cmd18},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd19), cmd19},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd20), cmd20},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd21), cmd21},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd22), cmd22},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd23), cmd23},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd24), cmd24},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd25), cmd25},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd26), cmd26},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd27), cmd27},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd28), cmd28},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd29), cmd29},
	//{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(cmd30), cmd30},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd31), cmd31},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd32), cmd32},
	
	//{DTYPE_GEN_LWRITE, 1, 0, 0, 120, sizeof(sw_reset), sw_reset},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 20,  sizeof(display_on), display_on},
        {DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(cmd30), cmd30},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0,  sizeof(write_ram), write_ram},
};

static char  ic_status_0x0a[] = {0x0a};

static struct dsi_cmd_desc get_nt35510_0x0a_value_cmd[] = {{DTYPE_DCS_READ, 1, 0, 0, 0, sizeof(ic_status_0x0a), ic_status_0x0a}};

static int azet_nt35510_read_panel_reg(struct msm_fb_data_type *mfd, char reg) 
{ 
        struct dsi_buf *rp, *tp; 
        struct dsi_cmd_desc *cmd; 
        unsigned char  *lp; 

        tp = &nt35510_tx_buf; 
        rp = &nt35510_tx_buf; 
        mipi_dsi_buf_init(tp); 
        mipi_dsi_buf_init(rp); 

        switch(reg) 
        { 
                case 0x0a: 
                        cmd = get_nt35510_0x0a_value_cmd; 
                break; 

                default: 
			cmd = get_nt35510_0x0a_value_cmd;
                       // cmd = nt35510_manufacture_id;//default read manufacture id 
                break; 
        } 
        mipi_dsi_cmds_rx(mfd, tp, rp, cmd, 1); 
        lp = (unsigned char  *)rp->data; 
        return *lp; 
} 

unsigned int azet_nt35510_monitor_reg_status(struct msm_fb_data_type *mfd) 
{ 
        uint32 data=0; 
        uint16 cnt = 0,ret=0; 
        uint8 err=0;     //ok 
      
       // if(mipi_dsi_wait_stop_state()) return 1; 
        
        mipi_set_tx_power_mode(1);                      //entry low power mode         
        MIPI_OUTP(MIPI_DSI_BASE + 0x094, 0x01);        //sw bta trigger 
        wmb(); 
        cnt = 0; 
        while (cnt++ < 10000) { 
                data = MIPI_INP(MIPI_DSI_BASE + 0x0004);/* DSI1_STATUS */ 
                if ((data & 0x0010) == 0){              //out of bta busy 
                        break; 
                } 
        } 
        mipi_dsi_ack_err_status(); 
        if(cnt >= 10000){err = 2; goto exit_stop_state; } 
        
        ret =azet_nt35510_read_panel_reg(mfd,0x0a); 
       // printk("luke:  the returned value of 0x0a is  0x%x\n",ret);
exit_stop_state: 
        mipi_set_tx_power_mode(0); //entry high speed mode 
        //if(mipi_dsi_wait_stop_state()) {err = 7;} 
         return 1; 
}


static int mipi_azet_lcd_reset(void)
{
    int rc = 0;
printk("luke: %s  %d \n", __func__, __LINE__	);
    gpio_set_value_cansleep(gpio_num[GPIO_LCD_RESET_INDEX], 1);
    msleep(20);
    gpio_set_value_cansleep(gpio_num[GPIO_LCD_RESET_INDEX], 0);
    msleep(20);
    gpio_set_value_cansleep(gpio_num[GPIO_LCD_RESET_INDEX], 1);
    msleep(120);

    return rc;
}

static int mipi_nt35510_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	if(!mfd->cont_splash_done){
        		mfd->cont_splash_done = 1;
		}//add for continue_display_splash.hxh
	if (mipi_azet_lcd_reset() < 0) {
		pr_err("mipi_azet_lcd_reset error\n");
		return -EINVAL;
	}
     
if (mfd->panel_info.mipi.mode == DSI_CMD_MODE) {
		mipi_dsi_cmds_tx(mfd, &nt35510_tx_buf,nt35510_cmd_display_on_cmds,
			ARRAY_SIZE(nt35510_cmd_display_on_cmds));

	}

azet_nt35510_monitor_reg_status(mfd); 
	return 0;
}

static int mipi_nt35510_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
printk("luke: %s  %d \n", __func__, __LINE__	);
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	mipi_dsi_cmds_tx(mfd, &nt35510_tx_buf, nt35510_display_off_cmds,
			ARRAY_SIZE(nt35510_display_off_cmds));

	return 0;
}

static int __devinit mipi_nt35510_lcd_probe(struct platform_device *pdev)
{
	int rc = 0;
	pr_debug("%s\n", __func__);
//printk("luke: %s  %d \n", __func__, __LINE__	);
	if (pdev->id == 0) {
		mipi_nt35510_pdata = pdev->dev.platform_data;
	    	if (mipi_nt35510_pdata) {
			gpio_backlight_en = mipi_nt35510_pdata->gpio;		
	    	}
		return 0;
	}

	rc = gpio_request(gpio_num[GPIO_LCD_RESET_INDEX], "gpio_disp_reset");
	if(rc < 0) {
		pr_err("Error request gpio %d\n", gpio_num[GPIO_LCD_RESET_INDEX]);
	}

	rc = gpio_tlmm_config(gpio_cfg[GPIO_LCD_RESET_INDEX], GPIO_CFG_ENABLE);
	
	if (rc < 0) {
		pr_err("Error config lcdc gpio:%d\n", gpio_num[GPIO_LCD_RESET_INDEX]);
	}

	rc = gpio_direction_output(gpio_num[GPIO_LCD_RESET_INDEX], 1);
	if (rc < 0) {
		pr_err("Error direct lcdc gpio:%d\n", gpio_num[GPIO_LCD_RESET_INDEX]);
	}

	msm_fb_add_device(pdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_nt35510_lcd_probe,
	.driver = {
		.name   = "mipi_azet_nt35510",
	},
};

static void set_backlight_by_driver(struct msm_fb_data_type *mfd)
{
	int bl_level = mfd->bl_level;
	AZET_CUSTOMER_0X51[1] = bl_level;

        mipi_dsi_mdp_busy_wait(mfd);	
	mipi_dsi_cmds_tx(mfd, &nt35510_tx_buf, azet_cmd_drive_backlight_cmds,
		    ARRAY_SIZE(azet_cmd_drive_backlight_cmds));
	//azet_nt35510_monitor_reg_status(mfd); 
	
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
}

static void set_backlight_by_host(struct msm_fb_data_type *mfd)
{
	int step = 0, i = 0;
    	unsigned long flags;
	int bl_level = mfd->bl_level;
printk("luke: %s  %d \n", __func__, __LINE__	);
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
		pr_debug("%s: turn off backlight\n", __func__);
		gpio_set_value_cansleep(gpio_backlight_en, 0);
		/*lilonghui add it for the keybaord led 2012-3-13*/
			mdelay(4);
		gpio_set_value_cansleep(gpio_keybaord_en, 0);
			udelay(30);
	} else {
        local_irq_save(flags);	
         
		if (prev_bl == (BACKLIGHT_BRIGHTNESS_LEVEL + 1)) {
			/* turn on backlight */
			pr_debug("%s: turn on backlight\n", __func__);
			gpio_set_value_cansleep(gpio_backlight_en, 1);
			udelay(400);
			/*lilonghui add it for the keybaord led 2012-3-13*/
		        gpio_set_value_cansleep(gpio_keybaord_en, 1);
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
	return;
}

#ifdef DRVIE_BACKLIGHT_BY_HOST
	static int driver_by_host = 1;
#else
	static int driver_by_host = 0;
#endif

static void  mipi_nt35510_azet_set_backlight(struct msm_fb_data_type *mfd)
{
	if(driver_by_host) {
		set_backlight_by_host(mfd);
	}
	else {
		set_backlight_by_driver(mfd);
	}
}

static struct msm_fb_panel_data nt35510_panel_data = {
	.on  = mipi_nt35510_lcd_on,
	.off = mipi_nt35510_lcd_off,
	.set_backlight = mipi_nt35510_azet_set_backlight,
};

static int ch_used[3];

int mipi_nt35510_azet_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

printk("luke: %s  %d \n", __func__, __LINE__	);

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_azet_nt35510", (panel << 8)|channel);

	if (!pdev)
		return -ENOMEM;

	nt35510_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &nt35510_panel_data,
		sizeof(nt35510_panel_data));

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


static int __init mipi_nt35510_lcd_init(void)
{
	mipi_dsi_buf_alloc(&nt35510_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&nt35510_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

module_init(mipi_nt35510_lcd_init);

