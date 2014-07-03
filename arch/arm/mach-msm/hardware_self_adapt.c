/*
 * Copyright (C) 2010 Huawei, Inc.
 * Copyright (c) 2008-2010, Huawei. All rights reserved.
 */
 
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/errno.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/setup.h>

#include <asm/mach-types.h>
#include "linux/hardware_self_adapt.h"

static unsigned int lcd_id = 0;
static unsigned int sub_board_id = 0;
#ifdef CONFIG_HUAWEI_POWER_DOWN_CHARGE
static unsigned int charge_flag = 0;
#endif

static unsigned int lcd_y_res = 480;

/* framebuffer size self adapter */
#ifdef CONFIG_FRAMEBUF_SELF_ADAPT
static __u32	frame_buffer_size = 0;
static __u32	frame_buffer_start = 0;	/* physical start address */
#endif

#ifdef CONFIG_FRAMEBUF_SELF_ADAPT_HACK
#define BL_BUF_BLOCK 1048576
static bool frame_buffer_boosted = false;
#endif

/* cust size self adapter */
static __u32	cust_buffer_size = 0;
static __u32	cust_buffer_start = 0;	/* physical start address */

static unsigned int recovery_boot_mode = 0;
 
#define ATAG_LCD_ID 0x4d534D73
int __init parse_tag_lcd_id(const struct tag *tags)
{
	 struct tag *t = (struct tag *)tags;
 
	 lcd_id = t->u.revision.rev;
	 printk(KERN_DEBUG "%s: lcd_id = 0x%x\n", __func__, lcd_id);
	 
	 return lcd_id;
}
 __tagtable(ATAG_LCD_ID, parse_tag_lcd_id);
 
#define ATAG_SUB_BOARD_ID 0x4d534D76
int __init parse_tag_sub_board_id(const struct tag *tags)
{
	 struct tag *t = (struct tag *)tags;

	 sub_board_id = t->u.revision.rev;
	 printk(KERN_DEBUG "%s: sub_board_id = 0x%x\n", __func__, sub_board_id);
	 
	 return sub_board_id;
}
 __tagtable(ATAG_SUB_BOARD_ID, parse_tag_sub_board_id);

#ifdef CONFIG_USB_AUTO_INSTALL
#define ATAG_BOOT_MODE_ID   0x4d534d77
 int __init parse_tag_boot_mode_id(const struct tag *tags)
 {
	 struct tag *t = (struct tag *)tags;
 
	 recovery_boot_mode = t->u.revision.rev;
	 printk(KERN_DEBUG "%s: usb_mode_id = 0x%x\n", __func__, recovery_boot_mode);
	 return recovery_boot_mode;
 }
 __tagtable(ATAG_BOOT_MODE_ID, parse_tag_boot_mode_id);
#endif  /* CONFIG_USB_AUTO_INSTALL */
 
#ifdef CONFIG_HUAWEI_POWER_DOWN_CHARGE
#define ATAG_CHARGE_FLAG  0x4d534D78
int __init parse_tag_charge_flag(const struct tag *tags)
{
    struct tag *t = (struct tag *)tags;

    charge_flag = t->u.revision.rev;
    printk(KERN_DEBUG "%s: charge_flag = 0x%x\n", __func__, charge_flag);

    return charge_flag;  
}
__tagtable(ATAG_CHARGE_FLAG, parse_tag_charge_flag);
#endif
 
/*get framebuffer address and size from atag, passed by bootloader*/
#ifdef CONFIG_FRAMEBUF_SELF_ADAPT
#define ATAG_FRAME_BUFFER_ID 0x4d534D79
int __init parse_tag_frame_buffer(const struct tag *tags)
{
    frame_buffer_size = tags->u.mem.size;
#ifdef CONFIG_FRAMEBUF_SELF_ADAPT_HACK
    if(frame_buffer_size < 6 * BL_BUF_BLOCK)
    {
        frame_buffer_size += BL_BUF_BLOCK;
        frame_buffer_boosted = true;
    }
#endif
    frame_buffer_start = tags->u.mem.start;
	
    printk(KERN_DEBUG "%s: fb addr= 0x%x, size=0x%0x\n", __func__, frame_buffer_start, frame_buffer_size);
    return 0;
}
__tagtable(ATAG_FRAME_BUFFER_ID, parse_tag_frame_buffer);

#define ATAG_LCD_Y_RES_FLAG 0x4d534D7A
int __init parse_tag_lcd_y_res_flag(const struct tag *tags)
{
    struct tag *t = (struct tag *)tags;

    lcd_y_res= t->u.revision.rev;
    printk(KERN_DEBUG "%s: lcd_y_res = %d\n", __func__, lcd_y_res);

    return lcd_y_res;  
}
__tagtable(ATAG_LCD_Y_RES_FLAG, parse_tag_lcd_y_res_flag);

/*used in board-msm7x27a.c*/
void get_frame_buffer_mem_region(__u32 *start_addr, __u32 *size)
{
	*start_addr = frame_buffer_start;
	*size = frame_buffer_size;
}
#endif


/*get cust address and size from atag, passed by bootloader*/
#define ATAG_CUST_BUFFER_ID 0x4d534D7B
int __init parse_tag_cust_buffer(const struct tag * tags)
{
    cust_buffer_size = tags->u.mem.size;
    cust_buffer_start = tags->u.mem.start;
#ifdef CONFIG_FRAMEBUF_SELF_ADAPT_HACK
    if(frame_buffer_boosted)
        cust_buffer_start += BL_BUF_BLOCK;
#endif

    printk(KERN_DEBUG "%s: cust addr= 0x%x, size=0x%0x\n", __func__, cust_buffer_start, cust_buffer_size);
    return 0;
}
__tagtable(ATAG_CUST_BUFFER_ID, parse_tag_cust_buffer);

