/**************************************************************************
*  AW5306_ts.c
* 
*  AW5306 spreadtrum sc8810 sample code version 1.0
* 
*  Create Date : 2012/07/31
* 
*  Modify Date : 
*
*  Create by   : wuhaijun
* 
**************************************************************************/

#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/gpio.h>
#include <linux/earlysuspend.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include <linux/firmware.h>
#include <linux/platform_device.h>

#include <linux/slab.h>
//#include <mach/ldo.h>
//#include <mach/common.h>

#include <linux/input/AW5306_Drv.h>
#include "AW5306_userpara.h"

#define TS_DEBUG_MSG 			1
#define I2C_BOARD_INFO_METHOD   1
#define TS_DATA_THRESHOLD_CHECK	0



#define TS_WIDTH_MAX			480
#define	TS_HEIGHT_MAX			800
#define SCREEN_MAX_X                    540// 	480   sym modified  
#define SCREEN_MAX_Y                    960//800 sym modified
#define PRESS_MAX			0x7F

static int debug_level=1;

#if TS_DEBUG_MSG
#define TS_DBG(format, ...)	\
	if(debug_level == 1)	\
		printk(KERN_INFO "AW5306 " format "\n", ## __VA_ARGS__)
#else
#define TS_DBG(format, ...)
#endif


struct sprd_i2c_setup_data {
	unsigned i2c_bus;  //the same number as i2c->adap.nr in adapter probe function
	unsigned short i2c_address;
	int irq;
	char type[I2C_NAME_SIZE];
};


int AW5306_irq;

extern int sprd_3rdparty_gpio_tp_pwr;
extern int sprd_3rdparty_gpio_tp_rst;
extern int sprd_3rdparty_gpio_tp_irq;

extern AW5306_UCF	AWTPCfg;
extern STRUCTCALI		AW_Cali;

unsigned char I2C_ReadByte(u8 addr);

#define AW5306_TS_NAME	   	       	"AW5306_ts"
#define AW5306_TS_ADDR				0x38

static struct i2c_client *this_client;
static struct sprd_i2c_setup_data AW5306_ts_setup={0, AW5306_TS_ADDR, 0, AW5306_TS_NAME};

/* Attribute */
static ssize_t AW5306_show_debug(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t AW5306_store_debug(struct device* cd, struct device_attribute *attr,const char* buf, size_t len);
static ssize_t AW5306_get_cali(struct device* cd,struct device_attribute *attr, char* buf);
//static ssize_t AW5306_set_cali(struct device* cd, struct device_attribute *attr,const char* buf, size_t len);
static ssize_t AW5306_get_reg(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t AW5306_get_Base(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t AW5306_get_Diff(struct device* cd,struct device_attribute *attr, char* buf);


static void AW5306_ts_suspend(struct early_suspend *handler);
static void AW5306_ts_resume(struct early_suspend *handler);

#define CONFIG_AW5306_MULTITOUCH 1

struct ts_event {
	int	x[5];
	int	y[5];
	int id[5];
	int	pressure;
  	int touch_point;
  	int	pre_point;
};

struct AW5306_ts_data {
	struct input_dev	*input_dev;
	struct ts_event		event;
	struct work_struct 	pen_event_work;
	struct workqueue_struct *ts_workqueue;
	struct early_suspend	early_suspend;
	struct timer_list touch_timer;
};

static unsigned char suspend_flag=0; //0: sleep out; 1: sleep in
static short tp_idlecnt = 0;
static char tp_SlowMode = 1;



static DEVICE_ATTR(debug, S_IRUGO | S_IWUSR, AW5306_show_debug, AW5306_store_debug);
static DEVICE_ATTR(cali,  S_IRUGO | S_IWUSR, AW5306_get_cali,  NULL);
static DEVICE_ATTR(getreg,  S_IRUGO | S_IWUSR, AW5306_get_reg,    NULL);
static DEVICE_ATTR(base,  S_IRUGO | S_IWUSR, AW5306_get_Base,    NULL);
static DEVICE_ATTR(diff, S_IRUGO | S_IWUSR, AW5306_get_Diff,    NULL);

//jed add the power control here
static void AW5306_ts_pwron(void)
{
//	LDO_SetVoltLevel(LDO_LDO_SIM2, LDO_VOLT_LEVEL0);
//	LDO_TurnOnLDO(LDO_LDO_SIM2);
//	msleep(20);
}

#if 0
static void AW5306_ts_pwroff(void)
{
	LDO_TurnOffLDO(LDO_LDO_SIM2);
}
#endif

static ssize_t AW5306_show_debug(struct device* cd,struct device_attribute *attr, char* buf)
{
	ssize_t ret = 0;
	
	sprintf(buf, "AW5306 Debug %d\n",debug_level);
	
	ret = strlen(buf) + 1;

	return ret;
}

static ssize_t AW5306_store_debug(struct device* cd, struct device_attribute *attr,
		       const char* buf, size_t len)
{
	unsigned long on_off = simple_strtoul(buf, NULL, 10);
	debug_level = on_off;

	TS_DBG("%s: debug_level=%d\n",__func__, debug_level);
	
	return len;
}

static ssize_t AW5306_get_cali(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char i;
	ssize_t len = 0;

	len += snprintf(buf+len, PAGE_SIZE-len,"*****AW5306 Calibrate data*****\n");
	len += snprintf(buf+len, PAGE_SIZE-len,"TXOFFSET:");
	
	for(i=0;i<10;i++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.TXOFFSET[i]);
	}
	
	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	len += snprintf(buf+len, PAGE_SIZE-len,  "RXOFFSET:");

	for(i=0;i<6;i++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.RXOFFSET[i]);
	}

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	len += snprintf(buf+len, PAGE_SIZE-len,  "TXCAC:");

	for(i=0;i<20;i++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.TXCAC[i]);
	}

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	len += snprintf(buf+len, PAGE_SIZE-len,  "RXCAC:");

	for(i=0;i<12;i++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.RXCAC[i]);
	}

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");
	len += snprintf(buf+len, PAGE_SIZE-len,  "TXGAIN:");

	for(i=0;i<20;i++)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "0x%02X ", AW_Cali.TXGAIN[i]);
	}

	len += snprintf(buf+len, PAGE_SIZE-len,  "\n");

	return len;
	
}


