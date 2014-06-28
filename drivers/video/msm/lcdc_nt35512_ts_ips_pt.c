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
	boolean initialized;
};

static struct lcdc_state_type lcdc_state = { 0 };
static struct msm_panel_common_pdata *lcdc_pdata;

static void SPI_WriteCommand( int  Command)
{
	unsigned char i,count1, count2,count3,count4;
	count1= Command>>8;
	count2= Command;
	count3=0x20;//00100000   //ÐŽÃüÁîžßÎ»
	count4=0x00;//00000000   //ÐŽÃüÁîµÍÎ»======ŸßÌåÇë¿ŽICµÄDatasheet
	spi_cs=0;
	for(i=0;i<8;i++)//
	{
	  spi_sclk=0;
	  if (count3 & 0x80) spi_mosi=1;
	  else             spi_mosi=0;
	  spi_sclk=1;
	  count3<<=1;
	}

    for(i=0;i<8;i++)//ÐŽÃüÁîžßÎ»
	{
	  spi_sclk=0;
	  if (count1 & 0x80) spi_mosi=1;
	  else             spi_mosi=0;
	  spi_sclk=1;
	  count1<<=1;
	}
 
	for(i=0;i<8;i++)
	{
	  spi_sclk=0;
	  if (count4 & 0x80) spi_mosi=1;
	  else             spi_mosi=0;
	  spi_sclk=1;
	  count4<<=1;
	}
  for(i=0;i<8;i++)//ÐŽÃüÁîµÍÎ»
	{
	  spi_sclk=0;
	  if (count2 & 0x80) spi_mosi=1;
	  else             spi_mosi=0;
	  spi_sclk=1;
	  count2<<=1;
	}
	spi_cs=1;
}

