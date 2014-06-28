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
#include "mipi_dijing_ips_nt35516.h"
#include <linux/leds.h>  // add by hanxiaohui
//#define DRVIE_BACKLIGHT_BY_HOST
#define GPIO_TCL_LCD_RESET 85
static struct msm_panel_common_pdata *mipi_nt35516_pdata;
static struct dsi_buf nt35516_tx_buf;
static struct dsi_buf nt35516_rx_buf;

#define BACKLIGHT_BRIGHTNESS_LEVEL 32
static int prev_bl = BACKLIGHT_BRIGHTNESS_LEVEL + 1 ;
static int gpio_backlight_en = 0xff;
static int gpio_keybaord_en = 0xff;

/* common setting */
static char exit_sleep[] = {0x11};
static char display_on[] = {0x29};
static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};
static char TCL_CUSTOMER_0X52[] = {0x53,0x00};  //write ctrl display
static struct dsi_cmd_desc nt35516_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(enter_sleep), enter_sleep},
        {DTYPE_DCS_WRITE, 1, 0, 0, 0, sizeof(TCL_CUSTOMER_0X52), TCL_CUSTOMER_0X52}
};
static char TCL_CUSTOMER_0X51[] = {0x51,0xFF};  //write brightness
static char TCL_CUSTOMER_0X53[] = {0x53,0x24};  //write ctrl display

static char TCL_CUSTOMER_0X55[] = {0x55,0x00};  //write content adaptive brightness ctrl

static struct dsi_cmd_desc tcl_cmd_drive_backlight_cmds[] = {
	{DTYPE_GEN_WRITE1, 1, 0, 0, 0, sizeof(TCL_CUSTOMER_0X51), TCL_CUSTOMER_0X51},
	{DTYPE_GEN_WRITE1, 1, 0, 0, 0, sizeof(TCL_CUSTOMER_0X53), TCL_CUSTOMER_0X53},
	{DTYPE_GEN_WRITE1, 1, 0, 0, 0, sizeof(TCL_CUSTOMER_0X55), TCL_CUSTOMER_0X55},
};
static char cmd0_1[] = {
	0x35, 
0x00
};
static char cmd0[] = {
	0xFF, 
0xAA, 0x55, 0x25,0x01
};
static char cmd1[] = {
	0xF2,
0x00,0x00,0x4A,0x0A,
0xA8,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x0B,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x40,0x01,0x51,0x00,
0x01,0x00,0x01
};
static char cmd2[] = {
	0xF3,
0x02,0x03,0x07,0x45,
0x88,0xD4,0x0D,
};

static char cmd3[] = {
	0xF0,
0x55,0xAA,0x52,0x08,
0x00
};

static char cmd4[] = {
	0xB1,
0xEC,///0x0c
0x00,
0x00
};
static char cmd5[] = {
	0xB8,
0x01,0x02,
0x02,0x02
};
static char cmd5_1[] = {
	0xBB,
0x63, 0x03,0x63
};
static char cmd5_2[] = {
	0xBC,
0x00, 0x00,0x00
};
static char cmd6[] = {
	0xC9,
0x63,0x06,
0x0D,0x1A,
0x17,0x00
};

static char cmd7[] = {
	0xF0,
0x55,0xAA,
0x52,0x08,
0x01
};
static char cmd71[] = {
	0xB0,
0x05,0x05,
0x05
};

static char cmd8[] = {
	0xB1,
0x05,0x05,
0x05
};
//VGL_REG -10V
static char cmd9[] = {
	0xB2,
0x01,0x01,
0x01

};
static char cmd10[] = {
	0xB3,
0x0E,0x0E,
0x0E

};

static char cmd11[] = {
	0xB4, 
0x0A,0x0A,
0xA
};

static char cmd12[] = {
	0xB6,
0x44,0x44,
0x44
};
static char cmd13[] = {
	0xB7,
0x34,0x34,
0x34
};

static char cmd14[] = {
	0xB8,
0x20,///
0x20,///
0x20///
};
static char cmd15[] = {
	0xB9,
0x26,0x26,
0x26
};


static char cmd16[] = {
	0xBA,
0x24,0x24,0x24
};
static char cmd17[] = {
	0xBC,
0x00,0xC8,0x00
};