static ssize_t AW5306_get_Base(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char i,j;
	unsigned short base;
	ssize_t len = 0;

	len += snprintf(buf+len, PAGE_SIZE-len, "base: \n");
	for(i=0;i< AWTPCfg.TX_LOCAL;i++)
	{
		for(j=0;j<AWTPCfg.RX_LOCAL;j++)
		{
			AW5306_GetBase(&base,i,j);	
			len += snprintf(buf+len, PAGE_SIZE-len, "%4d, ",base);
		}
		len += snprintf(buf+len, PAGE_SIZE-len, "\n");
	}
	
	return len;

}

static ssize_t AW5306_get_Diff(struct device* cd,struct device_attribute *attr, char* buf)
{
	unsigned char i,j;
	short diff;
	ssize_t len = 0;

	len += snprintf(buf+len, PAGE_SIZE-len, "Diff: \n");
	for(i=0;i< AWTPCfg.TX_LOCAL;i++)
	{
		for(j=0;j<AWTPCfg.RX_LOCAL;j++)
		{
			AW5306_GetDiff(&diff,i,j);	
			len += snprintf(buf+len, PAGE_SIZE-len, "%4d, ",diff);
		}
		len += snprintf(buf+len, PAGE_SIZE-len, "\n");
	}
	
	return len;

}

static ssize_t AW5306_get_reg(struct device* cd,struct device_attribute *attr, char* buf)
{
	u8 reg_val[1];
	ssize_t len = 0;
	u8 i;

	for(i=1;i<0x7F;i++)
	{
		reg_val[0] = I2C_ReadByte(i);
		len += snprintf(buf+len, PAGE_SIZE-len, "reg%02X = 0x%02X, ", i,reg_val[0]);
	}

	return len;

}
static int AW5306_create_sysfs(struct i2c_client *client)
{
	int err;
	struct device *dev = &(client->dev);

	TS_DBG("%s", __func__);
	
	err = device_create_file(dev, &dev_attr_debug);
	err = device_create_file(dev, &dev_attr_cali);
	err = device_create_file(dev, &dev_attr_getreg);
	err = device_create_file(dev, &dev_attr_base);
	err = device_create_file(dev, &dev_attr_diff);
	return err;
}

