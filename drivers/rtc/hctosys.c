/*
 * RTC subsystem, initialize system time on startup
 *
 * Copyright (C) 2005 Tower Technologies
 * Author: Alessandro Zummo <a.zummo@towertech.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/rtc.h>
//delete from path of lijiawu by lishangyou 20130123 (start)
/*
//add by lihua BUG_ID:QELS-234 2012.10.18 (start)
//modify by xianglong BUG_ID:QELS-3048 2012.12.24 (start)
//#define DEFAULT_SYSTEM_YEAR		112		//year+1900 (e.g: 112+1900=2012)
#define DEFAULT_SYSTEM_YEAR		113		//year+1900 (e.g: 113+1900=2013)
//modify by xianglong BUG_ID:QELS-3048 2012.12.24 (end)
#define DEFAULT_SYSTEM_MONTH	0		//month+1   (e.g: 0+1=1)
#define DEFAULT_SYSTEM_DAY		1		//day       (e.g: 1=1)
//add by lihua BUG_ID:QELS-234 2012.10.18 (end)
*/
//delete from path of lijiawu by lishangyou 20130123 (end)

/* IMPORTANT: the RTC only stores whole seconds. It is arbitrary
 * whether it stores the most close value or the value with partial
 * seconds truncated. However, it is important that we use it to store
 * the truncated value. This is because otherwise it is necessary,
 * in an rtc sync function, to read both xtime.tv_sec and
 * xtime.tv_nsec. On some processors (i.e. ARM), an atomic read
 * of >32bits is not possible. So storing the most close value would
 * slow down the sync API. So here we have the truncated value and
 * the best guess is to add 0.5s.
 */

int rtc_hctosys_ret = -ENODEV;

int rtc_hctosys(void)
{
	int err = -ENODEV;
	struct rtc_time tm;
	struct timespec tv = {
		.tv_nsec = NSEC_PER_SEC >> 1,
	};
	struct rtc_device *rtc = rtc_class_open(CONFIG_RTC_HCTOSYS_DEVICE);

	if (rtc == NULL) {
		pr_err("%s: unable to open rtc device (%s)\n",
			__FILE__, CONFIG_RTC_HCTOSYS_DEVICE);
		goto err_open;
	}

	err = rtc_read_time(rtc, &tm);
	if (err) {
		dev_err(rtc->dev.parent,
			"hctosys: unable to read the hardware clock\n");
		goto err_read;

	}

	err = rtc_valid_tm(&tm);
	if (err) {
		dev_err(rtc->dev.parent,
			"hctosys: invalid date/time\n");
		goto err_invalid;
	}
	//delete from path of lijiawu by lishangyou 20130123 (start)
/*
	//add by lihua BUG_ID:QELS-234 2012.10.18 (start)
	if (tm.tm_year == 80) {
		// set the new default rtc time
		tm.tm_year = DEFAULT_SYSTEM_YEAR;
		tm.tm_mon  = DEFAULT_SYSTEM_MONTH;
		tm.tm_mday = DEFAULT_SYSTEM_DAY;
		err = rtc_set_time(rtc, &tm);
		if (err) {
			dev_err(rtc->dev.parent,
				"hctosys: unable to set the hardware clock\n");
			goto err_read;
		}
		// read the rtc time again
		err = rtc_read_time(rtc, &tm);
		if (err) {
			dev_err(rtc->dev.parent,
				"hctosys: unable to read the hardware clock\n");
			goto err_read;
		}
		// check if the readtime is valid
		err = rtc_valid_tm(&tm);
		if (err) {
			dev_err(rtc->dev.parent,
				"hctosys: invalid date/time\n");
			goto err_invalid;
		}
	}
	//add by lihua BUG_ID:QELS-234 2012.10.18 (end)
*/
	//delete from path of lijiawu by lishangyou 20130123 (end)
	rtc_tm_to_time(&tm, &tv.tv_sec);

	do_settimeofday(&tv);

	dev_info(rtc->dev.parent,
		"setting system clock to "
		"%d-%02d-%02d %02d:%02d:%02d UTC (%u)\n",
		tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		tm.tm_hour, tm.tm_min, tm.tm_sec,
		(unsigned int) tv.tv_sec);

err_invalid:
err_read:
	rtc_class_close(rtc);

err_open:
	rtc_hctosys_ret = err;

	return err;
}

late_initcall(rtc_hctosys);
