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
#if 0
static __inline__ unsigned char Read_REG(void)
{
   unsigned char i,j,k,data;

	j=0xC0;
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
         gpio_direction_input(spi_mosi);
	for(i=0;i<8;i++)
	{
	   	gpio_set_value_cansleep(spi_sclk, 0);
	    	udelay(1);
		j=gpio_get_value(spi_mosi);
		gpio_set_value_cansleep(spi_sclk, 0);
		udelay(1);
		gpio_set_value_cansleep(spi_sclk, 1); 
		udelay(1);
	}
	gpio_set_value_cansleep(spi_cs, 1);
        gpio_direction_output(spi_mosi,0);

	j=0xC0;
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

		udelay(1);
		gpio_set_value_cansleep(spi_sclk, 0);
		k=j<<1;
		gpio_set_value_cansleep(spi_sclk, 1); 
		        udelay(1);
		j=k;
	    
	}
        gpio_direction_input(spi_mosi);
	for(i=0;i<8;i++)
	{
		gpio_set_value_cansleep(spi_sclk, 0);
	   	udelay(1);
		k=gpio_get_value(spi_mosi);
		data |= (k<<(7-i));
		udelay(1);
		gpio_set_value_cansleep(spi_sclk, 1); 
		udelay(1);
	}
	gpio_set_value_cansleep(spi_cs, 1); 
        gpio_direction_output(spi_mosi,0);
	udelay(1);
	gpio_set_value_cansleep(spi_sclk, 0);
	udelay(1);
	gpio_set_value_cansleep(spi_mosi, 0);
	udelay(1);
	
	return data;
}
#endif
static __inline__ void Write_REG(unsigned char c1, unsigned char c2, unsigned char d)
{
	WriteREG_command(c1,c2);
	WriteREG_para(d);
}
static int flag = 1;
static __inline void otm_lcd_panel_init(void)
{

	if(flag){
 	gpio_set_value_cansleep(gpio_display_reset, 1);
	msleep(8);
	gpio_set_value_cansleep(gpio_display_reset, 0);
	msleep(12);
	gpio_set_value_cansleep(gpio_display_reset, 1);
	msleep(150);
	WriteREG_command(0xFF,0x00);  WriteREG_para(0x80);
	WriteREG_command(0xFF,0x01);  WriteREG_para(0x09);
	WriteREG_command(0xFF,0x02);  WriteREG_para(0x01);
	WriteREG_command(0xFF,0x80);  WriteREG_para(0x80);
	WriteREG_command(0xFF,0x81);  WriteREG_para(0x09);
	WriteREG_command(0xFF,0x03);  WriteREG_para(0x01);
	WriteREG_command(0xC0,0xB4);  WriteREG_para(0x50);//
	WriteREG_command(0xC5,0x92);  WriteREG_para(0x01);
	WriteREG_command(0xC5,0x90);  WriteREG_para(0x96);
	WriteREG_command(0xC5,0x95);  WriteREG_para(0x34);
	WriteREG_command(0xC5,0x94);  WriteREG_para(0x33);
	WriteREG_command(0xC5,0x82);  WriteREG_para(0x83);
	WriteREG_command(0xC5,0xB1);  WriteREG_para(0xA9);
	WriteREG_command(0xC5,0x91);  WriteREG_para(0x3C);
	WriteREG_command(0xD8,0x00);  WriteREG_para(0x6F);
	WriteREG_command(0xD8,0x01);  WriteREG_para(0x6F);
	WriteREG_command(0xC1,0x81);  WriteREG_para(0x66);//
	WriteREG_command(0xD9,0x00);  WriteREG_para(0x4E);//57
	WriteREG_command(0xC1,0xA0);  WriteREG_para(0xFA);
	WriteREG_command(0xF5,0xB2);  WriteREG_para(0x15);
	WriteREG_command(0xF5,0xB3);  WriteREG_para(0x00);
	WriteREG_command(0xF5,0xB4);  WriteREG_para(0x15);
	WriteREG_command(0xF5,0xB5);  WriteREG_para(0x00);
	WriteREG_command(0xC5,0x93);  WriteREG_para(0x03);
	WriteREG_command(0xC5,0x96);  WriteREG_para(0x23);
	WriteREG_command(0xC4,0x81);  WriteREG_para(0x81);

	WriteREG_command(0xC4,0x89);  WriteREG_para(0x08);//
	WriteREG_command(0xC0,0xA3);  WriteREG_para(0x00);//
	WriteREG_command(0xB2,0x82);  WriteREG_para(0xC0);////

	WriteREG_command(0xB3,0xA6);  WriteREG_para(0x20);
	WriteREG_command(0xB3,0xA7);  WriteREG_para(0x01);
	WriteREG_command(0xC0,0x90);  WriteREG_para(0x00);
	WriteREG_command(0xC0,0x91);  WriteREG_para(0x44);
	WriteREG_command(0xC0,0x92);  WriteREG_para(0x00);
	WriteREG_command(0xC0,0x93);  WriteREG_para(0x00);
	WriteREG_command(0xC0,0x94);  WriteREG_para(0x00);
	WriteREG_command(0xC0,0x95);  WriteREG_para(0x03);
	WriteREG_command(0xC1,0xA6);  WriteREG_para(0x00);
	WriteREG_command(0xC1,0xA7);  WriteREG_para(0x00);
	WriteREG_command(0xC1,0xA8);  WriteREG_para(0x00);
	WriteREG_command(0xCE,0x80);  WriteREG_para(0x86);
	WriteREG_command(0xCE,0x81);  WriteREG_para(0x01);
	WriteREG_command(0xCE,0x82);  WriteREG_para(0x00);
	WriteREG_command(0xCE,0x83);  WriteREG_para(0x85);
	WriteREG_command(0xCE,0x84);  WriteREG_para(0x01);
	WriteREG_command(0xCE,0x85);  WriteREG_para(0x00);
	WriteREG_command(0xCE,0x86);  WriteREG_para(0x00);
	WriteREG_command(0xCE,0x87);  WriteREG_para(0x00);
	WriteREG_command(0xCE,0x88);  WriteREG_para(0x00);
	WriteREG_command(0xCE,0x89);  WriteREG_para(0x00);
	WriteREG_command(0xCE,0x8A);  WriteREG_para(0x00);
	WriteREG_command(0xCE,0x8B);  WriteREG_para(0x00);
	WriteREG_command(0xCE,0xA0);  WriteREG_para(0x18);
	WriteREG_command(0xCE,0xA1);  WriteREG_para(0x05);
	WriteREG_command(0xCE,0xA2);  WriteREG_para(0x03);
	WriteREG_command(0xCE,0xA3);  WriteREG_para(0x39);
	WriteREG_command(0xCE,0xA4);  WriteREG_para(0x00);
	WriteREG_command(0xCE,0xA5);  WriteREG_para(0x00);
	WriteREG_command(0xCE,0xA6);  WriteREG_para(0x00);
	WriteREG_command(0xCE,0xA7);  WriteREG_para(0x18);
	WriteREG_command(0xCE,0xA8);  WriteREG_para(0x04);
	WriteREG_command(0xCE,0xA9);  WriteREG_para(0x03);
	WriteREG_command(0xCE,0xAA);  WriteREG_para(0x3A);
	WriteREG_command(0xCE,0xAB);  WriteREG_para(0x00);
	WriteREG_command(0xCE,0xAC);  WriteREG_para(0x00);
	WriteREG_command(0xCE,0xAD);  WriteREG_para(0x00);
	WriteREG_command(0xCE,0xB0);  WriteREG_para(0x18);
	WriteREG_command(0xCE,0xB1);  WriteREG_para(0x03);
	WriteREG_command(0xCE,0xB2);  WriteREG_para(0x03);
	WriteREG_command(0xCE,0xB3);  WriteREG_para(0x3B);
	WriteREG_command(0xCE,0xB4);  WriteREG_para(0x00);
	WriteREG_command(0xCE,0xB5);  WriteREG_para(0x00);
	WriteREG_command(0xCE,0xB6);  WriteREG_para(0x00);
	WriteREG_command(0xCE,0xB7);  WriteREG_para(0x18);
	WriteREG_command(0xCE,0xB8);  WriteREG_para(0x02);
	WriteREG_command(0xCE,0xB9);  WriteREG_para(0x03);
	WriteREG_command(0xCE,0xBA);  WriteREG_para(0x3C);
	WriteREG_command(0xCE,0xBB);  WriteREG_para(0x00);
	WriteREG_command(0xCE,0xBC);  WriteREG_para(0x00);
	WriteREG_command(0xCE,0xBD);  WriteREG_para(0x00);
	WriteREG_command(0xCF,0xC0);  WriteREG_para(0x01);
	WriteREG_command(0xCF,0xC1);  WriteREG_para(0x01);
	WriteREG_command(0xCF,0xC2);  WriteREG_para(0x20);
	WriteREG_command(0xCF,0xC3);  WriteREG_para(0x20);
	WriteREG_command(0xCF,0xC4);  WriteREG_para(0x00);
	WriteREG_command(0xCF,0xC5);  WriteREG_para(0x00);
	WriteREG_command(0xCF,0xC6);  WriteREG_para(0x01);
	WriteREG_command(0xCF,0xC7);  WriteREG_para(0x00);
	WriteREG_command(0xCF,0xC8);  WriteREG_para(0x00);
	WriteREG_command(0xCF,0xC9);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xC0);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xC1);  WriteREG_para(0x04);
	WriteREG_command(0xCB,0xC2);  WriteREG_para(0x04);
	WriteREG_command(0xCB,0xC3);  WriteREG_para(0x04);
	WriteREG_command(0xCB,0xC4);  WriteREG_para(0x04);
	WriteREG_command(0xCB,0xC5);  WriteREG_para(0x04);
	WriteREG_command(0xCB,0xC6);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xC7);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xC8);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xC9);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xCA);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xCB);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xCC);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xCD);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xCE);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xD0);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xD1);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xD2);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xD3);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xD4);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xD5);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xD6);  WriteREG_para(0x04);
	WriteREG_command(0xCB,0xD7);  WriteREG_para(0x04);
	WriteREG_command(0xCB,0xD8);  WriteREG_para(0x04);
	WriteREG_command(0xCB,0xD9);  WriteREG_para(0x04);
	WriteREG_command(0xCB,0xDA);  WriteREG_para(0x04);
	WriteREG_command(0xCB,0xDB);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xDC);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xDD);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xDE);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xE0);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xE1);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xE2);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xE3);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xE4);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xE5);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xE6);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xE7);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xE8);  WriteREG_para(0x00);
	WriteREG_command(0xCB,0xE9);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0x80);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0x81);  WriteREG_para(0x26);
	WriteREG_command(0xCC,0x82);  WriteREG_para(0x09);
	WriteREG_command(0xCC,0x83);  WriteREG_para(0x0B);
	WriteREG_command(0xCC,0x84);  WriteREG_para(0x01);
	WriteREG_command(0xCC,0x85);  WriteREG_para(0x25);
	WriteREG_command(0xCC,0x86);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0x87);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0x88);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0x89);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0x90);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0x91);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0x92);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0x93);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0x94);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0x95);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0x96);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0x97);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0x98);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0x99);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0x9A);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0x9B);  WriteREG_para(0x26);
	WriteREG_command(0xCC,0x9C);  WriteREG_para(0x0A);
	WriteREG_command(0xCC,0x9D);  WriteREG_para(0x0C);
	WriteREG_command(0xCC,0x9E);  WriteREG_para(0x02);
	WriteREG_command(0xCC,0xA0);  WriteREG_para(0x25);
	WriteREG_command(0xCC,0xA1);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xA2);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xA3);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xA4);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xA5);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xA6);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xA7);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xA8);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xA9);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xAA);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xAB);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xAC);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xAD);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xAE);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xB0);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xB1);  WriteREG_para(0x25);
	WriteREG_command(0xCC,0xB2);  WriteREG_para(0x0C);
	WriteREG_command(0xCC,0xB3);  WriteREG_para(0x0A);
	WriteREG_command(0xCC,0xB4);  WriteREG_para(0x02);
	WriteREG_command(0xCC,0xB5);  WriteREG_para(0x26);
	WriteREG_command(0xCC,0xB6);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xB7);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xB8);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xB9);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xC0);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xC1);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xC2);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xC3);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xC4);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xC5);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xC6);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xC7);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xC8);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xC9);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xCA);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xCB);  WriteREG_para(0x25);
	WriteREG_command(0xCC,0xCC);  WriteREG_para(0x0B);
	WriteREG_command(0xCC,0xCD);  WriteREG_para(0x09);
	WriteREG_command(0xCC,0xCE);  WriteREG_para(0x01);
	WriteREG_command(0xCC,0xD0);  WriteREG_para(0x26);
	WriteREG_command(0xCC,0xD1);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xD2);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xD3);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xD4);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xD5);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xD6);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xD7);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xD8);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xD9);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xDA);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xDB);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xDC);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xDD);  WriteREG_para(0x00);
	WriteREG_command(0xCC,0xDE);  WriteREG_para(0x00);

	WriteREG_command(0xE1,0x00);  WriteREG_para(0x00);
	WriteREG_command(0xE1,0x01);  WriteREG_para(0x09);
	WriteREG_command(0xE1,0x02);  WriteREG_para(0x0F);
	WriteREG_command(0xE1,0x03);  WriteREG_para(0x0E);
	WriteREG_command(0xE1,0x04);  WriteREG_para(0x07);
	WriteREG_command(0xE1,0x05);  WriteREG_para(0x10);
	WriteREG_command(0xE1,0x06);  WriteREG_para(0x0B);
	WriteREG_command(0xE1,0x07);  WriteREG_para(0x0A);
	WriteREG_command(0xE1,0x08);  WriteREG_para(0x04);
	WriteREG_command(0xE1,0x09);  WriteREG_para(0x07);
	WriteREG_command(0xE1,0x0A);  WriteREG_para(0x0B);
	WriteREG_command(0xE1,0x0B);  WriteREG_para(0x08);
	WriteREG_command(0xE1,0x0C);  WriteREG_para(0x0F);
	WriteREG_command(0xE1,0x0D);  WriteREG_para(0x10);
	WriteREG_command(0xE1,0x0E);  WriteREG_para(0x0A);
	WriteREG_command(0xE1,0x0F);  WriteREG_para(0x01);

	WriteREG_command(0xE2,0x00);  WriteREG_para(0x00);
	WriteREG_command(0xE2,0x01);  WriteREG_para(0x09);
	WriteREG_command(0xE2,0x02);  WriteREG_para(0x0F);
	WriteREG_command(0xE2,0x03);  WriteREG_para(0x0E);
	WriteREG_command(0xE2,0x04);  WriteREG_para(0x07);
	WriteREG_command(0xE2,0x05);  WriteREG_para(0x10);
	WriteREG_command(0xE2,0x06);  WriteREG_para(0x0B);
	WriteREG_command(0xE2,0x07);  WriteREG_para(0x0A);
	WriteREG_command(0xE2,0x08);  WriteREG_para(0x04);
	WriteREG_command(0xE2,0x09);  WriteREG_para(0x07);
	WriteREG_command(0xE2,0x0A);  WriteREG_para(0x0B);
	WriteREG_command(0xE2,0x0B);  WriteREG_para(0x08);
	WriteREG_command(0xE2,0x0C);  WriteREG_para(0x0F);
	WriteREG_command(0xE2,0x0D);  WriteREG_para(0x10);
	WriteREG_command(0xE2,0x0E);  WriteREG_para(0x0A);
	WriteREG_command(0xE2,0x0F);  WriteREG_para(0x01);

	WriteREG_command(0xD6,0x80);  WriteREG_para(0x18);//

	WriteREG_command(0xFF,0x00);  WriteREG_para(0xFF);
	WriteREG_command(0xFF,0x01);  WriteREG_para(0xFF);
	WriteREG_command(0xFF,0x02);  WriteREG_para(0xFF);

	WriteREG_command(0x3A,0x00);  WriteREG_para(0x77); 
	WriteREG_command(0x11,0x00);
	msleep(50);
	WriteREG_command(0x29,0x00);
	flag = 0;
	}else{
	WriteREG_command(0x29,0x00);
	msleep(10);
	WriteREG_command(0x11,0x00);
	}
	lcdc_state.display_on = TRUE;

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
	//if((lcdc_pdata->cont_splash_enabled)&&(!mfd->cont_splash_done)){
		printk("luke: %s cont_splash_enabled! %d \n",__func__,__LINE__);
        	mfd->cont_splash_done = 1;
        	//return 0;
	//}