#if 0
static int AW5306_i2c_rxdata(char *rxdata, int length)
{
	int ret;

	struct i2c_msg msgs[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= rxdata,
		},
		{
			.addr	= this_client->addr,
			.flags	= I2C_M_RD,
			.len	= length,
			.buf	= rxdata,
		},
	};

	ret = i2c_transfer(this_client->adapter, msgs, 2);
	
	return ret;
}
#endif 

int I2C_WriteByte(u8 addr, u8 para)
{
	int ret;
	u8 buf[3] =	{addr,para,0};

	struct i2c_msg msg[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 2,
			.buf	= buf,
		},
	};


	ret = i2c_transfer(this_client->adapter, msg, 1);

	return ret;
}

unsigned char I2C_ReadByte(u8 addr)
{
	int ret;
	u8 buf[2] = {addr,0};

//	buf[0] = addr;
	struct i2c_msg msgs[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= buf,
		},
		{
			.addr	= this_client->addr,
			.flags	= I2C_M_RD,
			.len	= 1,
			.buf	= buf,
		},
	};

    //msleep(1);
	ret = i2c_transfer(this_client->adapter, msgs, 2);

	return buf[0];
  
}

unsigned char I2C_ReadXByte( unsigned char *buf, unsigned char addr, unsigned short len)
{
    int ret,i;
	u8 rdbuf[512] = {addr,0};

	//rdbuf[0] = addr;
	struct i2c_msg msgs[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= rdbuf,
		},
		{
			.addr	= this_client->addr,
			.flags	= I2C_M_RD,
			.len	= len,
			.buf	= rdbuf,
		},
	};

    //msleep(1);
	ret = i2c_transfer(this_client->adapter, msgs, 2);
	if (ret < 0)
		pr_err("msg %s i2c read error: %d\n", __func__, ret);

    for(i = 0; i < len; i++)
	{
		buf[i] = rdbuf[i];
	}

    return ret;
}

void AW_Sleep(unsigned int msec)
{
	msleep(msec);
}


static void AW5306_ts_config_pins(void)
{

	gpio_direction_output(26, 1);
	//gpio_direction_input(sprd_3rdparty_gpio_tp_irq);	
	gpio_set_value(26, 0);
	//AW5306_irq=sprd_alloc_gpio_irq(sprd_3rdparty_gpio_tp_irq);

	msleep(10); //wait for stable
	AW5306_ts_pwron();   //jed
}

//jed add the virtual key support
#define TOUCH_VIRTUAL_KEYS
#ifdef TOUCH_VIRTUAL_KEYS

static ssize_t virtual_keys_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	TS_DBG("%s\n",__func__);
	//the two pin position is (48,531),(272,531)
	return snprintf(buf, 200,
	__stringify(EV_KEY) ":" __stringify(KEY_MENU)  ":81:1011:80:60"
	":" __stringify(EV_KEY) ":" __stringify(KEY_HOMEPAGE)   ":189:1011:80:60"
	":" __stringify(EV_KEY) ":" __stringify(KEY_BACK) ":351:1011:80:60"
	":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH)   ":459:1011:80:60"
	"\n");
}

static struct kobj_attribute virtual_keys_attr = {
    .attr = {
        .name = "virtualkeys.AW5306_ts",
        .mode = S_IRUGO,
    },
    .show = &virtual_keys_show,
};

static struct attribute *properties_attrs[] = {
    &virtual_keys_attr.attr,
    NULL
};

static struct attribute_group properties_attr_group = {
    .attrs = properties_attrs,
};

static void AW5306_ts_virtual_keys_init(void)
{
    int ret;
    struct kobject *properties_kobj;
	
    TS_DBG("%s\n",__func__);
	
    properties_kobj = kobject_create_and_add("board_properties", NULL);
    if (properties_kobj)
        ret = sysfs_create_group(properties_kobj,
                     &properties_attr_group);
    if (!properties_kobj || ret)
        pr_err("failed to create board_properties\n");    
}


#endif