static void SPI_WriteParameter(char DH)
{
unsigned char i, count1, count2,count3,count4;
count1=DH>>8;
count2=DH;
count3=0x60;//ÐŽÊýŸÝžßÎ»
count4=0x40;//ÐŽÊýŸÝµÍÎ»

 spi_cs=0;

/*             //ÒòÎªÊýŸÝµÄžßÎ»»ù±ŸÊÇ²»ÓÃµÄ£¬¿ÉÒÔ²»Ž«žßÎ»£¬Ö±œÓŽ«µÍÎ»
    for(i=0;i<8;i++)
	{
	  spi_sclk=0;
	  if (count3 & 0x80) spi_mosi=1;
	  else             spi_mosi=0;
	  spi_sclk=1;
	  count3<<=1;
	}

   for(i=0;i<8;i++)
	{
	  spi_sclk=0;
	  if (count1 & 0x80) spi_mosi=1;
	  else             spi_mosi=0;
	  spi_sclk=1;
	  count1<<=1;
	}
	*/

 
    for(i=0;i<8;i++)
	{
	  spi_sclk=0;
	  if (count4 & 0x80) spi_mosi=1;
	  else             spi_mosi=0;
	  spi_sclk=1;
	  count4<<=1;
	}

  for(i=0;i<8;i++)
	{
	  spi_sclk=0;
	  if (count2 & 0x80) spi_mosi=1;
	  else             spi_mosi=0;
	  spi_sclk=1;
	  count2<<=1;
	}
	spi_cs=1;

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

#if 0
static void lcdc_disp_powerup(void)
{
	if (!lcdc_state.disp_powered_up && !lcdc_state.display_on) {
		/* Reset the hardware first */
		/* Include DAC power up implementation here */
	      lcdc_state.disp_powered_up = TRUE;
	}
}
#endif 

static void lcdc_ts_disp_reginit(void)
{
		gpio_set_value_cansleep(spi_cs, 1);	/* cs high */
		msleep(10);
		gpio_set_value_cansleep(gpio_display_reset, 1);
		msleep(10);
		gpio_set_value_cansleep(gpio_display_reset, 0);
		msleep(15);
		gpio_set_value_cansleep(gpio_display_reset, 1);
		msleep(100);

//==============================================================
// VCI=2.8V, IOVCC=2.8V
//== nt35510 HSD40" RGB initial code == //
//Enable Page1 
SPI_WriteCommand(0xF000);    SPI_WriteParameter(0x55); 
SPI_WriteCommand(0xF001);    SPI_WriteParameter(0xAA); 
SPI_WriteCommand(0xF002);    SPI_WriteParameter(0x52); 
SPI_WriteCommand(0xF003);    SPI_WriteParameter(0x08);
SPI_WriteCommand(0xF004);    SPI_WriteParameter(0x01); 
 
// AVDD: manual);   
SPI_WriteCommand(0xB600);    SPI_WriteParameter(0x34); 
SPI_WriteCommand(0xB601);    SPI_WriteParameter(0x34); 
SPI_WriteCommand(0xB602);    SPI_WriteParameter(0x34); 
SPI_WriteCommand(0xB000);    SPI_WriteParameter(0x09); 
SPI_WriteCommand(0xB001);    SPI_WriteParameter(0x09); 
SPI_WriteCommand(0xB002);    SPI_WriteParameter(0x09); 
 
// AVEE: manual); 
SPI_WriteCommand(0xB700);    SPI_WriteParameter(0x24); 
SPI_WriteCommand(0xB701);    SPI_WriteParameter(0x24); 
SPI_WriteCommand(0xB702);    SPI_WriteParameter(0x24); 
SPI_WriteCommand(0xB100);    SPI_WriteParameter(0x09); 
SPI_WriteCommand(0xB101);    SPI_WriteParameter(0x09); 
SPI_WriteCommand(0xB102);    SPI_WriteParameter(0x09); 
 
//Power Control for VCL 
SPI_WriteCommand(0xB800);    SPI_WriteParameter(0x34); 
SPI_WriteCommand(0xB200);    SPI_WriteParameter(0x00);
 
// VGH: Clamp Enable);    SPI_WriteParameter( 
SPI_WriteCommand(0xB900);    SPI_WriteParameter(0x24); 
SPI_WriteCommand(0xB901);    SPI_WriteParameter(0x24); 
SPI_WriteCommand(0xB902);    SPI_WriteParameter(0x24); 
SPI_WriteCommand(0xB300);    SPI_WriteParameter(0x05); 
SPI_WriteCommand(0xB301);    SPI_WriteParameter(0x05); 
SPI_WriteCommand(0xB302);    SPI_WriteParameter(0x05); 

SPI_WriteCommand(0xBF00);    SPI_WriteParameter(0x01); 
 
// VGL(LVGL) 
SPI_WriteCommand(0xBA00);    SPI_WriteParameter(0x34); 
SPI_WriteCommand(0xBA01);    SPI_WriteParameter(0x34); 
SPI_WriteCommand(0xBA02);    SPI_WriteParameter(0x34); 
 
// VGL_REG(VGLO) 
SPI_WriteCommand(0xB500);    SPI_WriteParameter(0x0B); 
SPI_WriteCommand(0xB501);    SPI_WriteParameter(0x0B); 
SPI_WriteCommand(0xB502);    SPI_WriteParameter(0x0B); 
 
// VGMP/VGSP 
SPI_WriteCommand(0xBC00);    SPI_WriteParameter(0X00); 
SPI_WriteCommand(0xBC01);    SPI_WriteParameter(0xA3); 
SPI_WriteCommand(0xBC02);    SPI_WriteParameter(0X00); 
 
//VGMN/VGSN 
SPI_WriteCommand(0xBD00);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xBD01);    SPI_WriteParameter(0xA3); 
SPI_WriteCommand(0xBD02);    SPI_WriteParameter(0x00); 
 
// VCOM=©\0.1 
SPI_WriteCommand(0xBE00);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xBE01);    SPI_WriteParameter(0x50); 
 