static char cmd18[] = {
	0xBD,
0x00,0xC8,0x00
};
static char cmd19[] = {
	0xBE,
0x82		
};
static char cmd20[] = {
	0xC0,
0x00,0x08
};
static char cmd21[] = {
	0xCA,
0x00
};
static char cmd22[] = {
	0xD0,
0x0A,0x10,
0x0D,0x0F
};
//Gamma Setting
static char cmd23[] = {
	0xD1,
0x00,0x70,0x00,0xCE,
0x00,0xf7,0x01,0x10,
0x01,0x21,0x01,0x44,
0x01,0x8A,0x01,0xA5
};
static char cmd24[] = {
	0xD2,
0x01,0xBF,0x01,0xe4,
0x02,0x0c,0x02,0x4d,
0x02,0x82,0x02,0x84,
0x02,0xB8,0x02,0xF0
};
static char cmd25[] = {
	0xD3,
0x03,0x14,0x03,0x42,
0x03,0x5e,0x03,0x80,
0x03,0x97,0x03,0xB0,
0x03,0xC0,0x03,0xDF
};
static char cmd26[] = {
	0xD4,
0x03,0xFD,
0x03,0xFF
};
static char cmd27[] = {
	0xD5,
0x00,0x70,0x00,0xCE,
0x00,0xF7,0x01,0x10,
0x01,0x21,0x01,0x44,
0x01,0x8A,0x01,0xA5
};
static char cmd28[] = {
	0xD6,
0x01,0xBf,0x01,0xe4,
0x02,0x0c,0x02,0x4d,
0x02,0x82,0x02,0x84,
0x02,0xb8,0x02,0xf0
};
static char cmd29[] = {
	0xD7,
0x03,0x14,0x03,0x42,
0x03,0x5e,0x03,0x80,
0x03,0x97,0x03,0xb0,
0x03,0xc0,0x03,0xdf
};

static char cmd30[] = {
	0xD8,
0x03,0xFD,
0x03,0xFF
};
static char cmd31[] = {
	0xD9,
0x00,0x70,0x00,0xCE,
0x00,0xF7,0x01,0x10,
0x01,0x21,0x01,0x44,
0x01,0x8A,0x01,0xA5
};

static char cmd32[] = {
	0xDD,
0x01,0xBF,0x01,0xe4,
0x02,0x0c,0x02,0x4d,
0x02,0x82,0x02,0x84,
0x02,0xb8,0x02,0xf0
};

static char cmd33[] = {
	0xDE,
0x03,0x14,0x03,0x42,
0x03,0x5e,0x03,0x80,
0x03,0x97,0x03,0xb0,
0x03,0xc0,0x03,0xdf
};

static char cmd34[] = {
	0xDF,
0x03,0xFD,
0x03,0xFF
};
static char cmd35[] = {
	0xE0,
0x00,0x70,0x00,0xCE,
0x00,0xF7,0x01,0x10,
0x01,0x21,0x01,0x44,
0x01,0x8A,0x01,0xA5
};

static char cmd36[] = {
        0xE1,
0x01,0xBf,0x01,0xe4,
0x02,0x0c,0x02,0x4d,
0x02,0x82,0x02,0x84,
0x02,0xb8,0x02,0xf0
};

static char cmd37[] = {
        0xE2,
0x03,0x14,0x03,0x42,
0x03,0x5e,0x03,0x80,
0x03,0x97,0x03,0xb0,
0x03,0xc0,0x03,0xdf
};
static char cmd38[] = {
	0xE3,
0x03,0xFD,
0x03,0xFF
};

static char cmd39[] = {
        0xE4,
0x00,0x70,0x00,0xCE,
0x00,0xF7,0x01,0x10,
0x01,0x21,0x01,0x44,
0x01,0x8A,0x01,0xA5
};