static void AW5306_ts_release(void)
{
	struct AW5306_ts_data *data = i2c_get_clientdata(this_client);
	
#ifdef CONFIG_AW5306_MULTITOUCH	
	input_report_abs(data->input_dev, ABS_MT_PRESSURE, 0);
	input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
	input_report_key(data->input_dev, BTN_TOUCH, 0);
	input_mt_sync(data->input_dev);
#else
	input_report_abs(data->input_dev, ABS_PRESSURE, 0);
	input_report_key(data->input_dev, BTN_TOUCH, 0);
#endif
	input_sync(data->input_dev);
	TS_DBG("%s",__func__);
}

static int AW5306_read_data(void)
{	
	int i;
    int Pevent;

	struct AW5306_ts_data *data = i2c_get_clientdata(this_client);
	struct ts_event *event = &data->event;

    AW5306_TouchProcess();

	event->touch_point = AW5306_GetPointNum();

    TS_DBG("AW5306 Touch_point = %d\n", event->touch_point);

	for(i=0;i<event->touch_point;i++)
	{
		AW5306_GetPoint(&event->x[i],&event->y[i],&event->id[i],&Pevent,i);
		TS_DBG("AW5306 point %d = %d, y=%d, id = %d!!!!!\n", i,event->x[i],event->y[i],event->id[i]);
	}

	if(event->touch_point == 0)
	{
		if(tp_idlecnt <= 60)
		{
			tp_idlecnt++;
		}
		if(tp_idlecnt > 60)
		{
			tp_SlowMode = 1;
		}
	    if (event->pre_point != 0)
		{
			event->pre_point = 0;
	        AW5306_ts_release();
	    }
		return 1;
	}
	else
	{
		tp_SlowMode = 0;
		tp_idlecnt = 0;
		event->pre_point = event->touch_point;
    	event->pressure = 200;
		return 0;
	}
}

static void AW5306_report_value(void)
{
	struct AW5306_ts_data *data = i2c_get_clientdata(this_client);
	struct ts_event *event = &data->event;

#ifdef CONFIG_AW5306_MULTITOUCH
	switch(event->touch_point) {
		case 5:
			input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0x7F);
			input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x[4]);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y[4]);
			input_report_abs(data->input_dev, ABS_MT_PRESSURE, 0x7F);
			input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, event->id[4]);
			input_mt_sync(data->input_dev);
			
		case 4:
			input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0x7F);
			input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x[3]);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y[3]);
			input_report_abs(data->input_dev, ABS_MT_PRESSURE, 0x7F);
			input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, event->id[3]);
			input_mt_sync(data->input_dev);
			
		case 3:
			input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0x7F);
			input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x[2]);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y[2]);
				input_report_abs(data->input_dev, ABS_MT_PRESSURE, 0x7F);
				input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, event->id[2]);
			input_mt_sync(data->input_dev);
			
		case 2:
			input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0x7F);
			input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x[1]);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y[1]);
			input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, event->id[1]);
				input_report_abs(data->input_dev, ABS_MT_PRESSURE, 0x7F);
			input_mt_sync(data->input_dev);
			
		case 1:
			input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0x7F);
			input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x[0]);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y[0]);
			input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, event->id[0]);
				input_report_abs(data->input_dev, ABS_MT_PRESSURE, 0x7F);
			input_mt_sync(data->input_dev);
			
		default:
			break;
	}
	input_report_key(data->input_dev, BTN_TOUCH, 1);
#else	/* CONFIG_AW5306_MULTITOUCH*/
	if (event->touch_point == 1) {
		input_report_abs(data->input_dev, ABS_X, event->x[0]);
		input_report_abs(data->input_dev, ABS_Y, event->y[0]);
		input_report_abs(data->input_dev, ABS_PRESSURE, 1);
	}
	input_report_key(data->input_dev, BTN_TOUCH, 1);
#endif	/* CONFIG_AW5306_MULTITOUCH*/
	input_sync(data->input_dev);
}	/*end AW5306_report_value*/

static void AW5306_ts_pen_irq_work(struct work_struct *work)
{
#if 1
	int ret = -1;

	ret = AW5306_read_data();	
	if (ret == 0) {	
		AW5306_report_value();
	}
#endif
 //   enable_irq(this_client->irq);

}

#if 0
static irqreturn_t AW5306_ts_interrupt(int irq, void *dev_id)
{

	struct AW5306_ts_data *AW5306_ts = (struct AW5306_ts_data *)dev_id;

    disable_irq_nosync(this_client->irq);
	if (!work_pending(&AW5306_ts->pen_event_work)) {
		queue_work(AW5306_ts->ts_workqueue, &AW5306_ts->pen_event_work);
	}
	return IRQ_HANDLED;
}
#endif