//R+ 
SPI_WriteCommand(0xD100);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD101);    SPI_WriteParameter(0x37); 
SPI_WriteCommand(0xD102);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD103);    SPI_WriteParameter(0x52); 
SPI_WriteCommand(0xD104);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD105);    SPI_WriteParameter(0x7B); 
SPI_WriteCommand(0xD106);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD107);    SPI_WriteParameter(0x99); 
SPI_WriteCommand(0xD108);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD109);    SPI_WriteParameter(0xB1); 
SPI_WriteCommand(0xD10A);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD10B);    SPI_WriteParameter(0xD2); 
SPI_WriteCommand(0xD10C);    SPI_WriteParameter(0x00);
SPI_WriteCommand(0xD10D);    SPI_WriteParameter(0xF6); 
SPI_WriteCommand(0xD10E);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD10F);    SPI_WriteParameter(0x27); 
SPI_WriteCommand(0xD110);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD111);    SPI_WriteParameter(0x4E); 
SPI_WriteCommand(0xD112);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD113);    SPI_WriteParameter(0x8C); 
SPI_WriteCommand(0xD114);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD115);    SPI_WriteParameter(0xBE); 
SPI_WriteCommand(0xD116);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD117);    SPI_WriteParameter(0x0B); 
SPI_WriteCommand(0xD118);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD119);    SPI_WriteParameter(0x48); 
SPI_WriteCommand(0xD11A);    SPI_WriteParameter(0x02);
SPI_WriteCommand(0xD11B);    SPI_WriteParameter(0x4A); 
SPI_WriteCommand(0xD11C);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD11D);    SPI_WriteParameter(0x7E); 
SPI_WriteCommand(0xD11E);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD11F);    SPI_WriteParameter(0xBC); 
SPI_WriteCommand(0xD120);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD121);    SPI_WriteParameter(0xE1); 
SPI_WriteCommand(0xD122);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD123);    SPI_WriteParameter(0x10); 
SPI_WriteCommand(0xD124);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD125);    SPI_WriteParameter(0x31); 
SPI_WriteCommand(0xD126);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD127);    SPI_WriteParameter(0x5A); 
SPI_WriteCommand(0xD128);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD129);    SPI_WriteParameter(0x73); 
SPI_WriteCommand(0xD12A);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD12B);    SPI_WriteParameter(0x94); 
SPI_WriteCommand(0xD12C);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD12D);    SPI_WriteParameter(0x9F); 
SPI_WriteCommand(0xD12E);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD12F);    SPI_WriteParameter(0xB3); 
SPI_WriteCommand(0xD130);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD131);    SPI_WriteParameter(0xB9); 
SPI_WriteCommand(0xD132);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD133);    SPI_WriteParameter(0xC1); 
 