static char cmd40[] = {
        0xE5,
0x01,0xBF,0x01,0xe4,
0x02,0x0c,0x02,0x4d,
0x02,0x82,0x02,0x84,
0x02,0xb8,0x02,0xf0
};
static char cmd41[] = {
        0xE6,
0x03,0x14,0x03,0x42,
0x03,0x5e,0x03,0x80,
0x03,0x97,0x03,0xb0,
0x03,0xc0,0x03,0xdf
};
static char cmd42[] = {
        0xE7,
0x03,0xFD,
0x03,0xFF
};
static char cmd43[] = {
       0xE8,
0x00,0x70,0x00,0xCE,
0x00,0xF7,0x01,0x10,
0x01,0x21,0x01,0x44,
0x01,0x8A,0x01,0xA5
};
static char cmd44[] = {
        0xE9,
0x01,0xBF,0x01,0xe4,
0x02,0x0c,0x02,0x4d,
0x02,0x82,0x02,0x84,
0x02,0xb8,0x02,0xf0
};
static char cmd45[] = {
        0xEA,
0x03,0x14,0x03,0x42,
0x03,0x5e,0x03,0x80,
0x03,0x97,0x03,0xb0,
0x03,0xc0,0x03,0xdf
};
static char cmd46[] = {
        0xEB,
0x03,0xFD,
0x03,0xFF
};

	
static struct dsi_cmd_desc nt35516_cmd_display_on_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd0_1),  cmd0_1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd0),  cmd0},   //
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd1),  cmd1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd2),  cmd2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd3),  cmd3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd4),  cmd4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd5),  cmd5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd5_1),  cmd5_1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd5_2),  cmd5_2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd6),  cmd6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd7),  cmd7},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd71),  cmd71},
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
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(cmd19), cmd19},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd20), cmd20},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(cmd21), cmd21},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd22), cmd22},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd23), cmd23},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd24), cmd24},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd25), cmd25},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd26), cmd26},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd27), cmd27},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd28), cmd28},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd29), cmd29},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd30), cmd30},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd31), cmd31},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd32), cmd32},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd33), cmd33},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd34), cmd34},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd35), cmd35},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd36), cmd36},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd37), cmd37},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd38), cmd38},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd39), cmd39},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd40), cmd40},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd41), cmd41},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd42), cmd42},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd43), cmd43},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd44), cmd44},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd45), cmd45},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(cmd46), cmd46},

        
	{DTYPE_DCS_WRITE, 1, 0, 0, 150,            sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, 20,            sizeof(display_on), display_on},

};

static char  ic_status_0x0a[] = {0x0a};

static struct dsi_cmd_desc get_nt35516_0x0a_value_cmd[] = {{DTYPE_DCS_READ, 1, 0, 0, 0, sizeof(ic_status_0x0a), ic_status_0x0a}};

static int nt35516_read_panel_reg(struct msm_fb_data_type *mfd, char reg) 
{ 
        struct dsi_buf *rp, *tp; 
        struct dsi_cmd_desc *cmd; 
        unsigned char  *lp; 

        tp = &nt35516_tx_buf; 
        rp = &nt35516_tx_buf; 
        mipi_dsi_buf_init(tp); 
        mipi_dsi_buf_init(rp); 

        switch(reg) 
        { 
                case 0x0a: 
                        cmd = get_nt35516_0x0a_value_cmd; 
                break; 

                default: 
			cmd = get_nt35516_0x0a_value_cmd;
                       // cmd = nt35516_manufacture_id;//default read manufacture id 
                break; 
        } 
        mipi_dsi_cmds_rx(mfd, tp, rp, cmd, 1); 
        lp = (unsigned char  *)rp->data; 
        return *lp; 
} 

static unsigned int nt35516_monitor_reg_status(struct msm_fb_data_type *mfd) 
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
        
        ret = nt35516_read_panel_reg(mfd,0x0a); 
        printk("hxh:  the returned value of 0x0a is  0x%x\n",ret);
exit_stop_state: 
        mipi_set_tx_power_mode(0); //entry high speed mode 
        //if(mipi_dsi_wait_stop_state()) {err = 7;} 
         return 1; 
}


static int mipi_dijing_lcd_reset(void)
{
    int rc = 0;
printk("hxh: %s  %d \n", __func__, __LINE__	);
    gpio_set_value_cansleep(GPIO_TCL_LCD_RESET, 1);
    msleep(20);
    gpio_set_value_cansleep(GPIO_TCL_LCD_RESET, 0);
    msleep(20);
    gpio_set_value_cansleep(GPIO_TCL_LCD_RESET, 1);
    msleep(120);

    return rc;
}



static int mipi_dijing_nt35516_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	if (mipi_dijing_lcd_reset() < 0) {
		pr_err("mipi_tcl_dijing_reset error\n");
		return -EINVAL;
	}