void AW5306_tpd_polling(unsigned long nouse)
{
    struct AW5306_ts_data *data = i2c_get_clientdata(this_client);

    if (!work_pending(&data->pen_event_work)) {
    queue_work(data->ts_workqueue, &data->pen_event_work);
    }
	if(tp_SlowMode)
	{
		data->touch_timer.expires = jiffies + HZ/SLOW_FRAME;
	}
	else
	{
		data->touch_timer.expires = jiffies + HZ/FAST_FRAME;
	}
	add_timer(&data->touch_timer);
}


static void AW5306_ts_suspend(struct early_suspend *handler)
{
	struct AW5306_ts_data *data = i2c_get_clientdata(this_client);

	if(suspend_flag != 1)
	{
	   	AW5306_Sleep();
	   	TS_DBG("==AW5306_ts_suspend=\n");
		
	    del_timer(&data->touch_timer);
		suspend_flag = 1;
	}
}

static void AW5306_ts_resume(struct early_suspend *handler)
{	
	struct AW5306_ts_data *data = i2c_get_clientdata(this_client);

	if(suspend_flag != 0)
	{
		AW5306_TP_Reinit();
		tp_idlecnt = 0;
		tp_SlowMode = 1;
		suspend_flag = 0;
		TS_DBG("AW5306 WAKE UP!!!");
		data->touch_timer.expires = jiffies + HZ/FAST_FRAME;
		add_timer(&data->touch_timer);
		
	}
}

static int AW5306_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct AW5306_ts_data *AW5306_ts;
	struct input_dev *input_dev;
	int err = 0;
	unsigned char reg_value; 
	
	
	TS_DBG("==AW5306_ts_probe=\n");
	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}

	TS_DBG("==kzalloc=");
	AW5306_ts = kzalloc(sizeof(*AW5306_ts), GFP_KERNEL);
	if (!AW5306_ts)	{
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}

	client->addr = 0x38;
	
//jed
#ifdef TOUCH_VIRTUAL_KEYS
	AW5306_ts_virtual_keys_init();
#endif

	this_client = client;
	i2c_set_clientdata(client, AW5306_ts);
//	sc8810_i2c_set_clk(2,400000);

	TS_DBG("I2C addr=%x", client->addr);
	
	
	reg_value = I2C_ReadByte(0x01);
    TS_DBG("AW5306 chip ID = 0x%x", reg_value);
	if(reg_value != 0xA8)
	{
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}

	INIT_WORK(&AW5306_ts->pen_event_work, AW5306_ts_pen_irq_work);

	AW5306_ts->ts_workqueue = create_singlethread_workqueue(dev_name(&client->dev));
	if (!AW5306_ts->ts_workqueue) {
		err = -ESRCH;
		goto exit_create_singlethread;
	}
//	client->irq = AW5306_irq;  //jed
//	TS_DBG("==request_irq=\n");
//	TS_DBG("%s IRQ number is %d", client->name, client->irq);
//	err = request_irq(client->irq, AW5306_ts_interrupt, IRQF_TRIGGER_FALLING, client->name, AW5306_ts);
//	if (err < 0) {
//		dev_err(&client->dev, "AW5306_probe: request irq failed\n");
//		goto exit_irq_request_failed;
//	}

//	__gpio_as_irq_fall_edge(pdata->intr);		//
//	disable_irq(this_client->irq);
//	disable_irq(client->irq);

//	TS_DBG("==input_allocate_device=\n");
	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		dev_err(&client->dev, "failed to allocate input device\n");
		goto exit_input_dev_alloc_failed;
	}
	
	AW5306_ts->input_dev = input_dev;

/* //sym add
	set_bit(EV_KEY, input_dev->evbit);
	set_bit(EV_ABS, input_dev->evbit);
	set_bit(BTN_MISC, input_dev->keybit);
	set_bit(BTN_TOUCH, input_dev->keybit);

	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0,
			     pdata->x_max, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0,
			     pdata->y_max, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0,
			     CFG_MAX_TOUCH_POINTS, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, FT_PRESS, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, FT_PRESS, 0, 0);
*/