//G+ 
SPI_WriteCommand(0xD200);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD201);    SPI_WriteParameter(0x37); 
SPI_WriteCommand(0xD202);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD203);    SPI_WriteParameter(0x52); 
SPI_WriteCommand(0xD204);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD205);    SPI_WriteParameter(0x7B); 
SPI_WriteCommand(0xD206);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD207);    SPI_WriteParameter(0x99); 
SPI_WriteCommand(0xD208);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD209);    SPI_WriteParameter(0xB1); 
SPI_WriteCommand(0xD20A);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD20B);    SPI_WriteParameter(0xD2); 
SPI_WriteCommand(0xD20C);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD20D);    SPI_WriteParameter(0xF6); 
SPI_WriteCommand(0xD20E);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD20F);    SPI_WriteParameter(0x27); 
SPI_WriteCommand(0xD210);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD211);    SPI_WriteParameter(0x4E); 
SPI_WriteCommand(0xD212);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD213);    SPI_WriteParameter(0x8C); 
SPI_WriteCommand(0xD214);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD215);    SPI_WriteParameter(0xBE); 
SPI_WriteCommand(0xD216);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD217);    SPI_WriteParameter(0x0B); 
SPI_WriteCommand(0xD218);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD219);    SPI_WriteParameter(0x48); 
SPI_WriteCommand(0xD21A);    SPI_WriteParameter(0x02);
SPI_WriteCommand(0xD21B);    SPI_WriteParameter(0x4A); 
SPI_WriteCommand(0xD21C);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD21D);    SPI_WriteParameter(0x7E); 
SPI_WriteCommand(0xD21E);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD21F);    SPI_WriteParameter(0xBC); 
SPI_WriteCommand(0xD220);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD221);    SPI_WriteParameter(0xE1); 
SPI_WriteCommand(0xD222);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD223);    SPI_WriteParameter(0x10); 
SPI_WriteCommand(0xD224);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD225);    SPI_WriteParameter(0x31); 
SPI_WriteCommand(0xD226);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD227);    SPI_WriteParameter(0x5A); 
SPI_WriteCommand(0xD228);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD229);    SPI_WriteParameter(0x73); 
SPI_WriteCommand(0xD22A);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD22B);    SPI_WriteParameter(0x94); 
SPI_WriteCommand(0xD22C);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD22D);    SPI_WriteParameter(0x9F); 
SPI_WriteCommand(0xD22E);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD22F);    SPI_WriteParameter(0xB3); 
SPI_WriteCommand(0xD230);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD231);    SPI_WriteParameter(0xB9); 
SPI_WriteCommand(0xD232);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD233);    SPI_WriteParameter(0xC1); 
 
//B+ 
SPI_WriteCommand(0xD300);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD301);    SPI_WriteParameter(0x37); 
SPI_WriteCommand(0xD302);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD303);    SPI_WriteParameter(0x52); 
SPI_WriteCommand(0xD304);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD305);    SPI_WriteParameter(0x7B); 
SPI_WriteCommand(0xD306);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD307);    SPI_WriteParameter(0x99); 
SPI_WriteCommand(0xD308);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD309);    SPI_WriteParameter(0xB1); 
SPI_WriteCommand(0xD30A);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD30B);    SPI_WriteParameter(0xD2); 
SPI_WriteCommand(0xD30C);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD30D);    SPI_WriteParameter(0xF6); 
SPI_WriteCommand(0xD30E);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD30F);    SPI_WriteParameter(0x27); 
SPI_WriteCommand(0xD310);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD311);    SPI_WriteParameter(0x4E); 
SPI_WriteCommand(0xD312);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD313);    SPI_WriteParameter(0x8C); 
SPI_WriteCommand(0xD314);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD315);    SPI_WriteParameter(0xBE); 
SPI_WriteCommand(0xD316);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD317);    SPI_WriteParameter(0x0B); 
SPI_WriteCommand(0xD318);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD319);    SPI_WriteParameter(0x48); 
SPI_WriteCommand(0xD31A);    SPI_WriteParameter(0x02);
SPI_WriteCommand(0xD31B);    SPI_WriteParameter(0x4A); 
SPI_WriteCommand(0xD31C);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD31D);    SPI_WriteParameter(0x7E); 
SPI_WriteCommand(0xD31E);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD31F);    SPI_WriteParameter(0xBC); 
SPI_WriteCommand(0xD320);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD321);    SPI_WriteParameter(0xE1); 
SPI_WriteCommand(0xD322);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD323);    SPI_WriteParameter(0x10); 
SPI_WriteCommand(0xD324);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD325);    SPI_WriteParameter(0x31); 
SPI_WriteCommand(0xD326);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD327);    SPI_WriteParameter(0x5A); 
SPI_WriteCommand(0xD328);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD329);    SPI_WriteParameter(0x73); 
SPI_WriteCommand(0xD32A);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD32B);    SPI_WriteParameter(0x94); 
SPI_WriteCommand(0xD32C);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD32D);    SPI_WriteParameter(0x9F); 
SPI_WriteCommand(0xD32E);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD32F);    SPI_WriteParameter(0xB3); 
SPI_WriteCommand(0xD330);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD331);    SPI_WriteParameter(0xB9); 
SPI_WriteCommand(0xD332);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD333);    SPI_WriteParameter(0xC1);  
 