if (mfd->panel_info.mipi.mode == DSI_CMD_MODE) {
		mipi_dsi_cmds_tx(mfd, &nt35516_tx_buf,nt35516_cmd_display_on_cmds,
			ARRAY_SIZE(nt35516_cmd_display_on_cmds));

	}

nt35516_monitor_reg_status(mfd); 
	return 0;
}

static int mipi_dijing_nt35516_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
printk("hxh: %s  %d \n", __func__, __LINE__	);
	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	mipi_dsi_cmds_tx(mfd, &nt35516_tx_buf, nt35516_display_off_cmds,
			ARRAY_SIZE(nt35516_display_off_cmds));

	return 0;
}

static int __devinit mipi_dijing_nt35516_lcd_probe(struct platform_device *pdev)
{
	int rc = 0;
	pr_debug("%s\n", __func__);
printk("hxh: %s  %d \n", __func__, __LINE__	);
	if (pdev->id == 0) {
		mipi_nt35516_pdata = pdev->dev.platform_data;
	    	if (mipi_nt35516_pdata) {
			gpio_backlight_en = mipi_nt35516_pdata->gpio;		
	    	}
		return 0;
	}

	rc = gpio_request(GPIO_TCL_LCD_RESET, "gpio_disp_reset");
	if(rc < 0) {
		pr_err("Error request gpio %d\n", GPIO_TCL_LCD_RESET);
	}

	rc = gpio_tlmm_config(GPIO_CFG(GPIO_TCL_LCD_RESET, 0,
				GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	
	if (rc < 0) {
		pr_err("Error config lcdc gpio:%d\n", GPIO_TCL_LCD_RESET);
	}

	rc = gpio_direction_output(GPIO_TCL_LCD_RESET, 0);
	if (rc < 0) {
		pr_err("Error direct lcdc gpio:%d\n", GPIO_TCL_LCD_RESET);
	}

	msm_fb_add_device(pdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_dijing_nt35516_lcd_probe,
	.driver = {
		.name   = "mipi_dijing_nt35516_qhd_pt",
	},
};

static void set_backlight_by_driver(struct msm_fb_data_type *mfd)
{
	int bl_level = mfd->bl_level;
	printk("hxh: %s  %d \n", __func__, __LINE__	);
	TCL_CUSTOMER_0X51[1] = bl_level;

        mipi_dsi_mdp_busy_wait(mfd);	
	mipi_dsi_cmds_tx(mfd, &nt35516_tx_buf, tcl_cmd_drive_backlight_cmds,
		    ARRAY_SIZE(tcl_cmd_drive_backlight_cmds));
	nt35516_monitor_reg_status(mfd); 
	printk("luke: second turn backlight on\n");	
	
	if(0 == bl_level){
		//Start===Allen
		keyboard_led_light(KEY_LED_USER_LCD, 0);
		//End===Allen
	}else {
		//Start===Allen
		keyboard_led_light(KEY_LED_USER_LCD,1);
		//End===Allen
	}    
}

static void set_backlight_by_host(struct msm_fb_data_type *mfd)
{
	int step = 0, i = 0;
    	unsigned long flags;
	int bl_level = mfd->bl_level;
printk("hxh: %s  %d \n", __func__, __LINE__	);
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

static void  mipi__dijing_nt35516_set_backlight(struct msm_fb_data_type *mfd)
{
	if(driver_by_host) {
		set_backlight_by_host(mfd);
	}
	else {
		set_backlight_by_driver(mfd);
	}
}

static struct msm_fb_panel_data nt35516_panel_data = {
	.on  = mipi_dijing_nt35516_lcd_on,
	.off = mipi_dijing_nt35516_lcd_off,
	.set_backlight = mipi__dijing_nt35516_set_backlight,
};

static int ch_used[3];

int mipi_nt35516_dijing_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

printk("hxh: %s  %d \n", __func__, __LINE__	);

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_dijing_nt35516_qhd_pt", (panel << 8)|channel);

	if (!pdev)
		return -ENOMEM;

	nt35516_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &nt35516_panel_data,
		sizeof(nt35516_panel_data));

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


static int __init mipi_dijing_nt35516_lcd_init(void)
{
	mipi_dsi_buf_alloc(&nt35516_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&nt35516_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

module_init(mipi_dijing_nt35516_lcd_init);

