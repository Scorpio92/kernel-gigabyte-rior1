#include "AW5306_Reg.h"
#include "AW5306_Drv.h"
#include <linux/string.h>
#include "AW5306_userpara.h"

#define	POS_PRECISION				64

extern AW5306_UCF	AWTPCfg;
extern STRUCTCALI	AW_Cali;

extern void AW5306_CLB_WriteCfg(void);
extern int I2C_WriteByte(unsigned char addr, unsigned char data);
extern unsigned char I2C_ReadByte(unsigned char addr);
extern unsigned char I2C_ReadXByte( unsigned char *buf, unsigned char addr, unsigned short len);

const STRUCTCALI Default_Cali = 
{
	"AWINIC TP CALI",
	{0x56,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55},		//TXOFFSET
	{0x24,0x33,0x33,0x43,0x24,0x42},								//RXOFFSET  
	{0x5a,0x5a,0x5a,0x5a,0x5a,0x5a,0x5a,0x5a,0x5a,0x5a,0x5a,0x5a,0x5a,0x5a,0x5a,0x5a,0x5a,0x5a,0x5a,0x5a},//TXCAC 
	{0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30},//RXCAC 
	{0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28}
};

void AW5306_User_Init()
{
//	unsigned char i;
	//unsigned char ret;

	AWTPCfg.TX_LOCAL = 18;		//TX number of TP
	AWTPCfg.RX_LOCAL = 10;		//RX number of TP
	AWTPCfg.RX_INV_ORDER = 1;	//RX mapping in inverted order

	AWTPCfg.HAVE_KEY_LINE = 1;


	AWTPCfg.MAPPING_MAX_X = 540;	//   320 LCD DISPLAY SOLUTION X
	AWTPCfg.MAPPING_MAX_Y = 1014;	//   960 LCD DISPLAY SOLUTION Y  480 + 480 /13   960/18Tx +960

	AWTPCfg.K_X = ((AWTPCfg.MAPPING_MAX_X - 1)*256)/(AWTPCfg.RX_LOCAL*POS_PRECISION - 1);  //192
	AWTPCfg.K_Y = ((AWTPCfg.MAPPING_MAX_Y - 1)*256)/(AWTPCfg.TX_LOCAL*POS_PRECISION - 1);   //195 

	AWTPCfg.FLYING_TH = 400;	//flying theshold
	AWTPCfg.MOVING_TH = 250;	//moving theshold
	AWTPCfg.MOVING_ACCELER = 70;	//moving ACC
	AWTPCfg.FIRST_CALI = 0;			//calibrate switch, 1 means calibration at first poweron 

	I2C_WriteByte(SA_IDRST,0x55);			//reset chip

	I2C_WriteByte(SA_SCANFREQ1,3);		//3-5
	I2C_WriteByte(SA_TCLKDLY,1);
	I2C_WriteByte(SA_TX_NUM,AWTPCfg.TX_LOCAL);
	I2C_WriteByte(SA_RX_NUM,AWTPCfg.RX_LOCAL);
	I2C_WriteByte(SA_RX_START,2);
	I2C_WriteByte(SA_WAITTIME, 100);		//16*(FREQ1+1)*32/27 us 

	I2C_WriteByte(SA_PAGE,1);
	I2C_WriteByte(SA_TRACEINTERVAL,0x55);
	I2C_WriteByte(SA_TRACESTEP,0x33);
	I2C_WriteByte(SA_CHAMPCFG,0x2b);
	I2C_WriteByte(SA_PAGE,0);
	
      #if 0
	for(i=0;i<AWTPCfg.TX_LOCAL;i++)
	{
		I2C_WriteByte(SA_TX_INDEX0+i,19-i);			//TX REVERT
	}
      #endif
	I2C_WriteByte(SA_TX_INDEX0+0,19);
	I2C_WriteByte(SA_TX_INDEX0+1,18);
	I2C_WriteByte(SA_TX_INDEX0+2,17);
	I2C_WriteByte(SA_TX_INDEX0+3,16);
	I2C_WriteByte(SA_TX_INDEX0+4,15);
	I2C_WriteByte(SA_TX_INDEX0+5,14);
	I2C_WriteByte(SA_TX_INDEX0+6,13);
	I2C_WriteByte(SA_TX_INDEX0+7,12);
	I2C_WriteByte(SA_TX_INDEX0+8,11);
	I2C_WriteByte(SA_TX_INDEX0+9,10);
	I2C_WriteByte(SA_TX_INDEX0+10,9);
	I2C_WriteByte(SA_TX_INDEX0+11,2);
	I2C_WriteByte(SA_TX_INDEX0+12,3);
	I2C_WriteByte(SA_TX_INDEX0+13,4);
	I2C_WriteByte(SA_TX_INDEX0+14,5);
	I2C_WriteByte(SA_TX_INDEX0+15,6);
	I2C_WriteByte(SA_TX_INDEX0+16,7);
	I2C_WriteByte(SA_TX_INDEX0+17,8);	

	  
	memcpy(&AW_Cali,&Default_Cali,sizeof(STRUCTCALI));		//load default cali value
	AW5306_CLB_WriteCfg();


}