//R- 
SPI_WriteCommand(0xD400);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD401);    SPI_WriteParameter(0x37); 
SPI_WriteCommand(0xD402);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD403);    SPI_WriteParameter(0x52); 
SPI_WriteCommand(0xD404);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD405);    SPI_WriteParameter(0x7B); 
SPI_WriteCommand(0xD406);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD407);    SPI_WriteParameter(0x99); 
SPI_WriteCommand(0xD408);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD409);    SPI_WriteParameter(0xB1); 
SPI_WriteCommand(0xD40A);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD40B);    SPI_WriteParameter(0xD2); 
SPI_WriteCommand(0xD40C);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD40D);    SPI_WriteParameter(0xF6); 
SPI_WriteCommand(0xD40E);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD40F);    SPI_WriteParameter(0x27); 
SPI_WriteCommand(0xD410);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD411);    SPI_WriteParameter(0x4E); 
SPI_WriteCommand(0xD412);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD413);    SPI_WriteParameter(0x8C); 
SPI_WriteCommand(0xD414);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD415);    SPI_WriteParameter(0xBE); 
SPI_WriteCommand(0xD416);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD417);    SPI_WriteParameter(0x0B); 
SPI_WriteCommand(0xD418);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD419);    SPI_WriteParameter(0x48); 
SPI_WriteCommand(0xD41A);    SPI_WriteParameter(0x02);
SPI_WriteCommand(0xD41B);    SPI_WriteParameter(0x4A); 
SPI_WriteCommand(0xD41C);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD41D);    SPI_WriteParameter(0x7E); 
SPI_WriteCommand(0xD41E);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD41F);    SPI_WriteParameter(0xBC); 
SPI_WriteCommand(0xD420);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD421);    SPI_WriteParameter(0xE1); 
SPI_WriteCommand(0xD422);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD423);    SPI_WriteParameter(0x10); 
SPI_WriteCommand(0xD424);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD425);    SPI_WriteParameter(0x31); 
SPI_WriteCommand(0xD426);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD427);    SPI_WriteParameter(0x5A); 
SPI_WriteCommand(0xD428);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD429);    SPI_WriteParameter(0x73); 
SPI_WriteCommand(0xD42A);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD42B);    SPI_WriteParameter(0x94); 
SPI_WriteCommand(0xD42C);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD42D);    SPI_WriteParameter(0x9F); 
SPI_WriteCommand(0xD42E);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD42F);    SPI_WriteParameter(0xB3); 
SPI_WriteCommand(0xD430);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD431);    SPI_WriteParameter(0xB9); 
SPI_WriteCommand(0xD432);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD433);    SPI_WriteParameter(0xC1); 
 
