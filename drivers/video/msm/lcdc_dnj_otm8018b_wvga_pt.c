/* Copyright (c) 2009-2011, Code Aurora Forum. All rights reserved.
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

#include <linux/delay.h>
#include <linux/module.h>
#include <linux/leds.h>
#include <mach/gpio.h>
#include <mach/pmic.h>
#include "msm_fb.h"

#define CONFIG_FB_MSM_LCDC_AUTO_DETECT

static int prev_bl = 32;

static int spi_cs;
static int spi_sclk;
static int spi_mosi;
static int gpio_backlight_en;
static int gpio_display_reset;

struct lcdc_state_type{
	boolean disp_initialized;
	boolean display_on;
	boolean disp_powered_up;
};

static struct lcdc_state_type lcdc_state = { 0 };
static struct msm_panel_common_pdata *lcdc_pdata;

static __inline__ int  WriteREG_command(unsigned char c1,unsigned char c2)
{
	unsigned char i,j,k;

	j=0x20;
	gpio_set_value_cansleep(spi_sclk, 0); 
        udelay(1);
	gpio_set_value_cansleep(spi_mosi, 0);
        udelay(1);
	gpio_set_value_cansleep(spi_cs, 0);
        udelay(1);
	for(i=0;i<8;i++)
	{
	    	gpio_set_value_cansleep(spi_sclk, 0);
        	udelay(1);
		if(j&0x80)
			gpio_set_value_cansleep(spi_mosi, 1);
		else    
			gpio_set_value_cansleep(spi_mosi, 0);

		udelay(1);
		gpio_set_value_cansleep(spi_sclk, 0);
		k=j<<1;
		gpio_set_value_cansleep(spi_sclk, 1); 
		        udelay(1);
		j=k;
	}

	for(i=0;i<8;i++)
	{
	   	gpio_set_value_cansleep(spi_sclk, 0);
	    	udelay(1);
		if(c1&0x80)
			gpio_set_value_cansleep(spi_mosi, 1);
		else    
			gpio_set_value_cansleep(spi_mosi, 0);
		
		gpio_set_value_cansleep(spi_sclk, 0);
		k=c1<<1;
		udelay(1);
		gpio_set_value_cansleep(spi_sclk, 1); 
		udelay(1);
		c1=k;
	}
	gpio_set_value_cansleep(spi_cs, 1);

	j=0x00;
	gpio_set_value_cansleep(spi_sclk, 0);
	udelay(1);
	gpio_set_value_cansleep(spi_mosi, 0);
	udelay(1);
	gpio_set_value_cansleep(spi_cs, 0);
	for(i=0;i<8;i++)
	{
	    gpio_set_value_cansleep(spi_sclk, 0);
	   	udelay(1);
		if(j&0x80)
			gpio_set_value_cansleep(spi_mosi, 1);
		else    
			gpio_set_value_cansleep(spi_mosi, 0);

		gpio_set_value_cansleep(spi_sclk, 0);
		k=j<<1;
		udelay(1);
		gpio_set_value_cansleep(spi_sclk, 1); 
		udelay(1);
		j=k;
	}
	for(i=0;i<8;i++)
	{
	   	gpio_set_value_cansleep(spi_sclk, 0);
	        udelay(1);
		if(c2&0x80)
			gpio_set_value_cansleep(spi_mosi, 1);
		else    
			gpio_set_value_cansleep(spi_mosi, 0);

		gpio_set_value_cansleep(spi_sclk, 0);
		k=c2<<1;
		udelay(1);
		gpio_set_value_cansleep(spi_sclk, 1); 
		udelay(1);
		c2=k;
	}
	gpio_set_value_cansleep(spi_cs, 1); 
	udelay(1);
	gpio_set_value_cansleep(spi_sclk, 0);
	udelay(1);
	gpio_set_value_cansleep(spi_mosi, 0);
	udelay(1);
	
	return 0;
}

static __inline__  int  WriteREG_para(unsigned char d)
{
	unsigned char i,j,k;
       
	j=0x40;
	gpio_set_value_cansleep(spi_sclk, 0);
	udelay(1);
	gpio_set_value_cansleep(spi_mosi, 0);
	udelay(1);
	gpio_set_value_cansleep(spi_cs, 0);
	for(i=0;i<8;i++)
	{
	        gpio_set_value_cansleep(spi_sclk, 0);
	   	udelay(1);
		if(j&0x80)
			gpio_set_value_cansleep(spi_mosi, 1);
		else    
			gpio_set_value_cansleep(spi_mosi, 0);

		gpio_set_value_cansleep(spi_sclk, 0);
		k=j<<1;
		udelay(1);
		gpio_set_value_cansleep(spi_sclk, 1); 
		udelay(1);
		j=k;
	}
	
	for(i=0;i<8;i++)
	{
	    	gpio_set_value_cansleep(spi_sclk, 0);
	  	udelay(1);
		if(d&0x80)
			gpio_set_value_cansleep(spi_mosi, 1);
		else    
			gpio_set_value_cansleep(spi_mosi, 0);

		gpio_set_value_cansleep(spi_sclk, 0);
		k=d<<1;
		udelay(1);
		gpio_set_value_cansleep(spi_sclk, 1); 
		udelay(1);
		d=k;
	}
	gpio_set_value_cansleep(spi_cs, 1); 
	udelay(1);
	gpio_set_value_cansleep(spi_sclk, 0);
	udelay(1);
	gpio_set_value_cansleep(spi_mosi, 0);
	udelay(1);
	
	return 0;
}

static __inline__ void Write_REG(unsigned char c1, unsigned char c2, unsigned char d)
{
	WriteREG_command(c1,c2);
	WriteREG_para(d);
}

static __inline void otm_lcd_panel_init(void)
{
 	gpio_set_value_cansleep(gpio_display_reset, 1);
	msleep(20);
	gpio_set_value_cansleep(gpio_display_reset, 0);
	msleep(20);
	gpio_set_value_cansleep(gpio_display_reset, 1);
	msleep(200);

WriteREG_command(0xff,0x00); WriteREG_para(0x80);
WriteREG_command(0xff,0x01); WriteREG_para(0x09); //enable EXTC
WriteREG_command(0xff,0x02); WriteREG_para(0x01);
WriteREG_command(0xff,0x80); WriteREG_para(0x80); //enable Orise mode
WriteREG_command(0xff,0x81); WriteREG_para(0x09);
WriteREG_command(0xff,0x03); WriteREG_para(0x01); //enable SPI+I2C cmd2 read

//gamma DC
WriteREG_command(0xc0,0xb4); WriteREG_para(0x10); //column inversion 	
WriteREG_command(0xC4,0x89); WriteREG_para(0x08); //reg off	
WriteREG_command(0xC0,0xa3); WriteREG_para(0x00); //pre-charge //V02             
WriteREG_command(0xC5,0x82); WriteREG_para(0xA3); //REG-pump23
WriteREG_command(0xC5,0x90); WriteREG_para(0x96); //Pump setting (3x=D6)-->(2x=96)//v02 01/11
WriteREG_command(0xC5,0x91); WriteREG_para(0x87); //Pump setting(VGH/VGL)   
WriteREG_command(0xD8,0x00); WriteREG_para(0x7c); //GVDD=4.5V  73   
WriteREG_command(0xD8,0x01); WriteREG_para(0x7a); //NGVDD=4.5V 71  

//VCOMDC                                                                 
WriteREG_command(0xD9,0x00); WriteREG_para(0x6a); // VCOMDC=   0x5C
msleep(20);

// Positive
WriteREG_command(0xE1,0x00); WriteREG_para(0x00);// 0x09
WriteREG_command(0xE1,0x01); WriteREG_para(0x0a);
WriteREG_command(0xE1,0x02); WriteREG_para(0x0e);
WriteREG_command(0xE1,0x03); WriteREG_para(0x0d);
WriteREG_command(0xE1,0x04); WriteREG_para(0x07);
WriteREG_command(0xE1,0x05); WriteREG_para(0x18);
WriteREG_command(0xE1,0x06); WriteREG_para(0x0d);
WriteREG_command(0xE1,0x07); WriteREG_para(0x0d);
WriteREG_command(0xE1,0x08); WriteREG_para(0x01);
WriteREG_command(0xE1,0x09); WriteREG_para(0x04);
WriteREG_command(0xE1,0x0A); WriteREG_para(0x05);
WriteREG_command(0xE1,0x0B); WriteREG_para(0x06);
WriteREG_command(0xE1,0x0C); WriteREG_para(0x0e);
WriteREG_command(0xE1,0x0D); WriteREG_para(0x25);
WriteREG_command(0xE1,0x0E); WriteREG_para(0x22);
WriteREG_command(0xE1,0x0F); WriteREG_para(0x05);

// Negative
WriteREG_command(0xE2,0x00); WriteREG_para(0x00);//0x09
WriteREG_command(0xE2,0x01); WriteREG_para(0x0a);
WriteREG_command(0xE2,0x02); WriteREG_para(0x0e);
WriteREG_command(0xE2,0x03); WriteREG_para(0x0d);
WriteREG_command(0xE2,0x04); WriteREG_para(0x07);
WriteREG_command(0xE2,0x05); WriteREG_para(0x18);
WriteREG_command(0xE2,0x06); WriteREG_para(0x0d);
WriteREG_command(0xE2,0x07); WriteREG_para(0x0d);
WriteREG_command(0xE2,0x08); WriteREG_para(0x01);
WriteREG_command(0xE2,0x09); WriteREG_para(0x04);
WriteREG_command(0xE2,0x0A); WriteREG_para(0x05);
WriteREG_command(0xE2,0x0B); WriteREG_para(0x06);
WriteREG_command(0xE2,0x0C); WriteREG_para(0x0e);
WriteREG_command(0xE2,0x0D); WriteREG_para(0x25);
WriteREG_command(0xE2,0x0E); WriteREG_para(0x22);
WriteREG_command(0xE2,0x0F); WriteREG_para(0x05);

WriteREG_command(0xC1,0x81); WriteREG_para(0x66);     //Frame rate 65Hz//V02   

// RGB I/F setting VSYNC for OTM8018 0x0e
WriteREG_command(0xC1,0xa1); WriteREG_para(0x08);     //external Vsync,Hsync,DE
WriteREG_command(0xC0,0xa3); WriteREG_para(0x1b);     //pre-charge //V02
WriteREG_command(0xC4,0x81); WriteREG_para(0x83);     //source bias //V02
WriteREG_command(0xC5,0x92); WriteREG_para(0x01);     //Pump45
WriteREG_command(0xC5,0xB1); WriteREG_para(0xA9);     //DC voltage setting ;[0]GVDD output, default: 0xa8

WriteREG_command(0xb2,0x82); WriteREG_para(0x00);    //RGBÐÅºÅŒ«ÐÔ

// CE8x : vst1, vst2, vst3, vst4
WriteREG_command(0xCE,0x80); WriteREG_para(0x85);	
WriteREG_command(0xCE,0x81); WriteREG_para(0x03);	
WriteREG_command(0xCE,0x82); WriteREG_para(0x00);	
WriteREG_command(0xCE,0x83); WriteREG_para(0x84);	
WriteREG_command(0xCE,0x84); WriteREG_para(0x03);	
WriteREG_command(0xCE,0x85); WriteREG_para(0x00);	                               
WriteREG_command(0xCE,0x86); WriteREG_para(0x83);	                               
WriteREG_command(0xCE,0x87); WriteREG_para(0x03);	
WriteREG_command(0xCE,0x88); WriteREG_para(0x00);	
WriteREG_command(0xCE,0x89); WriteREG_para(0x82);	
WriteREG_command(0xCE,0x8a); WriteREG_para(0x03);	
WriteREG_command(0xCE,0x8b); WriteREG_para(0x00);	

//CEAx : clka1, clka2
WriteREG_command(0xCE,0xa0); WriteREG_para(0x38);	
WriteREG_command(0xCE,0xa1); WriteREG_para(0x02);	
WriteREG_command(0xCE,0xa2); WriteREG_para(0x03);	 
WriteREG_command(0xCE,0xa3); WriteREG_para(0x21);	 
WriteREG_command(0xCE,0xa4); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xa5); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xa6); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xa7); WriteREG_para(0x38);	 
WriteREG_command(0xCE,0xa8); WriteREG_para(0x01);	 
WriteREG_command(0xCE,0xa9); WriteREG_para(0x03);	 
WriteREG_command(0xCE,0xaa); WriteREG_para(0x22);	 
WriteREG_command(0xCE,0xab); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xac); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xad); WriteREG_para(0x00);	 
                                                  
//CEBx : clka3, clka4                             
WriteREG_command(0xCE,0xb0); WriteREG_para(0x38);	 
WriteREG_command(0xCE,0xb1); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xb2); WriteREG_para(0x03);	 
WriteREG_command(0xCE,0xb3); WriteREG_para(0x23);	 
WriteREG_command(0xCE,0xb4); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xb5); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xb6); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xb7); WriteREG_para(0x30);	 
WriteREG_command(0xCE,0xb8); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xb9); WriteREG_para(0x03);	 
WriteREG_command(0xCE,0xba); WriteREG_para(0x24);	 
WriteREG_command(0xCE,0xbb); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xbc); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xbd); WriteREG_para(0x00);	 
                                                  
//CECx : clkb1, clkb2                             
WriteREG_command(0xCE,0xc0); WriteREG_para(0x30);	 
WriteREG_command(0xCE,0xc1); WriteREG_para(0x01);	 
WriteREG_command(0xCE,0xc2); WriteREG_para(0x03);	 
WriteREG_command(0xCE,0xc3); WriteREG_para(0x25);	 
WriteREG_command(0xCE,0xc4); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xc5); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xc6); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xc7); WriteREG_para(0x30);	 
WriteREG_command(0xCE,0xc8); WriteREG_para(0x02);	 
WriteREG_command(0xCE,0xc9); WriteREG_para(0x03);	 
WriteREG_command(0xCE,0xca); WriteREG_para(0x26);	 
WriteREG_command(0xCE,0xcb); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xcc); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xcd); WriteREG_para(0x00);	 
                                   	              
//CEDx : clkb3, clkb4                             
WriteREG_command(0xCE,0xd0); WriteREG_para(0x30);	 
WriteREG_command(0xCE,0xd1); WriteREG_para(0x03);	 
WriteREG_command(0xCE,0xd2); WriteREG_para(0x03);	 
WriteREG_command(0xCE,0xd3); WriteREG_para(0x27);	 
WriteREG_command(0xCE,0xd4); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xd5); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xd6); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xd7); WriteREG_para(0x30);	 
WriteREG_command(0xCE,0xd8); WriteREG_para(0x04);	 
WriteREG_command(0xCE,0xd9); WriteREG_para(0x03);	 
WriteREG_command(0xCE,0xda); WriteREG_para(0x28);	 
WriteREG_command(0xCE,0xdb); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xdc); WriteREG_para(0x00);	 
WriteREG_command(0xCE,0xdd); WriteREG_para(0x00);	 
                                                  
//CFCx :                                          
WriteREG_command(0xCF,0xc0); WriteREG_para(0x00);	 
WriteREG_command(0xCF,0xc1); WriteREG_para(0x00);	 
WriteREG_command(0xCF,0xc2); WriteREG_para(0x00);	 
WriteREG_command(0xCF,0xc3); WriteREG_para(0x00);	 
WriteREG_command(0xCF,0xc4); WriteREG_para(0x00);	 
WriteREG_command(0xCF,0xc5); WriteREG_para(0x00);	 
WriteREG_command(0xCF,0xc6); WriteREG_para(0x00);	 
WriteREG_command(0xCF,0xc7); WriteREG_para(0x00);	 
WriteREG_command(0xCF,0xc8); WriteREG_para(0x00);	 
WriteREG_command(0xCF,0xc); WriteREG_para(0x00);	 

//CFDx :
WriteREG_command(0xCF,0xd0);	WriteREG_para(0x00);
// Parameter 1

//--------------------------------------------------------------------------------
//		initial setting 3 < Panel setting >
//--------------------------------------------------------------------------------
// cbcx
WriteREG_command(0xCB,0xc0); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xc1); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xc2); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xc3); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xc4); WriteREG_para(0x04);	              
WriteREG_command(0xCB,0xc5); WriteREG_para(0x04);	              
WriteREG_command(0xCB,0xc6); WriteREG_para(0x04);	              
WriteREG_command(0xCB,0xc7); WriteREG_para(0x04);	              
WriteREG_command(0xCB,0xc8); WriteREG_para(0x04);	              
WriteREG_command(0xCB,0xc9); WriteREG_para(0x04);	              
WriteREG_command(0xCB,0xca); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xcb); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xcc); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xcd); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xce); WriteREG_para(0x00);	              
                                                  
// cbdx                                           
WriteREG_command(0xCB,0xd0); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xd1); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xd2); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xd3); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xd4); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xd5); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xd6); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xd7); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xd8); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xd9); WriteREG_para(0x04);	              
WriteREG_command(0xCB,0xda); WriteREG_para(0x04);	              
WriteREG_command(0xCB,0xdb); WriteREG_para(0x04);	              
WriteREG_command(0xCB,0xdc); WriteREG_para(0x04);	              
WriteREG_command(0xCB,0xdd); WriteREG_para(0x04);	              
WriteREG_command(0xCB,0xde); WriteREG_para(0x04);	              
                                                  
// cbex                                           
WriteREG_command(0xCB,0xe0); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xe1); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xe2); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xe3); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xe4); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xe5); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xe6); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xe7); WriteREG_para(0x00);	              
WriteREG_command(0xCB,0xe8); WriteREG_para(0x00);	                                                                                      
WriteREG_command(0xCB,0xe9); WriteREG_para(0x00);	                                                                                      
                                                  
// cc8x                                           
WriteREG_command(0xCC,0x80); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0x81); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0x82); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0x83); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0x84); WriteREG_para(0x0C);	              
WriteREG_command(0xCC,0x85); WriteREG_para(0x0A);	              
WriteREG_command(0xCC,0x86); WriteREG_para(0x10);	              
WriteREG_command(0xCC,0x87); WriteREG_para(0x0E);	              
WriteREG_command(0xCC,0x88); WriteREG_para(0x03);	              
WriteREG_command(0xCC,0x89); WriteREG_para(0x04);	              
                                                  
// cc9x                                           
WriteREG_command(0xCC,0x90); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0x91); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0x92); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0x93); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0x94); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0x95); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0x96); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0x97); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0x98); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0x99); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0x9a); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0x9b); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0x9c); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0x9d); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0x9e); WriteREG_para(0x0B);	              
                                                  
// ccax                                           
WriteREG_command(0xCC,0xa0); WriteREG_para(0x09);	              
WriteREG_command(0xCC,0xa1); WriteREG_para(0x0F);	              
WriteREG_command(0xCC,0xa2); WriteREG_para(0x0D);	              
WriteREG_command(0xCC,0xa3); WriteREG_para(0x01);	              
WriteREG_command(0xCC,0xa4); WriteREG_para(0x02);	              
WriteREG_command(0xCC,0xa5); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xa6); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xa7); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xa8); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xa9); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xaa); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xab); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xac); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xad); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xae); WriteREG_para(0x00);	              
                                                  
// ccbx                                           
WriteREG_command(0xCC,0xb0); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xb1); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xb2); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xb3); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xb4); WriteREG_para(0x0D);	              
WriteREG_command(0xCC,0xb5); WriteREG_para(0x0F);	              
WriteREG_command(0xCC,0xb6); WriteREG_para(0x09);	              
WriteREG_command(0xCC,0xb7); WriteREG_para(0x0B);	              
WriteREG_command(0xCC,0xb8); WriteREG_para(0x02);	              
WriteREG_command(0xCC,0xb9); WriteREG_para(0x01);	              
                                                  
// cccx                                           
WriteREG_command(0xCC,0xc0); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xc1); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xc2); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xc3); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xc4); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xc5); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xc6); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xc7); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xc8); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xc9); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xca); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xcb); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xcc); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xcd); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xce); WriteREG_para(0x0E);	              
                                                  
// ccdx                                           
WriteREG_command(0xCC,0xd0); WriteREG_para(0x10);	              
WriteREG_command(0xCC,0xd1); WriteREG_para(0x0A);	              
WriteREG_command(0xCC,0xd2); WriteREG_para(0x0C);	              
WriteREG_command(0xCC,0xd3); WriteREG_para(0x04);	              
WriteREG_command(0xCC,0xd4); WriteREG_para(0x03);	              
WriteREG_command(0xCC,0xd5); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xd6); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xd7); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xd8); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xd9); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xda); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xdb); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xdc); WriteREG_para(0x00);	              
WriteREG_command(0xCC,0xdd); WriteREG_para(0x00);	
WriteREG_command(0xCC,0xde); WriteREG_para(0x00);	

///=============================
WriteREG_command(0xb2,0x82); WriteREG_para(0xe0);   
WriteREG_command(0x3A,0x00); WriteREG_para(0x77);     //MCU 16bits D[17:0]
//WriteREG_command(0x36,0x00); WriteREG_para(0xc0);     //MCU 16bits D[17:0]
WriteREG_command(0x11,0x00);
msleep(120);	
WriteREG_command(0x29,0x00);
msleep(20);
//WriteREG_command(0x2c,0x00);
}

static __inline__ int otm_set_backlight(void)
{
	Write_REG(0x51,0x00,0xFF);
	Write_REG(0x53,0x00,0x24);
	Write_REG(0x55,0x00,0x00);

	return 0;
}

static void spi_pin_assign(void)
{
	/* Setting the Default GPIO's */
	spi_mosi	= *(lcdc_pdata->gpio_num);
	spi_sclk	= *(lcdc_pdata->gpio_num + 1);
	spi_cs		= *(lcdc_pdata->gpio_num + 2);
	gpio_backlight_en = *(lcdc_pdata->gpio_num + 3);
	gpio_display_reset = *(lcdc_pdata->gpio_num + 4);
}