/*used in board-msm7x27a.c*/
void get_cust_buffer_mem_region(__u32 *start_addr, __u32 *size)
{
	*start_addr = cust_buffer_start;
	*size = cust_buffer_size;
}

char *get_wifi_device_name(void)
{                                                                                                        
  hw_wifi_device_model wifi_device_model = WIFI_UNKNOW;  
  char *wifi_device_id = NULL;                       
                                                             
  wifi_device_model = get_hw_wifi_device_model();  
  //printk("wifi_device_id = %d\n",wifi_device_model);  
  if(WIFI_BROADCOM_4329 == wifi_device_model)                 
  {                                                  
	wifi_device_id = "1.1";
  }
  
  else if(WIFI_BROADCOM_4330 == wifi_device_model)
  {                                                  
	wifi_device_id = "1.2";
  }
                                                 
  else if(WIFI_QUALCOMM_6005 == wifi_device_model)            
  { 
    wifi_device_id = "2.1"; 
  }                                                  
  else                                               
  {                                                  
    wifi_device_id = "UNKNOWN WIFI DEVICE";          
  }                                                  
  return wifi_device_id;                             
} 

/* store bt device model and bt device name in bt_device_array[] */
struct bt_device bt_device_array[] = 
{
    { BT_BCM4329, "1.1" },
	{ BT_BCM4330, "1.2" },
	{ BT_WCN2243, "2.1" },
	{ BT_UNKNOWN, "Unknown" }
};

/* get bt device model by board id */

/* get bt device name */
char *get_bt_device_name(void)
{                   
    hw_bt_device_model bt_device_model = BT_UNKNOWN;
    int i = 0;

    bt_device_model = get_hw_bt_device_model();

    /* lookup bt_device_model in bt_device_array[] */
    for(i = 0; i < BT_UNKNOWN; i++)
    {
        if(bt_device_model == bt_device_array[i].dev_model)
        {
            break; 
        }
    }
	
	return bt_device_array[i].dev_name;
} 

char *get_framebuffer_boosted(void)
{
  char *framebuffer_boosted = NULL;
#ifdef CONFIG_FRAMEBUF_SELF_ADAPT_HACK
  if(frame_buffer_boosted)
  {
    framebuffer_boosted = "1";
  }
  else
  {
#endif
    framebuffer_boosted = "0";
#ifdef CONFIG_FRAMEBUF_SELF_ADAPT_HACK
  }
#endif
  return framebuffer_boosted;
}

/* modify spk mic function name */
/* Add DTS property */
/* add special requirement for U8951-1/-51 */
void get_audio_property(char *audio_property)
{
  unsigned int property = AUDIO_PROPERTY_INVALID;
  audio_property_type mic_type = MIC_NONE;
  audio_property_type fir_enable = FIR_DISABLE;
  audio_property_type fm_type = FM_BROADCOM;
  audio_property_type spk_type = MONO_SPEAKER;
  audio_property_type spkmic_type = SPK_MAIN_MIC;
  audio_property_type dts_enable = DTS_DISABLE;
  audio_property_type submic_mmi = SMICMMI_ENABLE;
  
  mic_type = get_audio_mic_type();
  fir_enable = get_audio_fir_enabled();
  fm_type =  get_audio_fm_type();
  spk_type = get_audio_speaker_type();
  spkmic_type = get_audio_spkmic_type();
  dts_enable = get_audio_dts_enable();
  submic_mmi = get_audio_mmi_submic_test_enable();
  
  property = spkmic_type | spk_type | fm_type | fir_enable | mic_type | dts_enable | submic_mmi;

  sprintf(audio_property, "%8x", property);
}

unsigned int get_hw_lcd_id(void)
{
	return lcd_id;
}

hw_ver_sub_type get_hw_sub_board_id(void)
{
	return (hw_ver_sub_type)(sub_board_id&HW_VER_SUB_MASK);
}

#ifdef CONFIG_HUAWEI_POWER_DOWN_CHARGE
unsigned int get_charge_flag(void)
{
    return charge_flag;
}
#endif

lcd_type atag_get_lcd_y_res(void)
{
   return (lcd_type)lcd_y_res;
}

/* the function interface to check boot mode in kernel */
unsigned char bootimage_is_recovery(void)
{
  return recovery_boot_mode;
}
/*  FUNCTION  get_hw_lcd_interface_type
 *  DEPENDENCIES 
 *      get lcd interface type
 *      affect nfc.
 *  RETURN VALUE
 *      lcd interface type:LCD_IS_MIPI or LCD_IS_RGB
 */
/*
 *brief: get lcd panel resolution
 */
lcd_type get_hw_lcd_resolution_type(void)
{
    lcd_type lcd_resolution = LCD_IS_HVGA;

    lcd_resolution = LCD_IS_WVGA;

    return lcd_resolution;
}


unsigned int get_mdp_pmem_size(void)
{
	unsigned int mdp_pmem_size = 0;
	lcd_type lcd_resolution = LCD_IS_HVGA;
	lcd_resolution = get_hw_lcd_resolution_type();
	switch(lcd_resolution)
	{
		case LCD_IS_QVGA:
		case LCD_IS_HVGA:
			mdp_pmem_size = 0x1500000;  //21M
			break;
		case LCD_IS_WVGA:
		case LCD_IS_FWVGA:
			mdp_pmem_size = 0x1C00000; //28M
			break;
		case LCD_IS_QHD:
			mdp_pmem_size = 0x2300000; //35M
			break;
		default:
			mdp_pmem_size = 0x2300000; //35M
			break;
	}
	
	return mdp_pmem_size;	
}