//G- 
SPI_WriteCommand(0xD500);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD501);    SPI_WriteParameter(0x37); 
SPI_WriteCommand(0xD502);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD503);    SPI_WriteParameter(0x52); 
SPI_WriteCommand(0xD504);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD505);    SPI_WriteParameter(0x7B); 
SPI_WriteCommand(0xD506);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD507);    SPI_WriteParameter(0x99); 
SPI_WriteCommand(0xD508);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD509);    SPI_WriteParameter(0xB1); 
SPI_WriteCommand(0xD50A);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD50B);    SPI_WriteParameter(0xD2); 
SPI_WriteCommand(0xD50C);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD50D);    SPI_WriteParameter(0xF6); 
SPI_WriteCommand(0xD50E);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD50F);    SPI_WriteParameter(0x27); 
SPI_WriteCommand(0xD510);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD511);    SPI_WriteParameter(0x4E); 
SPI_WriteCommand(0xD512);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD513);    SPI_WriteParameter(0x8C); 
SPI_WriteCommand(0xD514);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD515);    SPI_WriteParameter(0xBE); 
SPI_WriteCommand(0xD516);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD517);    SPI_WriteParameter(0x0B); 
SPI_WriteCommand(0xD518);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD519);    SPI_WriteParameter(0x48); 
SPI_WriteCommand(0xD51A);    SPI_WriteParameter(0x02);
SPI_WriteCommand(0xD51B);    SPI_WriteParameter(0x4A); 
SPI_WriteCommand(0xD51C);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD51D);    SPI_WriteParameter(0x7E); 
SPI_WriteCommand(0xD51E);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD51F);    SPI_WriteParameter(0xBC); 
SPI_WriteCommand(0xD520);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD521);    SPI_WriteParameter(0xE1); 
SPI_WriteCommand(0xD522);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD523);    SPI_WriteParameter(0x10); 
SPI_WriteCommand(0xD524);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD525);    SPI_WriteParameter(0x31); 
SPI_WriteCommand(0xD526);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD527);    SPI_WriteParameter(0x5A); 
SPI_WriteCommand(0xD528);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD529);    SPI_WriteParameter(0x73); 
SPI_WriteCommand(0xD52A);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD52B);    SPI_WriteParameter(0x94); 
SPI_WriteCommand(0xD52C);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD52D);    SPI_WriteParameter(0x9F); 
SPI_WriteCommand(0xD52E);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD52F);    SPI_WriteParameter(0xB3); 
SPI_WriteCommand(0xD530);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD531);    SPI_WriteParameter(0xB9); 
SPI_WriteCommand(0xD532);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD533);    SPI_WriteParameter(0xC1); 
 
//B- 
SPI_WriteCommand(0xD600);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD601);    SPI_WriteParameter(0x37); 
SPI_WriteCommand(0xD602);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD603);    SPI_WriteParameter(0x52); 
SPI_WriteCommand(0xD604);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD605);    SPI_WriteParameter(0x7B); 
SPI_WriteCommand(0xD606);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD607);    SPI_WriteParameter(0x99); 
SPI_WriteCommand(0xD608);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD609);    SPI_WriteParameter(0xB1); 
SPI_WriteCommand(0xD60A);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD60B);    SPI_WriteParameter(0xD2); 
SPI_WriteCommand(0xD60C);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xD60D);    SPI_WriteParameter(0xF6); 
SPI_WriteCommand(0xD60E);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD60F);    SPI_WriteParameter(0x27); 
SPI_WriteCommand(0xD610);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD611);    SPI_WriteParameter(0x4E); 
SPI_WriteCommand(0xD612);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD613);    SPI_WriteParameter(0x8C); 
SPI_WriteCommand(0xD614);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xD615);    SPI_WriteParameter(0xBE); 
SPI_WriteCommand(0xD616);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD617);    SPI_WriteParameter(0x0B); 
SPI_WriteCommand(0xD618);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD619);    SPI_WriteParameter(0x48); 
SPI_WriteCommand(0xD61A);    SPI_WriteParameter(0x02);
SPI_WriteCommand(0xD61B);    SPI_WriteParameter(0x4A); 
SPI_WriteCommand(0xD61C);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD61D);    SPI_WriteParameter(0x7E); 
SPI_WriteCommand(0xD61E);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD61F);    SPI_WriteParameter(0xBC); 
SPI_WriteCommand(0xD620);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xD621);    SPI_WriteParameter(0xE1); 
SPI_WriteCommand(0xD622);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD623);    SPI_WriteParameter(0x10); 
SPI_WriteCommand(0xD624);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD625);    SPI_WriteParameter(0x31); 
SPI_WriteCommand(0xD626);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD627);    SPI_WriteParameter(0x5A); 
SPI_WriteCommand(0xD628);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD629);    SPI_WriteParameter(0x73); 
SPI_WriteCommand(0xD62A);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD62B);    SPI_WriteParameter(0x94); 
SPI_WriteCommand(0xD62C);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD62D);    SPI_WriteParameter(0x9F); 
SPI_WriteCommand(0xD62E);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD62F);    SPI_WriteParameter(0xB3); 
SPI_WriteCommand(0xD630);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD631);    SPI_WriteParameter(0xB9); 
SPI_WriteCommand(0xD632);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xD633);    SPI_WriteParameter(0xC1); 
 