static void lcdc_disp_powerup(void)
{
	if (!lcdc_state.disp_powered_up && !lcdc_state.display_on) {
		/* Reset the hardware first */
		/* Include DAC power up implementation here */
	      lcdc_state.disp_powered_up = TRUE;
	}
}

static void lcdc_disp_reginit(void)
{
	if (lcdc_state.disp_powered_up && !lcdc_state.display_on) {
		gpio_set_value_cansleep(spi_cs, 1);	/* cs high */
		msleep(10);
		otm_lcd_panel_init();
		//otm_set_backlight();
		lcdc_state.display_on = TRUE;
	}
}

static int lcdc_panel_on(struct platform_device *pdev)
{
//luke: add for continue
	struct msm_fb_data_type *mfd = platform_get_drvdata(pdev);
	if((lcdc_pdata->cont_splash_enabled)&&(!mfd->cont_splash_done)){
		printk("luke: %s cont_splash_enabled! %d \n",__func__,__LINE__);
        	mfd->cont_splash_done = 1;
        	//return 0;
	}
//luke: add end

	if (lcdc_pdata->panel_config_gpio){
		lcdc_pdata->panel_config_gpio(1);
	}
printk("luke: %s: %d\n",__func__,__LINE__);
	lcdc_disp_powerup();
	lcdc_disp_reginit();	
	lcdc_state.disp_initialized = TRUE;

	return 0;
}