//luke: add end
	if (lcdc_pdata->panel_config_gpio){
		lcdc_pdata->panel_config_gpio(1);
	}
	lcdc_disp_powerup();
	lcdc_disp_reginit();	
	lcdc_state.disp_initialized = TRUE;

	return 0;
}

static int lcdc_panel_off(struct platform_device *pdev)
{
	if (lcdc_state.disp_powered_up && lcdc_state.display_on) {

		/* Main panel power off (Pull down reset) */
		//gpio_set_value_cansleep(gpio_display_reset, 0);
		WriteREG_command(0x28,0x00);
		msleep(10);
		WriteREG_command(0x10,0x00);
		msleep(10);
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
		.name   = "lcdc_dnj_otm8018b_wvga_ips", 
	},
};

static struct msm_fb_panel_data lcdc_panel_data = {
	.on = lcdc_panel_on,
	.off = lcdc_panel_off,
	.set_backlight = lcdc_set_backlight,
};

static struct platform_device this_device = {
	.name   = "lcdc_dnj_otm8018b_wvga_ips",
	.id	= 1,
	.dev	= {
		.platform_data = &lcdc_panel_data,
	}
};

static int __init lcdc_dijing_panel_ips_init(void)
{

	int ret;
	struct msm_panel_info *pinfo;
	printk("%s\n",__func__);

#ifdef CONFIG_FB_MSM_LCDC_AUTO_DETECT
	if (msm_fb_detect_client("lcdc_dnj_otm8018b_wvga_ips")) {
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

device_initcall(lcdc_dijing_panel_ips_init);