//Enable Page0 
SPI_WriteCommand(0xF000);    SPI_WriteParameter(0x55); 
SPI_WriteCommand(0xF001);    SPI_WriteParameter(0xAA); 
SPI_WriteCommand(0xF002);    SPI_WriteParameter(0x52); 
SPI_WriteCommand(0xF003);    SPI_WriteParameter(0x08); 
SPI_WriteCommand(0xF004);    SPI_WriteParameter(0x00); 
 
//RGB I/F Setting 
SPI_WriteCommand(0xB000);    SPI_WriteParameter(0x08); 
SPI_WriteCommand(0xB001);    SPI_WriteParameter(0x05); 
SPI_WriteCommand(0xB002);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xB003);    SPI_WriteParameter(0x05); 
SPI_WriteCommand(0xB004);    SPI_WriteParameter(0x02); 

// SDT: 
SPI_WriteCommand(0xB600);    SPI_WriteParameter(0x0A); 

// Gate EQ: 
SPI_WriteCommand(0xB700);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xB701);    SPI_WriteParameter(0x70); 
 
// Source EQ 
SPI_WriteCommand(0xB800);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xB801);    SPI_WriteParameter(0x05); 
SPI_WriteCommand(0xB802);    SPI_WriteParameter(0x05); 
SPI_WriteCommand(0xB803);    SPI_WriteParameter(0x05); 

//Inversion: Column inversion (NVT) 
SPI_WriteCommand(0xBC00);    SPI_WriteParameter(0x02); 
SPI_WriteCommand(0xBC01);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xBC02);    SPI_WriteParameter(0x00); 
 
//BOE's Setting (default) 
SPI_WriteCommand(0xCC00);    SPI_WriteParameter(0x03); 
SPI_WriteCommand(0xCC01);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xCC02);    SPI_WriteParameter(0x00); 
 
//Display Timing 
SPI_WriteCommand(0xBD00);    SPI_WriteParameter(0x01); 
SPI_WriteCommand(0xBD01);    SPI_WriteParameter(0x84); 
SPI_WriteCommand(0xBD02);    SPI_WriteParameter(0x07); 
SPI_WriteCommand(0xBD03);    SPI_WriteParameter(0x31); 
SPI_WriteCommand(0xBD04);    SPI_WriteParameter(0x00); 
SPI_WriteCommand(0xBA00);    SPI_WriteParameter(0x01);

 
SPI_WriteCommand(0xFF00);    SPI_WriteParameter(0xAA); 
SPI_WriteCommand(0xFF01);    SPI_WriteParameter(0x55); 
SPI_WriteCommand(0xFF02);    SPI_WriteParameter(0x25); 
SPI_WriteCommand(0xFF03);    SPI_WriteParameter(0x01); 

SPI_WriteCommand(0x3500);SPI_WriteParameter(0x00);

SPI_WriteCommand(0x3A00);SPI_WriteParameter(0x77);

 
SPI_WriteCommand(0x1100); 
msleep(120);//msleep 120 
SPI_WriteCommand(0x2900);
//==========================================================
}

static int lcdc_ts_panel_on(struct platform_device *pdev)
{
//luke: add for continue
   struct msm_fb_data_type *mfd = platform_get_drvdata(pdev);

    if (!mfd->cont_splash_done) {
        mfd->cont_splash_done = 1;
printk("luke: %s: %d\n",__func__,__LINE__);
        //return 0;
    }
//luke: add end

	if (lcdc_pdata->panel_config_gpio)
		lcdc_pdata->panel_config_gpio(1);
printk("luke: %s: %d\n",__func__,__LINE__);
if (! lcdc_state.initialized){
	lcdc_ts_disp_reginit();
	lcdc_state.initialized = TRUE;
//printk("luke:  _________________%s____________________ \n",__func__);
}
	return 0;
}