#ifdef CONFIG_AW5306_MULTITOUCH
	set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit);
	set_bit(ABS_MT_POSITION_X, input_dev->absbit);
	set_bit(ABS_MT_POSITION_Y, input_dev->absbit);
	//set_bit(ABS_MT_WIDTH_MAJOR, input_dev->absbit);
	//jed add this four lines
    set_bit(KEY_MENU,  input_dev->keybit);
	set_bit(KEY_BACK,  input_dev->keybit);
	set_bit(KEY_HOME,  input_dev->keybit);
	set_bit(KEY_SEARCH,  input_dev->keybit);
//sym add
	set_bit(EV_KEY, input_dev->evbit);
	set_bit(EV_ABS, input_dev->evbit);
	set_bit(BTN_MISC, input_dev->keybit);
	set_bit(BTN_TOUCH, input_dev->keybit);
//sym end	
	input_set_abs_params(input_dev,ABS_MT_POSITION_X, 0, SCREEN_MAX_X, 0, 0);
	input_set_abs_params(input_dev,ABS_MT_POSITION_Y, 0, SCREEN_MAX_Y, 0, 0);
	input_set_abs_params(input_dev,ABS_MT_TOUCH_MAJOR, 0, PRESS_MAX, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0, 5, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, PRESS_MAX, 0, 0);

#else
	set_bit(ABS_X, input_dev->absbit);
	set_bit(ABS_Y, input_dev->absbit);
	set_bit(ABS_PRESSURE, input_dev->absbit);
	set_bit(BTN_TOUCH, input_dev->keybit);

	input_set_abs_params(input_dev, ABS_X, 0, SCREEN_MAX_X, 0, 0);
	input_set_abs_params(input_dev, ABS_Y, 0, SCREEN_MAX_Y, 0, 0);
	input_set_abs_params(input_dev,
			     ABS_PRESSURE, 0, PRESS_MAX, 0 , 0);
#endif

	set_bit(EV_ABS, input_dev->evbit);
	set_bit(EV_KEY, input_dev->evbit);

	input_dev->name		= AW5306_TS_NAME;		//dev_name(&client->dev)
	err = input_register_device(input_dev);
	if (err) {
		dev_err(&client->dev,
		"AW5306_ts_probe: failed to register input device: %s\n",
		dev_name(&client->dev));
		goto exit_input_register_device_failed;
	}

	TS_DBG("==register_early_suspend =");
	AW5306_ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	AW5306_ts->early_suspend.suspend = AW5306_ts_suspend;
	AW5306_ts->early_suspend.resume	= AW5306_ts_resume;
	register_early_suspend(&AW5306_ts->early_suspend);

    msleep(50);

	AW5306_create_sysfs(client);

 //   enable_irq(client->irq);
    
    AW5306_TP_Init();

    AW5306_ts->touch_timer.function = AW5306_tpd_polling;
	AW5306_ts->touch_timer.data = 0;
	init_timer(&AW5306_ts->touch_timer);
	AW5306_ts->touch_timer.expires = jiffies + HZ*5;
	add_timer(&AW5306_ts->touch_timer);

	TS_DBG("==probe over =\n");
    return 0;

exit_input_register_device_failed:
	input_free_device(input_dev);
exit_input_dev_alloc_failed:
	free_irq(client->irq, AW5306_ts);
//exit_irq_request_failed:
//	cancel_work_sync(&AW5306_ts->pen_event_work);
//	destroy_workqueue(AW5306_ts->ts_workqueue);
exit_create_singlethread:
	TS_DBG("==singlethread error =\n");
	i2c_set_clientdata(client, NULL);
	kfree(AW5306_ts);
exit_alloc_data_failed:
exit_check_functionality_failed:
	gpio_free(AW5306_ts_setup.irq);//sprd_free_gpio_irq(AW5306_ts_setup.irq);
	return err;
}
/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static int __devexit AW5306_ts_remove(struct i2c_client *client)
{

	struct AW5306_ts_data *AW5306_ts = i2c_get_clientdata(client);

	TS_DBG("==AW5306_ts_remove=\n");
	
	unregister_early_suspend(&AW5306_ts->early_suspend);
//	free_irq(client->irq, AW5306_ts);   //sym modify
//	free_irq(AW5306_ts_setup.irq);//sprd_free_gpio_irq(AW5306_ts_setup.irq);
	input_unregister_device(AW5306_ts->input_dev);
	kfree(AW5306_ts);
	cancel_work_sync(&AW5306_ts->pen_event_work);
	destroy_workqueue(AW5306_ts->ts_workqueue);
	i2c_set_clientdata(client, NULL);
	return 0;
}