static int lcdc_panel_off(struct platform_device *pdev)
{
	if (lcdc_state.disp_powered_up && lcdc_state.display_on) {
printk("luke: %s: %d\n",__func__,__LINE__);
		WriteREG_command(0x28,0x00);    	
		WriteREG_command(0x10,0x00);
		msleep(50);
		/* Main panel power off (Pull down reset) */
		gpio_set_value_cansleep(gpio_display_reset, 0);
		lcdc_state.display_on = FALSE;
		lcdc_state.disp_initialized = FALSE;
	}

	return 0;
}

#define LCD_DRIVER_BL 1

static void lcdc_set_backlight(struct msm_fb_data_type *mfd)
{
    if(!LCD_DRIVER_BL)
    {
	int step = 0, i = 0;
	int bl_level = mfd->bl_level;
	/* real backlight level, 1 - max, 16 - min, 32 - off */
	bl_level = 32 - bl_level;
	//printk("%s: prev_bl = %d, bl_level = %d\n", __func__, prev_bl, bl_level);
	if (bl_level > prev_bl) {
		step = bl_level - prev_bl;
		if (bl_level == 32) {
			step--;
		}
	} else if (bl_level < prev_bl) {
		step = bl_level + 31 - prev_bl;
	} else {
		return;
	}

	if (bl_level == 32) {
		/* turn off backlight */
		gpio_set_value_cansleep(gpio_backlight_en, 0);
	} else {
		if (prev_bl == 32) {
			/* turn on backlight */
			gpio_set_value_cansleep(gpio_backlight_en, 1);
			udelay(30);
		}

		/* adjust backlight level */
		for (i = 0; i < step; i++) {
			gpio_set_value_cansleep(gpio_backlight_en, 0);
			udelay(1);
			gpio_set_value_cansleep(gpio_backlight_en, 1);
			udelay(1);
		}
	}
	msleep(1);
	prev_bl = bl_level;
    }else {
	int bl_level = mfd->bl_level;
	//int bl_level = 200;
	gpio_set_value_cansleep(spi_cs, 0);
	Write_REG(0x51,0x00,bl_level);		
	gpio_set_value_cansleep(spi_mosi, 1);
	gpio_set_value_cansleep(spi_cs, 1);
	udelay(10);
	Write_REG(0x53,0x00,0x24);
	Write_REG(0x55,0x00,0x00);

	if(0 == bl_level){
		//Start===Allen
		keyboard_led_light(KEY_LED_USER_LCD, 0);
		//End===Allen
	}else{
		//Start===Allen
		keyboard_led_light(KEY_LED_USER_LCD, 1);
		//End===Allen
	}    
    }
    return;
}