static int lcdc_ts_panel_off(struct platform_device *pdev)
{

	if ( lcdc_state.initialized) {
printk("luke: %s: %d\n",__func__,__LINE__);
		/* Main panel power off (Pull down reset) */
		SPI_WriteCommand(0x2800);
                SPI_WriteCommand(0x1000);
		msleep(50);
		//lcdc_state.display_on = FALSE;
		lcdc_state.initialized = FALSE;
	}
//printk("luke:  _________________%s____________________ \n",__func__);
	return 0;
}

#define LCD_DRIVER_BL 1
//static int lcd_backlight = 0;
static void lcdc_ts_set_backlight(struct msm_fb_data_type *mfd)
{

if (lcdc_state.initialized) {
    if(!LCD_DRIVER_BL)
    {
	int step = 0, i = 0;
	int bl_level = mfd->bl_level;

	/* real backlight level, 1 - max, 16 - min, 32 - off */
	bl_level = 32 - bl_level;

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
			msleep(30);
		}

		/* adjust backlight level */
		for (i = 0; i < step; i++) {
			gpio_set_value_cansleep(gpio_backlight_en, 0);
			mdelay(1);
			gpio_set_value_cansleep(gpio_backlight_en, 1);
			mdelay(1);
		}
	}
	msleep(1);
	prev_bl = bl_level;
    }
    else {
	int bl_level = mfd->bl_level;
        printk("luke: %s -----------------------------  bl_level=%d \n",__func__, bl_level);
	gpio_set_value_cansleep(spi_cs, 0);
	//if(!lcd_backlight)
	//mmdelay(80);
        SPI_WriteCommand(0x5100);
        SPI_WriteParameter(bl_level);
		
	gpio_set_value_cansleep(spi_mosi, 1);
	gpio_set_value_cansleep(spi_cs, 1);

	SPI_WriteCommand(0x5300);
	SPI_WriteParameter(0x24);
	SPI_WriteCommand(0x5500);
	SPI_WriteParameter(0x00);


	if(0 == bl_level){
		//Start===Allen
		keyboard_led_light(KEY_LED_USER_LCD, 0);
		//lcd_backlight = 0;
		//End===Allen
	}else {
		//Start===Allen
		keyboard_led_light(KEY_LED_USER_LCD, 1);
		//End===Allen
		//lcd_backlight = 1;
	}  
    }
}
    return;
}

static int __devinit lcdc_ts_probe(struct platform_device *pdev)
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
	.probe  = lcdc_ts_probe,
	.driver = {
		.name   = "lcdc_ts_ips_nt35512_pt",
	},	          
};

static struct msm_fb_panel_data lcdc_panel_data = {

	.on = lcdc_ts_panel_on,
	.off = lcdc_ts_panel_off,
	.set_backlight = lcdc_ts_set_backlight,
};

static struct platform_device this_device = {
	.name   = "lcdc_ts_ips_nt35512_pt",
	.id	= 1,
	.dev	= {
		.platform_data = &lcdc_panel_data,
	}
};




static int __init lcdc_ts_panel_init(void)
{
	int ret;
	struct msm_panel_info *pinfo;

#ifdef CONFIG_FB_MSM_LCDC_AUTO_DETECT
	if (msm_fb_detect_client("lcdc_ts_ips_nt35512_pt")) {
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

	pinfo->lcdc.h_back_porch = 10;		/* hsw = 8 + hbp=16 */
	pinfo->lcdc.h_front_porch = 10;
	pinfo->lcdc.h_pulse_width = 10;
	pinfo->lcdc.v_back_porch = 6;		/* vsw=1 + vbp = 7 */
	pinfo->lcdc.v_front_porch = 6;
	pinfo->lcdc.v_pulse_width = 4;
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

device_initcall(lcdc_ts_panel_init);


