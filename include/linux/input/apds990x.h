#include <linux/earlysuspend.h>

struct apds990x_data {
	struct i2c_client *client;
	struct mutex update_lock;
	struct delayed_work	dwork;	/* for PS interrupt */
	struct delayed_work    als_dwork; /* for ALS polling */
	struct input_dev *input_dev_als;
	struct input_dev *input_dev_ps;
	struct early_suspend early_suspend;

	unsigned int enable;
	unsigned int atime;
	unsigned int ptime;
	unsigned int wtime;
	unsigned int ailt;
	unsigned int aiht;
	unsigned int pilt;
	unsigned int piht;
	unsigned int pers;
	unsigned int config;
	unsigned int ppcount;
	unsigned int control;

	/* control flag from HAL */
	unsigned int enable_ps_sensor;
	unsigned int enable_als_sensor;

	/* PS parameters */
	unsigned int ps_threshold;
	unsigned int ps_hysteresis_threshold; /* always lower than ps_threshold */
	unsigned int ps_detection;		/* 0 = near-to-far; 1 = far-to-near */
	unsigned int ps_data;			/* to store PS data */

	/* ALS parameters */
	unsigned int als_threshold_l;	/* low threshold */
	unsigned int als_threshold_h;	/* high threshold */
	unsigned int als_data;			/* to store ALS data */

	unsigned int als_gain;			/* needed for Lux calculation */
	unsigned int als_poll_delay;	/* needed for light sensor polling : micro-second (us) */
	unsigned int als_atime;			/* storage for als integratiion time */

//	int (*power_on)(void);
	
};

	
struct apds990x_platform_data 
	{	
		int (*power_onoff)(int onoff);	
		int irq;  /* proximity/light-sensor- external irq*/	
		unsigned int ps_det_thld;	
		unsigned int ps_hsyt_thld;	
		unsigned int als_hsyt_thld;
	};

#define ERROR_CHECK(x)                                                  \
	do {								\
		if (0 != x) {					\
			printk(KERN_ERR "[APDS990X][sym]%s|%s|%d returning %d\n",		\
				__FILE__, __func__, __LINE__, x);	\
			return x;					\
		}							\
	} while (0)

#define DEBUG_APDS990X		
#if defined(DEBUG_APDS990X)
#define sym_dbg(format, arg...)		\
	printk("[Sensor_APDS990X][sym] [%-16s]\n "format , __func__,  ## arg)
#else
#define sym_dbg(format, arg...)
#endif