static int __devinit lcdc_probe(struct platform_device *pdev)
{
	printk("%s\n",__func__);
	if (pdev->id == 0) {
		lcdc_pdata = pdev->dev.platform_data;
		spi_pin_assign();
		return 0;
	}
	msm_fb_add_device(pdev);
	return 0;
}

static struct platform_driver this_driver = {
	.probe  = lcdc_probe,
	.driver = {
		.name   = "lcdc_dnj_otm8018b_wvga_pt", 
	},
};

static struct msm_fb_panel_data lcdc_panel_data = {
	.on = lcdc_panel_on,
	.off = lcdc_panel_off,
	.set_backlight = lcdc_set_backlight,
};

static struct platform_device this_device = {
	.name   = "lcdc_dnj_otm8018b_wvga_pt",
	.id	= 1,
	.dev	= {
		.platform_data = &lcdc_panel_data,
	}
};

static int __init lcdc_byd_panel_init(void)
{

	int ret;
	struct msm_panel_info *pinfo;
	printk("%s\n",__func__);

#ifdef CONFIG_FB_MSM_LCDC_AUTO_DETECT
	if (msm_fb_detect_client("lcdc_dnj_otm8018b_wvga_pt")) {
		return 0;
	}
#endif	
	ret = platform_driver_register(&this_driver);
	if (ret)
		return ret;

	pinfo = &lcdc_panel_data.panel_info;
	pinfo->xres = 480;
	pinfo->yres = 800;
	MSM_FB_SINGLE_MODE_PANEL(pinfo);
	pinfo->type = LCDC_PANEL;
	pinfo->pdest = DISPLAY_1;
	pinfo->wait_cycle = 0;
	pinfo->bpp = 24;
	pinfo->fb_num = 2;
	/* 10Mhz mdp_lcdc_pclk and mdp_lcdc_pad_pcl */
	pinfo->clk_rate = 24500000;
	pinfo->bl_max = 255;
	pinfo->bl_min = 1;

	pinfo->lcdc.h_back_porch = 20;		/* hsw = 8 + hbp=16 */
	pinfo->lcdc.h_front_porch = 20;
	pinfo->lcdc.h_pulse_width = 2;
	pinfo->lcdc.v_back_porch = 20;		/* vsw=1 + vbp = 7 */
	pinfo->lcdc.v_front_porch = 20;
	pinfo->lcdc.v_pulse_width = 10;

	pinfo->lcdc.border_clr = 0;			/* blk */
	pinfo->lcdc.underflow_clr = 0xff;	/* blue */
	pinfo->lcdc.hsync_skew = 0;
	ret = platform_device_register(&this_device);
	if (ret) {
		printk(KERN_ERR "%s not able to register the device\n",
			 __func__);
		goto fail_driver;
	}

	return 0;

fail_driver:
	platform_driver_unregister(&this_driver);
	return ret;
}

device_initcall(lcdc_byd_panel_init);