static const struct i2c_device_id AW5306_ts_id[] = {
	{ AW5306_TS_NAME, 0 },{ }
};


MODULE_DEVICE_TABLE(i2c, AW5306_ts_id);

static struct i2c_driver AW5306_ts_driver = {
	.probe		= AW5306_ts_probe,
	.remove		= __devexit_p(AW5306_ts_remove),
	.id_table	= AW5306_ts_id,
	.driver	= {
		.name	= AW5306_TS_NAME,
		.owner	= THIS_MODULE,
	},
};

#if I2C_BOARD_INFO_METHOD
/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static int __init AW5306_ts_init(void)
{
	int ret;
	TS_DBG("==AW5306_ts_init==\n");
   AW5306_ts_config_pins();//jed
	ret = i2c_add_driver(&AW5306_ts_driver);
	return ret;
}

/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static void __exit AW5306_ts_exit(void)
{
	TS_DBG("==AW5306_ts_exit==\n");
	i2c_del_driver(&AW5306_ts_driver);
}
#else //register i2c device&driver dynamicly

int sprd_add_i2c_device(struct sprd_i2c_setup_data *i2c_set_data, struct i2c_driver *driver)
{
	struct i2c_board_info info;
	struct i2c_adapter *adapter;
	struct i2c_client *client;
	int ret,err;


	TS_DBG("%s : i2c_bus=%d; slave_address=0x%x; i2c_name=%s",__func__,i2c_set_data->i2c_bus, \
		    i2c_set_data->i2c_address, i2c_set_data->type);

	memset(&info, 0, sizeof(struct i2c_board_info));
	info.addr = i2c_set_data->i2c_address;
	strlcpy(info.type, i2c_set_data->type, I2C_NAME_SIZE);
	if(i2c_set_data->irq > 0)
		info.irq = i2c_set_data->irq;

	adapter = i2c_get_adapter( i2c_set_data->i2c_bus);
	if (!adapter) {
		TS_DBG("%s: can't get i2c adapter %d\n",
			__func__,  i2c_set_data->i2c_bus);
		err = -ENODEV;
		goto err_driver;
	}

	client = i2c_new_device(adapter, &info);
	if (!client) {
		TS_DBG("%s:  can't add i2c device at 0x%x\n",
			__func__, (unsigned int)info.addr);
		err = -ENODEV;
		goto err_driver;
	}

	i2c_put_adapter(adapter);

	ret = i2c_add_driver(driver);
	if (ret != 0) {
		TS_DBG("%s: can't add i2c driver\n", __func__);
		err = -ENODEV;
		goto err_driver;
	}	

	return 0;

err_driver:
	return err;
}

void sprd_del_i2c_device(struct i2c_client *client, struct i2c_driver *driver)
{
	TS_DBG("%s : slave_address=0x%x; i2c_name=%s",__func__, client->addr, client->name);
	i2c_unregister_device(client);
	i2c_del_driver(driver);
}

static int __init AW5306_ts_init(void)
{	
	TS_DBG("%s\n", __func__);
	
	AW5306_irq=AW5306_ts_config_pins();
	AW5306_ts_setup.i2c_bus = 0;
	AW5306_ts_setup.i2c_address = AW5306_TS_ADDR;
	strcpy (AW5306_ts_setup.type,AW5306_TS_NAME);
	AW5306_ts_setup.irq = AW5306_irq;
	return sprd_add_i2c_device(&AW5306_ts_setup, &AW5306_ts_driver);
}

static void __exit AW5306_ts_exit(void)
{
	TS_DBG("%s\n", __func__);
	sprd_del_i2c_device(this_client, &AW5306_ts_driver);
	
}
#endif

module_init(AW5306_ts_init);
module_exit(AW5306_ts_exit);

MODULE_AUTHOR("<wuhaijun@AWINIC.com>");
MODULE_DESCRIPTION("AWINIC AW5306 TouchScreen driver");
MODULE_LICENSE("GPL");
