/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio_event.h>
#include <linux/usb/android.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/i2c.h>
#include <linux/android_pmem.h>
#include <linux/bootmem.h>
#include <linux/mfd/marimba.h>
#include <linux/power_supply.h>
#include <linux/input/rmi_platformdata.h>
#include <linux/input/rmi_i2c.h>
#include <linux/i2c/atmel_mxt_ts.h>
#include <linux/regulator/consumer.h>
#include <linux/memblock.h>
#include <linux/input/ft5x06_ts.h>
#include <linux/msm_adc.h>
#include <linux/fmem.h>
#include <linux/regulator/msm-gpio-regulator.h>
#include <linux/ion.h>
#include <asm/mach/mmc.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/hardware/gic.h>
#include <mach/board.h>
#include <mach/msm_iomap.h>
#include <mach/msm_hsusb.h>
#include <mach/rpc_hsusb.h>
#include <mach/rpc_pmapp.h>
#include <mach/usbdiag.h>
#include <mach/msm_memtypes.h>
#include <mach/msm_serial_hs.h>
#include <mach/pmic.h>
#include <mach/socinfo.h>
#include <mach/vreg.h>
#include <mach/rpc_pmapp.h>
#include <mach/msm_battery.h>
#include <mach/rpc_server_handset.h>
#include <mach/socinfo.h>
#include "board-msm7x27a-regulator.h"
#include "devices.h"
#include "devices-msm7x2xa.h"
#include "pm.h"
#include "timer.h"
#include "pm-boot.h"
#include "board-msm7x27a-regulator.h"
#include "board-msm7627a.h"

//Start=====Allen
#include <mach/msm_smsm.h>
#include <linux/board-ragentek-cfg.h>
#include <linux/string.h>
#include <mach/msm_smsm.h>
#include <../../../../build/buildplus/target/QRDExt_target.h>
//End=====Allen

#define PMEM_KERNEL_EBI1_SIZE	0x3A000
#define MSM_PMEM_AUDIO_SIZE	0x1F4000
#define BAHAMA_SLAVE_ID_FM_REG 0x02
#define FM_GPIO	83
#define BT_PCM_BCLK_MODE  0x88
#define BT_PCM_DIN_MODE   0x89
#define BT_PCM_DOUT_MODE  0x8A
#define BT_PCM_SYNC_MODE  0x8B
#define FM_I2S_SD_MODE    0x8E
#define FM_I2S_WS_MODE    0x8F
#define FM_I2S_SCK_MODE   0x90
#define I2C_PIN_CTL       0x15
#define I2C_NORMAL        0x40


static struct platform_device msm_wlan_ar6000_pm_device = {
	.name           = "wlan_ar6000_pm_dev",
	.id             = -1,
};

uint8_t current_qcomm_mode = MSM_BOOT_NORMAL;

static struct msm_gpio qup_i2c_gpios_io[] = {
	{ GPIO_CFG(60, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
		"qup_scl" },
	{ GPIO_CFG(61, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
		"qup_sda" },
	{ GPIO_CFG(131, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
		"qup_scl" },
	{ GPIO_CFG(132, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
		"qup_sda" },
};

static struct msm_gpio qup_i2c_gpios_hw[] = {
	{ GPIO_CFG(60, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
		"qup_scl" },
	{ GPIO_CFG(61, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
		"qup_sda" },
	{ GPIO_CFG(131, 2, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
		"qup_scl" },
	{ GPIO_CFG(132, 2, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
		"qup_sda" },
};

static void gsbi_qup_i2c_gpio_config(int adap_id, int config_type)
{
	int rc;

	if (adap_id < 0 || adap_id > 1)
		return;

	/* Each adapter gets 2 lines from the table */
	if (config_type)
		rc = msm_gpios_request_enable(&qup_i2c_gpios_hw[adap_id*2], 2);
	else
		rc = msm_gpios_request_enable(&qup_i2c_gpios_io[adap_id*2], 2);
	if (rc < 0)
		pr_err("QUP GPIO request/enable failed: %d\n", rc);
}

//Start === Allen
static void gsbi_qup_i2c_gpio_config_for_recovery(int adap_id, int config_type)
{
	int rc;

	if (adap_id < 0 || adap_id > 1)
		return;

	/* Each adapter gets 2 lines from the table */
	if (config_type){
		rc = gpio_tlmm_config(qup_i2c_gpios_hw[adap_id*2].gpio_cfg, GPIO_CFG_ENABLE);
		rc = gpio_tlmm_config(qup_i2c_gpios_hw[adap_id*2+1].gpio_cfg, GPIO_CFG_ENABLE);

	}else{
		rc = gpio_tlmm_config(qup_i2c_gpios_io[adap_id*2].gpio_cfg, GPIO_CFG_ENABLE);
		rc = gpio_tlmm_config(qup_i2c_gpios_io[adap_id*2+1].gpio_cfg, GPIO_CFG_ENABLE);
	}

	if (rc < 0)
		pr_err("QUP GPIO request/enable failed: %d\n", rc);
}
//End === Allen

static struct msm_i2c_platform_data msm_gsbi0_qup_i2c_pdata = {
	.clk_freq		= 400000,
	.msm_i2c_config_gpio	= gsbi_qup_i2c_gpio_config,
	//Start===Allen
	.msm_i2c_config_gpi_for_recovery = gsbi_qup_i2c_gpio_config_for_recovery,	
	.pri_clk = 60,
	.pri_dat = 61,
	//End===Allen
};

static struct msm_i2c_platform_data msm_gsbi1_qup_i2c_pdata = {
	.clk_freq		= 400000,
	.msm_i2c_config_gpio	= gsbi_qup_i2c_gpio_config,
	//Start===Allen
	.msm_i2c_config_gpi_for_recovery = gsbi_qup_i2c_gpio_config_for_recovery,
	.pri_clk = 131,
	.pri_dat = 132,
	//End===Allen
};

#ifdef CONFIG_ARCH_MSM7X27A
#define MSM_PMEM_MDP_SIZE       0x2300000
#define MSM_PMEM_ADSP_SIZE      0x3700000 // 50M //0x1200000   //18MB
#define CAMERA_ZSL_SIZE		(SZ_1M * 60)

#ifdef CONFIG_ION_MSM
#define MSM_ION_HEAP_NUM	4
static struct platform_device ion_dev;
static int msm_ion_camera_size;
static int msm_ion_audio_size;
static int msm_ion_sf_size;
#endif
#endif

static struct android_usb_platform_data android_usb_pdata = {
	.update_pid_and_serial_num = usb_diag_update_pid_and_serial_num,
	.cdrom = 1,
};

static struct platform_device android_usb_device = {
	.name	= "android_usb",
	.id	= -1,
	.dev	= {
		.platform_data = &android_usb_pdata,
	},
};

#ifdef CONFIG_USB_EHCI_MSM_72K
static void msm_hsusb_vbus_power(unsigned phy_info, int on)
{
	int rc = 0;
	unsigned gpio;

	gpio = QRD_GPIO_HOST_VBUS_EN;

	rc = gpio_request(gpio,	"i2c_host_vbus_en");
	if (rc < 0) {
		pr_err("failed to request %d GPIO\n", gpio);
		return;
	}
	gpio_direction_output(gpio, !!on);
	gpio_set_value_cansleep(gpio, !!on);
	gpio_free(gpio);
}

static struct msm_usb_host_platform_data msm_usb_host_pdata = {
	.phy_info       = (USB_PHY_INTEGRATED | USB_PHY_MODEL_45NM),
};

static void __init msm7627a_init_host(void)
{
	msm_add_host(0, &msm_usb_host_pdata);
}
#endif

#ifdef CONFIG_USB_MSM_OTG_72K
static int hsusb_rpc_connect(int connect)
{
	if (connect)
		return msm_hsusb_rpc_connect();
	else
		return msm_hsusb_rpc_close();
}

static struct regulator *reg_hsusb;
static int msm_hsusb_ldo_init(int init)
{
	int rc = 0;

	if (init) {
		reg_hsusb = regulator_get(NULL, "usb");
		if (IS_ERR(reg_hsusb)) {
			rc = PTR_ERR(reg_hsusb);
			pr_err("%s: could not get regulator: %d\n",
					__func__, rc);
			goto out;
		}

		rc = regulator_set_voltage(reg_hsusb, 3300000, 3300000);
		if (rc) {
			pr_err("%s: could not set voltage: %d\n",
					__func__, rc);
			goto reg_free;
		}

		return 0;
	}
	/* else fall through */
reg_free:
	regulator_put(reg_hsusb);
out:
	reg_hsusb = NULL;
	return rc;
}

static int msm_hsusb_ldo_enable(int enable)
{
	static int ldo_status;

	if (IS_ERR_OR_NULL(reg_hsusb))
		return reg_hsusb ? PTR_ERR(reg_hsusb) : -ENODEV;

	if (ldo_status == enable)
		return 0;

	ldo_status = enable;

	return enable ?
		regulator_enable(reg_hsusb) :
		regulator_disable(reg_hsusb);
}

#ifndef CONFIG_USB_EHCI_MSM_72K
static int msm_hsusb_pmic_notif_init(void (*callback)(int online), int init)
{
	int ret = 0;

	if (init)
		ret = msm_pm_app_rpc_init(callback);
	else
		msm_pm_app_rpc_deinit(callback);

	return ret;
}
#endif

static struct msm_otg_platform_data msm_otg_pdata = {
#ifndef CONFIG_USB_EHCI_MSM_72K
	.pmic_vbus_notif_init	 = msm_hsusb_pmic_notif_init,
#else
	.vbus_power		 = msm_hsusb_vbus_power,
#endif
	.rpc_connect		 = hsusb_rpc_connect,
	.pemp_level		 = PRE_EMPHASIS_WITH_20_PERCENT,
	.cdr_autoreset		 = CDR_AUTO_RESET_DISABLE,
	.drv_ampl		 = HS_DRV_AMPLITUDE_DEFAULT,
	.se1_gating		 = SE1_GATING_DISABLE,
	.ldo_init		 = msm_hsusb_ldo_init,
	.ldo_enable		 = msm_hsusb_ldo_enable,
	.chg_init		 = hsusb_chg_init,
	.chg_connected		 = hsusb_chg_connected,
	.chg_vbus_draw		 = hsusb_chg_vbus_draw,
};
#endif

static struct msm_hsusb_gadget_platform_data msm_gadget_pdata = {
	.is_phy_status_timer_on = 1,
	.prop_chg = 0,
};

#ifdef CONFIG_SERIAL_MSM_HS
static struct msm_serial_hs_platform_data msm_uart_dm1_pdata = {
	.inject_rx_on_wakeup	= 1,
	.rx_to_inject		= 0xFD,
};
#endif
static struct msm_pm_platform_data msm7627a_pm_data[MSM_PM_SLEEP_MODE_NR] = {
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE] = {
					.idle_supported = 1,
					.suspend_supported = 1,
					.idle_enabled = 1,
					.suspend_enabled = 1,
					.latency = 16000,
					.residency = 20000,
	},
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN] = {
					.idle_supported = 1,
					.suspend_supported = 1,
					.idle_enabled = 1,
					.suspend_enabled = 1,
					.latency = 12000,
					.residency = 20000,
	},
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT] = {
					.idle_supported = 1,
					.suspend_supported = 1,
					.idle_enabled = 0,
					.suspend_enabled = 1,
					.latency = 2000,
					.residency = 0,
	},
	[MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT] = {
					.idle_supported = 1,
					.suspend_supported = 1,
					.idle_enabled = 1,
					.suspend_enabled = 1,
					.latency = 2,
					.residency = 0,
	},
};

static struct msm_pm_boot_platform_data msm_pm_boot_pdata __initdata = {
	.mode = MSM_PM_BOOT_CONFIG_RESET_VECTOR_PHYS,
	.p_addr = 0,
};

/* 8625 PM platform data */
static struct msm_pm_platform_data msm8625_pm_data[MSM_PM_SLEEP_MODE_NR * 2] = {
	/* CORE0 entries */
	[MSM_PM_MODE(0, MSM_PM_SLEEP_MODE_POWER_COLLAPSE)] = {
					.idle_supported = 1,
					.suspend_supported = 1,
					.idle_enabled = 0,
					.suspend_enabled = 0,
					.latency = 16000,
					.residency = 20000,
	},

	[MSM_PM_MODE(0, MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN)] = {
					.idle_supported = 1,
					.suspend_supported = 1,
					.idle_enabled = 0,
					.suspend_enabled = 0,
					.latency = 12000,
					.residency = 20000,
	},

	/* picked latency & redisdency values from 7x30 */
	[MSM_PM_MODE(0, MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE)] = {
					.idle_supported = 1,
					.suspend_supported = 1,
					.idle_enabled = 0,
					.suspend_enabled = 0,
					.latency = 500,
					.residency = 6000,
	},

	[MSM_PM_MODE(0, MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT)] = {
					.idle_supported = 1,
					.suspend_supported = 1,
					.idle_enabled = 1,
					.suspend_enabled = 1,
					.latency = 2,
					.residency = 10,
	},

	/* picked latency & redisdency values from 7x30 */
	[MSM_PM_MODE(1, MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE)] = {
					.idle_supported = 1,
					.suspend_supported = 1,
					.idle_enabled = 0,
					.suspend_enabled = 0,
					.latency = 500,
					.residency = 6000,
	},

	[MSM_PM_MODE(1, MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT)] = {
					.idle_supported = 1,
					.suspend_supported = 1,
					.idle_enabled = 1,
					.suspend_enabled = 1,
					.latency = 2,
					.residency = 10,
	},

};

static struct msm_pm_boot_platform_data msm_pm_8625_boot_pdata __initdata = {
	.mode = MSM_PM_BOOT_CONFIG_REMAP_BOOT_ADDR,
	.v_addr = MSM_CFG_CTL_BASE,
};

static struct android_pmem_platform_data android_pmem_adsp_pdata = {
	.name = "pmem_adsp",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 1,
	.memory_type = MEMTYPE_EBI1,
	.request_region = request_fmem_c_region,
	.release_region = release_fmem_c_region,
	.reusable = 1,
};

static struct platform_device android_pmem_adsp_device = {
	.name = "android_pmem",
	.id = 1,
	.dev = { .platform_data = &android_pmem_adsp_pdata },
};

static unsigned pmem_mdp_size = MSM_PMEM_MDP_SIZE;
static int __init pmem_mdp_size_setup(char *p)
{
	pmem_mdp_size = memparse(p, NULL);
	return 0;
}

early_param("pmem_mdp_size", pmem_mdp_size_setup);

static unsigned pmem_adsp_size = MSM_PMEM_ADSP_SIZE;
static int __init pmem_adsp_size_setup(char *p)
{
	pmem_adsp_size = memparse(p, NULL);
	return 0;
}

early_param("pmem_adsp_size", pmem_adsp_size_setup);

#define SND(desc, num) { .name = #desc, .id = num }
static struct snd_endpoint snd_endpoints_list[] = {
	SND(HANDSET, 0),
	SND(MONO_HEADSET, 2),
	SND(HEADSET, 3),
	SND(SPEAKER, 6),
	SND(TTY_HEADSET, 8),
	SND(TTY_VCO, 9),
	SND(TTY_HCO, 10),
	SND(BT, 12),
	SND(IN_S_SADC_OUT_HANDSET, 16),
	SND(IN_S_SADC_OUT_SPEAKER_PHONE, 25),
	SND(FM_DIGITAL_STEREO_HEADSET, 26),
	SND(FM_DIGITAL_SPEAKER_PHONE, 27),
	SND(FM_DIGITAL_BT_A2DP_HEADSET, 28),
	SND(STEREO_HEADSET_AND_SPEAKER, 31),
	SND(CURRENT, 0x7FFFFFFE),
	SND(FM_ANALOG_STEREO_HEADSET, 35),
	SND(FM_ANALOG_STEREO_HEADSET_CODEC, 36),
	SND(NO_MIC_HEADSET, 40),//add by fangxing for headphone with main mic input
    SND(HANDSET_LOOP,41),//add by fangxing for cit loop
    SND(HEADSET_LOOP,42),//add by fangxing for cit loop
};
#undef SND

static struct msm_snd_endpoints msm_device_snd_endpoints = {
	.endpoints = snd_endpoints_list,
	.num = sizeof(snd_endpoints_list) / sizeof(struct snd_endpoint)
};

static struct platform_device msm_device_snd = {
	.name = "msm_snd",
	.id = -1,
	.dev    = {
		.platform_data = &msm_device_snd_endpoints
	},
};

#define DEC0_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC1_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC2_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC3_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
	(1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
	(1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
	(1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
	(1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
	(1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC4_FORMAT (1<<MSM_ADSP_CODEC_MIDI)

static unsigned int dec_concurrency_table[] = {
	/* Audio LP */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DMA)), 0,
	0, 0, 0,

	/* Concurrency 1 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	 /* Concurrency 2 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 3 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 4 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 5 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 6 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|
			(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	0, 0, 0, 0,

	/* Concurrency 7 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),
};

#define DEC_INFO(name, queueid, decid, nr_codec) { .module_name = name, \
	.module_queueid = queueid, .module_decid = decid, \
	.nr_codec_support = nr_codec}

static struct msm_adspdec_info dec_info_list[] = {
	DEC_INFO("AUDPLAY0TASK", 13, 0, 11), /* AudPlay0BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY1TASK", 14, 1, 11),  /* AudPlay1BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY2TASK", 15, 2, 11),  /* AudPlay2BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY3TASK", 16, 3, 11),  /* AudPlay3BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY4TASK", 17, 4, 1),  /* AudPlay4BitStreamCtrlQueue */
};

static struct msm_adspdec_database msm_device_adspdec_database = {
	.num_dec = ARRAY_SIZE(dec_info_list),
	.num_concurrency_support = (ARRAY_SIZE(dec_concurrency_table) / \
					ARRAY_SIZE(dec_info_list)),
	.dec_concurrency_table = dec_concurrency_table,
	.dec_info_list = dec_info_list,
};

static struct platform_device msm_device_adspdec = {
	.name = "msm_adspdec",
	.id = -1,
	.dev    = {
		.platform_data = &msm_device_adspdec_database
	},
};

static struct android_pmem_platform_data android_pmem_audio_pdata = {
	.name = "pmem_audio",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 0,
	.memory_type = MEMTYPE_EBI1,
};

static struct platform_device android_pmem_audio_device = {
	.name = "android_pmem",
	.id = 2,
	.dev = { .platform_data = &android_pmem_audio_pdata },
};

static struct android_pmem_platform_data android_pmem_pdata = {
	.name = "pmem",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 1,
	.memory_type = MEMTYPE_EBI1,
};
static struct platform_device android_pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = { .platform_data = &android_pmem_pdata },
};

static u32 msm_calculate_batt_capacity(u32 current_voltage);

static struct msm_psy_batt_pdata msm_psy_batt_data = {
	.voltage_min_design     = 2800,
	.voltage_max_design     = 4300,
	.voltage_fail_safe      = 2900,
	.avail_chg_sources      = AC_CHG | USB_CHG ,
	.batt_technology        = POWER_SUPPLY_TECHNOLOGY_LION,
	.calculate_capacity     = &msm_calculate_batt_capacity,
};

static u32 msm_calculate_batt_capacity(u32 current_voltage)
{
	u32 low_voltage	 = msm_psy_batt_data.voltage_min_design;
	u32 high_voltage = msm_psy_batt_data.voltage_max_design;

	if (current_voltage <= low_voltage)
		return 0;
	else if (current_voltage >= high_voltage)
		return 100;
	else
		return (current_voltage - low_voltage) * 100
			/ (high_voltage - low_voltage);
}

static struct platform_device msm_fastboot_device = {
	.name               = "fastboot",
	.id                 = -1,
};

static struct platform_device msm_batt_device = {
	.name               = "msm-battery",
	.id                 = -1,
	.dev.platform_data  = &msm_psy_batt_data,
};

static char *msm_adc_surf_device_names[] = {
	"XO_ADC",
};

static struct msm_adc_platform_data msm_adc_pdata = {
	.dev_names = msm_adc_surf_device_names,
	.num_adc = ARRAY_SIZE(msm_adc_surf_device_names),
	.target_hw = MSM_8x25,
};

static struct platform_device msm_adc_device = {
	.name   = "msm_adc",
	.id = -1,
	.dev = {
		.platform_data = &msm_adc_pdata,
	},
};

static struct fmem_platform_data fmem_pdata;

static struct platform_device fmem_device = {
	.name = "fmem",
	.id = 1,
	.dev = { .platform_data = &fmem_pdata },
};

#define GPIO_VREG_INIT(_id, _reg_name, _gpio_label, _gpio, _active_low) \
	[GPIO_VREG_ID_##_id] = { \
		.init_data = { \
			.constraints = { \
				.valid_ops_mask	= REGULATOR_CHANGE_STATUS, \
			}, \
			.num_consumer_supplies	= \
					ARRAY_SIZE(vreg_consumers_##_id), \
			.consumer_supplies	= vreg_consumers_##_id, \
		}, \
		.regulator_name	= _reg_name, \
		.active_low	= _active_low, \
		.gpio_label	= _gpio_label, \
		.gpio		= _gpio, \
	}

#define GPIO_VREG_ID_EXT_2P85V	0
#define GPIO_VREG_ID_EXT_1P8V	1

static struct regulator_consumer_supply vreg_consumers_EXT_2P85V[] = {
	REGULATOR_SUPPLY("cam_ov5647_avdd", "0-006c"),
	REGULATOR_SUPPLY("cam_ov7692_avdd", "0-0078"),
	REGULATOR_SUPPLY("cam_ov8825_avdd", "0-000d"),
	REGULATOR_SUPPLY("lcd_vdd", "mipi_dsi.1"),
};

static struct regulator_consumer_supply vreg_consumers_EXT_1P8V[] = {
	REGULATOR_SUPPLY("cam_ov5647_vdd", "0-006c"),
	REGULATOR_SUPPLY("cam_ov7692_vdd", "0-0078"),
	REGULATOR_SUPPLY("cam_ov8825_vdd", "0-000d"),
	REGULATOR_SUPPLY("lcd_vddi", "mipi_dsi.1"),
};

/* GPIO regulator constraints */
static struct gpio_regulator_platform_data msm_gpio_regulator_pdata[] = {
	GPIO_VREG_INIT(EXT_2P85V, "ext_2p85v", "ext_2p85v_en", 35, 0),
	GPIO_VREG_INIT(EXT_1P8V, "ext_1p8v", "ext_1p8v_en", 40, 0),
};

/* GPIO regulator */
static struct platform_device qrd_vreg_gpio_ext_2p85v __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= 35,
	.dev	= {
		.platform_data =
			&msm_gpio_regulator_pdata[GPIO_VREG_ID_EXT_2P85V],
	},
};

static struct platform_device qrd_vreg_gpio_ext_1p8v __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= 40,
	.dev	= {
		.platform_data =
			&msm_gpio_regulator_pdata[GPIO_VREG_ID_EXT_1P8V],
	},
};

static struct platform_device *common_devices[] __initdata = {
	&android_usb_device,
	&android_pmem_device,
	&android_pmem_adsp_device,
	&android_pmem_audio_device,
	&msm_batt_device,
	&msm_device_adspdec,
	&msm_device_snd,
	&msm_device_cad, //???
	&asoc_msm_pcm,
	&asoc_msm_dai0,
	&asoc_msm_dai1,
	&msm_adc_device,
	&fmem_device,
    &msm_fastboot_device,
#ifdef CONFIG_ION_MSM
	&ion_dev,
#endif
};

static struct platform_device *qrd7627a_devices[] __initdata = {
	&msm_device_dmov,
	&msm_device_smd,
	&msm_device_uart1,
	&msm_device_uart_dm1,
	&msm_gsbi0_qup_i2c_device,
	&msm_gsbi1_qup_i2c_device,
	&msm_device_otg,
	&msm_device_gadget_peripheral,
	&msm_kgsl_3d0,
//	&qrd_vreg_gpio_ext_2p85v,
//	&qrd_vreg_gpio_ext_1p8v,
};

static struct platform_device *qrd3_devices[] __initdata = {
	&msm_device_nand,
};

static struct platform_device *msm8625_evb_devices[] __initdata = {
	&msm8625_device_dmov,
	&msm8625_device_smd,
	&msm8625_gsbi0_qup_i2c_device,
	&msm8625_gsbi1_qup_i2c_device,
	&msm8625_device_uart1,
	&msm8625_device_uart_dm1,
	&msm8625_device_otg,
	&msm8625_device_gadget_peripheral,
	&msm8625_kgsl_3d0,
//	&qrd_vreg_gpio_ext_2p85v,
//	&qrd_vreg_gpio_ext_1p8v,
};

static unsigned pmem_kernel_ebi1_size = PMEM_KERNEL_EBI1_SIZE;
static int __init pmem_kernel_ebi1_size_setup(char *p)
{
	pmem_kernel_ebi1_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_kernel_ebi1_size", pmem_kernel_ebi1_size_setup);

static unsigned pmem_audio_size = MSM_PMEM_AUDIO_SIZE;
static int __init pmem_audio_size_setup(char *p)
{
	pmem_audio_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_audio_size", pmem_audio_size_setup);

static void fix_sizes(void)
{
	if (get_ddr_size() > SZ_512M)
		pmem_adsp_size = CAMERA_ZSL_SIZE;
#ifdef CONFIG_ION_MSM
	msm_ion_camera_size = pmem_adsp_size;
	msm_ion_audio_size = (MSM_PMEM_AUDIO_SIZE + PMEM_KERNEL_EBI1_SIZE);
	msm_ion_sf_size = pmem_mdp_size;
#endif
}

#ifdef CONFIG_ION_MSM
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
static struct ion_co_heap_pdata co_ion_pdata = {
	.adjacent_mem_id = INVALID_HEAP_ID,
	.align = PAGE_SIZE,
};
#endif

/**
 * These heaps are listed in the order they will be allocated.
 * Don't swap the order unless you know what you are doing!
 */
static struct ion_platform_data ion_pdata = {
	.nr = MSM_ION_HEAP_NUM,
	.has_outer_cache = 1,
	.heaps = {
		{
			.id	= ION_SYSTEM_HEAP_ID,
			.type	= ION_HEAP_TYPE_SYSTEM,
			.name	= ION_VMALLOC_HEAP_NAME,
		},
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
		/* PMEM_ADSP = CAMERA */
		{
			.id	= ION_CAMERA_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_CAMERA_HEAP_NAME,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *)&co_ion_pdata,
		},
		/* PMEM_AUDIO */
		{
			.id	= ION_AUDIO_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_AUDIO_HEAP_NAME,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *)&co_ion_pdata,
		},
		/* PMEM_MDP = SF */
		{
			.id	= ION_SF_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_SF_HEAP_NAME,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *)&co_ion_pdata,
		},
#endif
	}
};

static struct platform_device ion_dev = {
	.name = "ion-msm",
	.id = 1,
	.dev = { .platform_data = &ion_pdata },
};
#endif

static struct memtype_reserve msm7627a_reserve_table[] __initdata = {
	[MEMTYPE_SMI] = {
	},
	[MEMTYPE_EBI0] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
	[MEMTYPE_EBI1] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
};

#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
static struct android_pmem_platform_data *pmem_pdata_array[] __initdata = {
		&android_pmem_adsp_pdata,
		&android_pmem_audio_pdata,
		&android_pmem_pdata,
};
#endif
#endif

static void __init size_pmem_devices(void)
{
#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	unsigned int i;
	unsigned int reusable_count = 0;

	android_pmem_adsp_pdata.size = pmem_adsp_size;
	android_pmem_pdata.size = pmem_mdp_size;
	android_pmem_audio_pdata.size = pmem_audio_size;

	fmem_pdata.size = 0;
	fmem_pdata.align = PAGE_SIZE;

	/* Find pmem devices that should use FMEM (reusable) memory.
	 */
	for (i = 0; i < ARRAY_SIZE(pmem_pdata_array); ++i) {
		struct android_pmem_platform_data *pdata = pmem_pdata_array[i];

		if (!reusable_count && pdata->reusable)
			fmem_pdata.size += pdata->size;

		reusable_count += (pdata->reusable) ? 1 : 0;

		if (pdata->reusable && reusable_count > 1) {
			pr_err("%s: Too many PMEM devices specified as reusable. PMEM device %s was not configured as reusable.\n",
				__func__, pdata->name);
			pdata->reusable = 0;
		}
	}
#endif
#endif
}

#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
static void __init reserve_memory_for(struct android_pmem_platform_data *p)
{
	msm7627a_reserve_table[p->memory_type].size += p->size;
}
#endif
#endif

static void __init reserve_pmem_memory(void)
{
#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	unsigned int i;
	for (i = 0; i < ARRAY_SIZE(pmem_pdata_array); ++i)
		reserve_memory_for(pmem_pdata_array[i]);

	msm7627a_reserve_table[MEMTYPE_EBI1].size += pmem_kernel_ebi1_size;
#endif
#endif
}

static void __init size_ion_devices(void)
{
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
	ion_pdata.heaps[1].size = msm_ion_camera_size;
	ion_pdata.heaps[2].size = msm_ion_audio_size;
	ion_pdata.heaps[3].size = msm_ion_sf_size;
#endif
}

static void __init reserve_ion_memory(void)
{
#if defined(CONFIG_ION_MSM) && defined(CONFIG_MSM_MULTIMEDIA_USE_ION)
	msm7627a_reserve_table[MEMTYPE_EBI1].size += msm_ion_camera_size;
	msm7627a_reserve_table[MEMTYPE_EBI1].size += msm_ion_audio_size;
	msm7627a_reserve_table[MEMTYPE_EBI1].size += msm_ion_sf_size;
#endif
}

static void __init msm7627a_calculate_reserve_sizes(void)
{
	fix_sizes();
	size_pmem_devices();
	reserve_pmem_memory();
	size_ion_devices();
	reserve_ion_memory();
}

static int msm7627a_paddr_to_memtype(unsigned int paddr)
{
	return MEMTYPE_EBI1;
}

static struct reserve_info msm7627a_reserve_info __initdata = {
	.memtype_reserve_table = msm7627a_reserve_table,
	.calculate_reserve_sizes = msm7627a_calculate_reserve_sizes,
	.paddr_to_memtype = msm7627a_paddr_to_memtype,
};

static void __init msm7627a_reserve(void)
{
	reserve_info = &msm7627a_reserve_info;
	msm_reserve();
	memblock_remove(MSM8625_WARM_BOOT_PHYS, SZ_32);
}

static void __init msm8625_reserve(void)
{
	memblock_remove(MSM8625_SECONDARY_PHYS, SZ_8);
	memblock_remove(MSM8625_NON_CACHE_MEM, SZ_2K);
	msm7627a_reserve();
}

static void msmqrd_adsp_add_pdev(void)
{
	int rc = 0;
	struct rpc_board_dev *rpc_adsp_pdev;

	rpc_adsp_pdev = kzalloc(sizeof(struct rpc_board_dev), GFP_KERNEL);
	if (rpc_adsp_pdev == NULL) {
		pr_err("%s: Memory Allocation failure\n", __func__);
		return;
	}
	rpc_adsp_pdev->prog = ADSP_RPC_PROG;

	if (cpu_is_msm8625())
		rpc_adsp_pdev->pdev = msm8625_device_adsp;
	else
		rpc_adsp_pdev->pdev = msm_adsp_device;
	rc = msm_rpc_add_board_dev(rpc_adsp_pdev, 1);
	if (rc < 0) {
		pr_err("%s: return val: %d\n",	__func__, rc);
		kfree(rpc_adsp_pdev);
	}
}

static void __init msm7627a_device_i2c_init(void)
{
	msm_gsbi0_qup_i2c_device.dev.platform_data = &msm_gsbi0_qup_i2c_pdata;
	msm_gsbi1_qup_i2c_device.dev.platform_data = &msm_gsbi1_qup_i2c_pdata;
}

static void __init msm8625_device_i2c_init(void)
{
	msm8625_gsbi0_qup_i2c_device.dev.platform_data
					= &msm_gsbi0_qup_i2c_pdata;
	msm8625_gsbi1_qup_i2c_device.dev.platform_data
					= &msm_gsbi1_qup_i2c_pdata;
}

static struct platform_device msm_proccomm_regulator_dev = {
	.name   = PROCCOMM_REGULATOR_DEV_NAME,
	.id     = -1,
	.dev    = {
		.platform_data = &msm7x27a_proccomm_regulator_data
	}
};

/* LCD, Camera power supply */
static struct platform_device evb_device_ext_2v8_vreg = {
	.name			= GPIO_REGULATOR_DEV_NAME,
	.id				= 35,
	.dev = {
		.platform_data =
			&msm7x27a_evb_gpio_regulator_pdata[MSM7X27A_GPIO_VREG_ID_EXT_2V8],
	},
};

#if 0 //add by fangxing for sd hot plug in 20120628
static struct platform_device evb_device_ext_1v8_vreg = {
	.name			= GPIO_REGULATOR_DEV_NAME,
	.id				= 40,
	.dev = {
		.platform_data =
			&msm7x27a_evb_gpio_regulator_pdata[MSM7X27A_GPIO_VREG_ID_EXT_1V8],
	},
};
#endif //fangxing end
static struct platform_device sku3_device_ext_2v8_vreg = {
	.name			= GPIO_REGULATOR_DEV_NAME,
	.id				= 35,
	.dev = {
		.platform_data =
			&msm7x27a_sku3_gpio_regulator_pdata[MSM7X27A_GPIO_VREG_ID_EXT_2V8],
	},
};

static struct platform_device sku3_device_ext_1v8_vreg = {
	.name			= GPIO_REGULATOR_DEV_NAME,
	.id				= 34,
	.dev = {
		.platform_data =
			&msm7x27a_sku3_gpio_regulator_pdata[MSM7X27A_GPIO_VREG_ID_EXT_1V8],
	},
};

static struct platform_device sku7_device_ext_2v8_vreg = {
	.name			= GPIO_REGULATOR_DEV_NAME,
	.id				= 35,
	.dev = {
		.platform_data =
			&msm7x27a_sku7_gpio_regulator_pdata[MSM7X27A_GPIO_VREG_ID_EXT_2V8],
	},
};

static struct platform_device sku7_device_ext_1v8_vreg = {
	.name			= GPIO_REGULATOR_DEV_NAME,
	.id				= 58,
	.dev = {
		.platform_data =
			&msm7x27a_sku7_gpio_regulator_pdata[MSM7X27A_GPIO_VREG_ID_EXT_1V8],
	},
};

static void __init msm7627a_init_regulators(void)
{
	int rc = 0;

	printk("%s\n", __func__);

	rc = platform_device_register(&msm_proccomm_regulator_dev);
	if (rc)
		pr_err("%s: could not register msm_proccomm_regulator_dev: %d\n",
				__func__, rc);

	if (machine_is_msm7627a_evb() || machine_is_msm8625_evb() || machine_is_msm8625_qrd5() || machine_is_msm7x27a_qrd5a()) {
		rc = platform_device_register(&evb_device_ext_2v8_vreg);
		if (rc)
			pr_err("%s: could not register evb_device_ext_2v8_vreg: %d\n",
					__func__, rc);
#if 0 //add by fangxing for sd hot plug in 20120628
		rc = platform_device_register(&evb_device_ext_1v8_vreg);
		if (rc)
			pr_err("%s: could not register evb_device_ext_1v8_vreg: %d\n",
					__func__, rc);
#endif //fangxing end
	} else if (machine_is_msm8625_qrd7()) {
		rc = platform_device_register(&sku7_device_ext_2v8_vreg);
		if (rc)
			pr_err("%s: could not register sku7_device_ext_2v8_vreg: %d\n",
					__func__, rc);

		rc = platform_device_register(&sku7_device_ext_1v8_vreg);
		if (rc)
			pr_err("%s: could not register sku7_device_ext_1v8_vreg: %d\n",
					__func__, rc);
	} else if (machine_is_msm7627a_qrd3()) {
		rc = platform_device_register(&sku3_device_ext_2v8_vreg);
		if (rc)
			pr_err("%s: could not register sku3_device_ext_2v8_vreg: %d\n",
					__func__, rc);

		rc = platform_device_register(&sku3_device_ext_1v8_vreg);
		if (rc)
			pr_err("%s: could not register sku3_device_ext_1v8_vreg: %d\n",
					__func__, rc);
	}

	return;
}

#if 0
/* 8625 keypad device information */
static unsigned int kp_row_gpios_8625[] = {31};
static unsigned int kp_col_gpios_8625[] = {36, 37};

static const unsigned short keymap_8625[] = {
	KEY_VOLUMEUP,
	KEY_VOLUMEDOWN,
};

static const unsigned short keymap_8625_qrd5[] = {
	KEY_VOLUMEDOWN,
	KEY_VOLUMEUP,
};

static struct gpio_event_matrix_info kp_matrix_info_8625 = {
	.info.func      = gpio_event_matrix_func,
	.keymap         = keymap_8625,
	.output_gpios   = kp_row_gpios_8625,
	.input_gpios    = kp_col_gpios_8625,
	.noutputs       = ARRAY_SIZE(kp_row_gpios_8625),
	.ninputs        = ARRAY_SIZE(kp_col_gpios_8625),
	.settle_time.tv_nsec = 40 * NSEC_PER_USEC,
	.poll_time.tv_nsec = 20 * NSEC_PER_MSEC,
	.flags          = GPIOKPF_LEVEL_TRIGGERED_IRQ | GPIOKPF_DRIVE_INACTIVE |
			  GPIOKPF_PRINT_UNMAPPED_KEYS,
};

static struct gpio_event_info *kp_info_8625[] = {
	&kp_matrix_info_8625.info,
};
static struct gpio_event_platform_data kp_pdata_8625 = {
	.name           = "8625_kp",
	.info           = kp_info_8625,
	.info_count     = ARRAY_SIZE(kp_info_8625)
};

static struct platform_device kp_pdev_8625 = {
	.name   = GPIO_EVENT_DEV_NAME,
	.id     = -1,
	.dev    = {
		.platform_data  = &kp_pdata_8625,
	},
};

/* Disable the leds init in AP
#define LED_RED_GPIO_8625 49
#define LED_GREEN_GPIO_8625 34

static struct gpio_led gpio_leds_config_8625[] = {
	{
		.name = "green",
		.gpio = LED_GREEN_GPIO_8625,
	},
	{
		.name = "red",
		.gpio = LED_RED_GPIO_8625,
	},
};

static struct gpio_led_platform_data gpio_leds_pdata_8625 = {
	.num_leds = ARRAY_SIZE(gpio_leds_config_8625),
	.leds = gpio_leds_config_8625,
};

static struct platform_device gpio_leds_8625 = {
	.name          = "leds-gpio",
	.id            = -1,
	.dev           = {
		.platform_data = &gpio_leds_pdata_8625,
	},
};
*/

#define MXT_TS_IRQ_GPIO         48
#define MXT_TS_RESET_GPIO       26

static const u8 mxt_config_data[] = {
	/* T6 Object */
	0, 0, 0, 0, 0, 0,
	/* T38 Object */
	16, 0, 0, 0, 0, 0, 0, 0,
	/* T7 Object */
	32, 16, 50,
	/* T8 Object */
	30, 0, 20, 20, 0, 0, 20, 0, 50, 0,
	/* T9 Object */
	3, 0, 0, 18, 11, 0, 32, 75, 3, 3,
	0, 1, 1, 0, 10, 10, 10, 10, 31, 3,
	223, 1, 11, 11, 15, 15, 151, 43, 145, 80,
	100, 15, 0, 0, 0,
	/* T15 Object */
	131, 0, 11, 11, 1, 1, 0, 45, 3, 0,
	0,
	/* T18 Object */
	0, 0,
	/* T19 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	/* T23 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	/* T25 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,
	/* T40 Object */
	0, 0, 0, 0, 0,
	/* T42 Object */
	0, 0, 0, 0, 0, 0, 0, 0,
	/* T46 Object */
	0, 2, 32, 48, 0, 0, 0, 0, 0,
	/* T47 Object */
	1, 20, 60, 5, 2, 50, 40, 0, 0, 40,
	/* T48 Object */
	1, 12, 80, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 6, 6, 0, 0, 100, 4, 64,
	10, 0, 20, 5, 0, 38, 0, 20, 0, 0,
	0, 0, 0, 0, 16, 65, 3, 1, 1, 0,
	10, 10, 10, 0, 0, 15, 15, 154, 58, 145,
	80, 100, 15, 3,
};

#define MAX_LEN             100

static ssize_t mxt_virtual_keys_register(struct kobject *kobj,
                     struct kobj_attribute *attr, char *buf)
{
	char *virtual_keys = __stringify(EV_KEY) ":" __stringify(KEY_MENU) \
				":60:840:120:80" ":" __stringify(EV_KEY) \
				":" __stringify(KEY_HOME)   ":180:840:120:80" \
				":" __stringify(EV_KEY) ":" \
				__stringify(KEY_BACK) ":300:840:120:80" \
				":" __stringify(EV_KEY) ":" \
				__stringify(KEY_SEARCH)   ":420:840:120:80" "\n";

	return snprintf(buf, strnlen(virtual_keys, MAX_LEN) + 1 , "%s",
                        virtual_keys);
}

static struct kobj_attribute mxt_virtual_keys_attr = {
        .attr = {
                .name = "virtualkeys.atmel_mxt_ts",
                .mode = S_IRUGO,
        },
        .show = &mxt_virtual_keys_register,
};

static struct attribute *mxt_virtual_key_properties_attrs[] = {
        &mxt_virtual_keys_attr.attr,
        NULL
};

static struct attribute_group mxt_virtual_key_properties_attr_group = {
        .attrs = mxt_virtual_key_properties_attrs,
};

struct kobject *mxt_virtual_key_properties_kobj;

static int mxt_touchpad_setup(void)
{
	int retval;

        mxt_virtual_key_properties_kobj =
                kobject_create_and_add("board_properties", NULL);
        if (mxt_virtual_key_properties_kobj)
                retval = sysfs_create_group(mxt_virtual_key_properties_kobj,
                                &mxt_virtual_key_properties_attr_group);
        if (!mxt_virtual_key_properties_kobj || retval)
                pr_err("failed to create mxt board_properties\n");

	return retval;
}

static const u8 mxt_config_data_qrd5[] = {
	/* T6 Object */
	0, 0, 0, 0, 0, 0,
	/* T38 Object */
	16, 0, 0, 0, 0, 0, 0, 0,
	/* T7 Object */
	255, 255, 10,
	/* T8 Object */
	30, 0, 20, 20, 0, 0, 9, 45, 10, 192,
	/* T9 Object */
	131, 0, 0, 18, 11, 0, 16, 45, 2, 1,
	0, 1, 1, 0, 10, 10, 10, 10, 107, 3,
	223, 1, 11, 11, 15, 15, 151, 43, 0, 0,
	100, 15, 0, 0, 0,
	/* T15 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0,
	/* T18 Object */
	0, 0,
	/* T19 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	/* T23 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	/* T25 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,
	/* T40 Object */
	0, 0, 0, 0, 0,
	/* T42 Object */
	0, 0, 0, 0, 0, 0, 0, 0,
	/* T46 Object */
	0, 2, 32, 48, 0, 0, 0, 0, 0,
	/* T47 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* T48 Object */
	1, 0, 96, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 6, 6, 0, 0, 100, 4, 64,
	10, 0, 20, 5, 0, 38, 0, 20, 0, 0,
	0, 0, 0, 0, 16, 65, 3, 1, 1, 0,
	10, 10, 10, 0, 0, 15, 15, 154, 58, 145,
	80, 100, 15, 3,
};

static struct mxt_config_info mxt_config_array[] = {
	{
		.config		= mxt_config_data,
		.config_length	= ARRAY_SIZE(mxt_config_data),
		.family_id	= 0x81,
		.variant_id	= 0x01,
		.version	= 0x10,
		.build		= 0xAA,
	},
};

static int mxt_key_codes[MXT_KEYARRAY_MAX_KEYS] = {
	[0] = KEY_HOME,
	[1] = KEY_MENU,
	[9] = KEY_BACK,
	[10] = KEY_SEARCH,
};

static struct mxt_platform_data mxt_platform_data = {
	.config_array		= mxt_config_array,
	.config_array_size	= ARRAY_SIZE(mxt_config_array),
	.panel_minx		= 0,
	.panel_maxx		= 479,
	.panel_miny		= 0,
	.panel_maxy		= 879,
	.disp_minx		= 0,
	.disp_maxx		= 479,
	.disp_miny		= 0,
	.disp_maxy		= 799,
	.irqflags               = IRQF_TRIGGER_FALLING,
	.i2c_pull_up            = true,
	.reset_gpio		= MXT_TS_RESET_GPIO,
	.irq_gpio		= MXT_TS_IRQ_GPIO,
	.key_codes		= mxt_key_codes,
};

static struct i2c_board_info mxt_device_info[] __initdata = {
	{
		I2C_BOARD_INFO("atmel_mxt_ts", 0x4a),
		.platform_data = &mxt_platform_data,
		.irq = MSM_GPIO_TO_INT(MXT_TS_IRQ_GPIO),
	},
};

static void msm7627a_add_io_devices(void)
{
	int rc;

	/* touchscreen */
	if (machine_is_msm7627a_qrd1()) {
		i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
					synaptic_i2c_clearpad3k,
					ARRAY_SIZE(synaptic_i2c_clearpad3k));
	} else if (machine_is_msm7627a_evb() || machine_is_msm8625_evb() || machine_is_msm8625_qrd5() || machine_is_msm7x27a_qrd5a()) {
		if (machine_is_msm8625_qrd5() || machine_is_msm7x27a_qrd5a()) {
			mxt_config_array[0].config = mxt_config_data_qrd5;
			mxt_config_array[0].config_length = ARRAY_SIZE(mxt_config_data_qrd5);
			mxt_touchpad_setup();
		}

		rc = gpio_tlmm_config(GPIO_CFG(MXT_TS_IRQ_GPIO, 0,
					GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,
					GPIO_CFG_8MA), GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("%s: gpio_tlmm_config for %d failed\n",
					__func__, MXT_TS_IRQ_GPIO);
		}

		rc = gpio_tlmm_config(GPIO_CFG(MXT_TS_RESET_GPIO, 0,
					GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN,
					GPIO_CFG_8MA), GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("%s: gpio_tlmm_config for %d failed\n",
					__func__, MXT_TS_RESET_GPIO);
		}

		i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
				mxt_device_info,
				ARRAY_SIZE(mxt_device_info));

	}else if (machine_is_msm7627a_qrd3() || 
			machine_is_msm8625_skua() ||
			machine_is_msm8625_qrd7()){
		ft5x06_touchpad_setup();
	}

	/* headset */
	platform_device_register(&hs_pdev);

	/* vibrator */
#ifdef CONFIG_MSM_RPC_VIBRATOR
	msm_init_pmic_vibrator();
#endif

	/* change the keymap for qrd sku5 */
	if(machine_is_msm8625_qrd5() || machine_is_msm7x27a_qrd5a())
		kp_matrix_info_8625.keymap = keymap_8625_qrd5;

	/* keypad */
	if (machine_is_msm7627a_evb() ||
		 machine_is_msm8625_evb()||
		 machine_is_msm8625_qrd5() || machine_is_msm7x27a_qrd5a() ||
		 machine_is_msm8625_qrd7() ||
		 machine_is_msm8625_skua())
		platform_device_register(&kp_pdev_8625);

	/* leds */
/* disable the AP leds
	if (machine_is_msm7627a_evb() || machine_is_msm8625_evb() || machine_is_msm8625_qrd5()) {
		rc = gpio_tlmm_config(GPIO_CFG(LED_RED_GPIO_8625, 0,
				GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP,
				GPIO_CFG_16MA), GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("%s: gpio_tlmm_config for %d failed\n",
				__func__, LED_RED_GPIO_8625);
		}

		rc = gpio_tlmm_config(GPIO_CFG(LED_GREEN_GPIO_8625, 0,
				GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP,
				GPIO_CFG_16MA), GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("%s: gpio_tlmm_config for %d failed\n",
				__func__, LED_GREEN_GPIO_8625);
		}

		platform_device_register(&gpio_leds_8625);
	}
*/
#ifdef CONFIG_LEDS_TRICOLOR_FLAHSLIGHT
	    /*tricolor leds init*/
	tricolor_leds_gpio_setup();
#endif
}
#endif

#define NV_ITEM_WLAN_MAC_ADDR   4678
extern int msm_read_nv(unsigned int nv_item, void *buf);
unsigned char wlan_mac_addr[6];

static int __init msm_qrd_init_ar6000pm(void)
{
	//if (cpu_is_msm8625())
	//	return 0;
	msm_wlan_ar6000_pm_device.dev.platform_data = &ar600x_wlan_power;
	msm_read_nv(NV_ITEM_WLAN_MAC_ADDR,wlan_mac_addr);
	return platform_device_register(&msm_wlan_ar6000_pm_device);
}

static void __init msm_add_footswitch_devices(void)
{
	platform_add_devices(msm_footswitch_devices,
				msm_num_footswitch_devices);
}

static void __init add_platform_devices(void)
{
	if (machine_is_msm8625_evb() || machine_is_msm8625_qrd5() || 
		machine_is_msm8625_skua() || machine_is_msm8625_qrd7() || 
		machine_is_msm8625_skub() || machine_is_msm8625_evt())
		platform_add_devices(msm8625_evb_devices,
				ARRAY_SIZE(msm8625_evb_devices));
	else {
		platform_add_devices(qrd7627a_devices,
				ARRAY_SIZE(qrd7627a_devices));
	}
	if (machine_is_msm7627a_qrd3() || machine_is_msm7627a_evb() || machine_is_msm8625_qrd7() || machine_is_msm8625_skub())
		platform_add_devices(qrd3_devices,
				ARRAY_SIZE(qrd3_devices));
	platform_add_devices(common_devices,
			ARRAY_SIZE(common_devices));
}

#define UART1DM_RX_GPIO		45
static void __init qrd7627a_uart1dm_config(void)
{
	msm_uart_dm1_pdata.wakeup_irq = gpio_to_irq(UART1DM_RX_GPIO);
	if (cpu_is_msm8625())
		msm8625_device_uart_dm1.dev.platform_data =
			&msm_uart_dm1_pdata;
	else
		msm_device_uart_dm1.dev.platform_data = &msm_uart_dm1_pdata;
}

static void __init qrd7627a_otg_gadget(void)
{
	if (cpu_is_msm8625()) {
		msm_otg_pdata.swfi_latency = msm8625_pm_data
		[MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT].latency;
		msm8625_device_otg.dev.platform_data = &msm_otg_pdata;
		msm8625_device_gadget_peripheral.dev.platform_data =
					&msm_gadget_pdata;

	} else {
	msm_otg_pdata.swfi_latency = msm7627a_pm_data
		[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].latency;
		msm_device_otg.dev.platform_data = &msm_otg_pdata;
		msm_device_gadget_peripheral.dev.platform_data =
					&msm_gadget_pdata;
	}
}

static void __init msm_pm_init(void)
{
#if 0
	if (machine_is_msm8625_qrd7())
		return;
#endif

	if (!(machine_is_msm8625_evb() || machine_is_msm8625_qrd5() || machine_is_msm8625_skua() || machine_is_msm8625_qrd7() || machine_is_msm8625_skub())) {
		msm_pm_set_platform_data(msm7627a_pm_data,
				ARRAY_SIZE(msm7627a_pm_data));
		BUG_ON(msm_pm_boot_init(&msm_pm_boot_pdata));
	} else {
		msm_pm_set_platform_data(msm8625_pm_data,
				ARRAY_SIZE(msm8625_pm_data));
		BUG_ON(msm_pm_boot_init(&msm_pm_8625_boot_pdata));
		msm8x25_spm_device_init();
	}
}

//luke: add
static char ragentek_mmc_product_name[7] = {0};
static unsigned int ragentek_manufacturer_id = 0;
static unsigned int ragentek_oem_id = 0;
int __init ragentek_get_mmc_product_name(char *s)
{
	strcpy(ragentek_mmc_product_name, s);
	//printk("luke: ragentek_mmc_product_name = %s\n", ragentek_mmc_product_name);
	return 1;
}
__setup("ragentek.mmcproductname=", ragentek_get_mmc_product_name);

int __init ragentek_get_mmc_manufacturer_id(char *s)
{
	ragentek_manufacturer_id = simple_strtoul(s, NULL, 0);
	//printk("luke: ragentek_manufacturer_id = %d\n", ragentek_manufacturer_id);
	return 1;
}
__setup("ragentek.mmcmanufacturerid=", ragentek_get_mmc_manufacturer_id);

int __init ragentek_get_mmc_oem_id(char *s)
{
	ragentek_oem_id = simple_strtoul(s, NULL, 0);
	//printk("luke: ragentek_oem_id = %d\n", ragentek_oem_id);
	return 1;
}
__setup("ragentek.mmcoemid=", ragentek_get_mmc_oem_id);


typedef struct {
	unsigned int mid;	/* 8 bit manufacturer id */
	unsigned int oid;	/* 16 bits 2 character ASCII - OEM ID */
	unsigned char pnm[7];	/* 6 character ASCII -  product name */
	unsigned char flash_type[255]; /* nand flash type information */
}msm_nand_info;

static msm_nand_info msm_nand_info_table[] = {                                    //table 

          {0x15, 0x100, "JJS00M", "KMJJSOOOWA-B409"}, //Q801
	  {0x15, 0x100, "SJS00M", "KMSJS000KM-B308"}, //Q802
	  {0, 0, {0}, {0}},
};
static struct proc_dir_entry *msm_nand_dir;
static struct proc_dir_entry *msm_nand_info_file;
static int ragentek_msm_nand_info_read(char *page, char **start, off_t off, int count,
		int *eof, void *data)
{
	ssize_t len = 0;
	msm_nand_info *nand_info_ptr = msm_nand_info_table;
	char *nand_info = NULL;

	if(off > 0)
	{
		*eof = 1;
		return 0;
	}	
	//to find nand information
	while(strlen(nand_info_ptr->flash_type) > 0) {
		if(nand_info_ptr->mid == ragentek_manufacturer_id
			&& nand_info_ptr->oid == ragentek_oem_id
				&& !strcmp(ragentek_mmc_product_name, nand_info_ptr->pnm)) {
				nand_info = nand_info_ptr->flash_type;
				break;
		}
		nand_info_ptr++;	
	}
	
	if(!nand_info)
	{
		#if defined( CONFIG_Q203 )
			nand_info = "KMS5U000KM-B308";
		#else
			nand_info = "UNKNOWN";
		#endif
	}

	len = (ssize_t)sprintf(page, "%s\n", nand_info);
	
	return len;
}

static int ragentek_msm_nand_info_write(struct file * filp, const char __user *buff, unsigned long len,
		void *data)
{
	//nothing to do
	return 0;
}

static void ragentek_create_proc_file_for_nand_info(void)
{
	msm_nand_dir = proc_mkdir("msm_mtd_nand", NULL);
	if( NULL == msm_nand_dir) {
		pr_err("can't create msm_mtd_nand dir for ragentek factory test\n");
		return;	
	}

	msm_nand_info_file = create_proc_entry("msm_nand_info", 0444, msm_nand_dir);
	if( NULL == msm_nand_info_file ) {
		pr_err("can't create msm_nand_info file for ragentek factory test\n");
		return;	
	}
	
	msm_nand_info_file->read_proc = ragentek_msm_nand_info_read;
	msm_nand_info_file->write_proc = ragentek_msm_nand_info_write;
}
//end

//Start=====Allen
static void diag_getversion(void)
{
	char *apver;	

	apver = (char*)smem_alloc(SMEM_OSS_RRCASN1_BUF1, 32);	

	if(apver == NULL)
	
	{ 
		printk(" smem_alloc failed !!!\n");
		return;
	}

	//strcpy(apver, RAGENTEK_BUILD_VERSION);

	printk(" Ragentek build version = %s\n",apver);
	
}
//End=====Allen

static void __init msm_qrd_init(void)
{
	//Start=====Allen
	if(is_rgtk_product(RGTK_PRODUCT_Q801)) {
		q801_setup_gpio();
	}else if(is_rgtk_product(RGTK_PRODUCT_Q802)) {
		q802_setup_gpio();
	}else if(is_rgtk_product(RGTK_PRODUCT_Q803)) {
		q803_setup_gpio();
	}else if(is_rgtk_product(RGTK_PRODUCT_Q203)) {
		q203_setup_gpio();
	}else if(is_rgtk_product(RGTK_PRODUCT_Q203_NAND)) {
		q203_nand_setup_gpio();
	}else if(is_rgtk_product(RGTK_PRODUCT_DC205)) {
		dc205_setup_gpio();
	}
	//End=====Allen
	
        ragentek_create_proc_file_for_nand_info();          //luke: add
        if(creat_lcd_info_proc_file() < 0)
        	pr_err("creat_lcd_info_proc_file failed!"); //end

	msm7x2x_misc_init();
	msm7627a_init_regulators();
	msmqrd_adsp_add_pdev();

	if (cpu_is_msm8625())
		msm8625_device_i2c_init();
	else
		msm7627a_device_i2c_init();

	/* uart1dm*/
	qrd7627a_uart1dm_config();
	/*OTG gadget*/
	qrd7627a_otg_gadget();

	msm_add_footswitch_devices();
	add_platform_devices();

    //Start=====Allen
	diag_getversion();
	//End ===Allen

	/* Ensure ar6000pm device is registered before MMC/SDC */
	msm_qrd_init_ar6000pm();
	msm7627a_init_mmc();

#ifdef CONFIG_USB_EHCI_MSM_72K
	msm7627a_init_host();
#endif
	msm_pm_init();

	msm_pm_register_irqs();
	msm_fb_add_devices();

#if defined(CONFIG_BT) && defined(CONFIG_MARIMBA_CORE)
    /* qcomm QRD phone concerned not need by hw product*/
	msm7627a_bt_power_init();
#endif
	msm7627a_sensor_init();
	msm7627a_camera_init();
	qrd7627a_add_io_devices();
	msm7x25a_kgsl_3d0_init();
	msm8x25_kgsl_3d0_init();
}

static void __init qrd7627a_init_early(void)
{
	msm_msm7627a_allocate_memory_regions();
}

#ifdef CONFIG_MSM_AMSS_ENHANCE_DEBUG
#define TASK_OFFSET_SEND(member, name)		\
	do {					\
		input.extension.len = 2;	\
		input.extension.data[0] = (uint32_t)TASK_STRUCT_TAG; \
		input.extension.data[1] = offsetof(struct task_struct, member); \
		input.address = (uint32_t)__virt_to_phys((unsigned long)&init_task); \
		input.size = sizeof(struct task_struct); \
		strncpy(input.file_name, name, NZI_ITEM_FILE_NAME_LENGTH); \
		input.file_name[NZI_ITEM_FILE_NAME_LENGTH - 1] = 0; \
		send_modem_logaddr(&input); \
	} while (0)

static int __init qrd7627a_logbuf_init(void)
{
	nzi_buf_item_type input;
	extern char __log_buf[];
	extern unsigned long totalram_pages;
	extern atomic_long_t vm_stat[];

#ifdef CONFIG_PRINTK
	/* send the kernel log address */
	input.extension.len = 0;
	input.address = (uint32_t)__virt_to_phys((unsigned long)__log_buf);
	input.size = (1 << CONFIG_LOG_BUF_SHIFT);
	strncpy(input.file_name, "dmesg", NZI_ITEM_FILE_NAME_LENGTH);
	input.file_name[NZI_ITEM_FILE_NAME_LENGTH - 1] = 0;
	send_modem_logaddr(&input);
#endif

	/* ******struct task_struct part ******/
	TASK_OFFSET_SEND(tasks, "tasks_of");
	TASK_OFFSET_SEND(thread_group, "tg_of");
	TASK_OFFSET_SEND(mm, "mm_of");
	TASK_OFFSET_SEND(comm, "comm_of");
	TASK_OFFSET_SEND(pid, "pid_of");
	TASK_OFFSET_SEND(tgid, "tgid_of");
	/* ******struct task_struct end ******/

	/* totalram_pages */
	input.extension.len = 1;
	input.extension.data[0] = (uint32_t)MEM_INFO_TAG;
	input.address = (uint32_t)__virt_to_phys((unsigned long)&totalram_pages);
	input.size = sizeof(totalram_pages);
	strncpy(input.file_name, "totalram", NZI_ITEM_FILE_NAME_LENGTH);
	input.file_name[NZI_ITEM_FILE_NAME_LENGTH - 1] = 0;
	send_modem_logaddr(&input);

	/* vm_stat[vm_stat[NR_VM_ZONE_STAT_ITEMS]; */
	input.extension.len = 1;
	input.extension.data[0] = (uint32_t)MEM_INFO_TAG;
	input.address = (uint32_t)__virt_to_phys((unsigned long)vm_stat);
	input.size = sizeof(vm_stat);
	strncpy(input.file_name, "vm_stat", NZI_ITEM_FILE_NAME_LENGTH);
	input.file_name[NZI_ITEM_FILE_NAME_LENGTH - 1] = 0;
	send_modem_logaddr(&input);

	return 0;
}
late_initcall(qrd7627a_logbuf_init);
#endif

MACHINE_START(MSM7627A_QRD1, "QRD MSM7627a QRD1")
	.atag_offset	= 0x100,
	.map_io		= msm_common_io_init,
	.reserve	= msm7627a_reserve,
	.init_irq	= msm_init_irq,
	.init_machine	= msm_qrd_init,
	.timer		= &msm_timer,
	.init_early	= qrd7627a_init_early,
	.handle_irq	= vic_handle_irq,
MACHINE_END
MACHINE_START(MSM7627A_QRD3, "QRD MSM7627a QRD3")
	.atag_offset	= 0x100,
	.map_io		= msm_common_io_init,
	.reserve	= msm7627a_reserve,
	.init_irq	= msm_init_irq,
	.init_machine	= msm_qrd_init,
	.timer		= &msm_timer,
	.init_early	= qrd7627a_init_early,
	.handle_irq	= vic_handle_irq,
MACHINE_END
MACHINE_START(MSM7627A_EVB, "QRD MSM7627a EVB")
	.atag_offset	= 0x100,
	.map_io		= msm_common_io_init,
	.reserve	= msm7627a_reserve,
	.init_irq	= msm_init_irq,
	.init_machine	= msm_qrd_init,
	.timer		= &msm_timer,
	.init_early	= qrd7627a_init_early,
	.handle_irq	= vic_handle_irq,
MACHINE_END
MACHINE_START(MSM8625_SKUA, "QRD MSM8625 SKUA")
	.boot_params	= 0x100,
	.map_io		= msm8625_map_io,
	.reserve	= msm7627a_reserve,
	.init_irq	= msm8625_init_irq,
	.init_machine	= msm_qrd_init,
	.timer		= &msm_timer,
	.init_early	= qrd7627a_init_early,
	.handle_irq	= gic_handle_irq,
MACHINE_END
MACHINE_START(MSM8625_EVB, "QRD MSM8625 EVB")
	.atag_offset	= 0x100,
	.map_io		= msm8625_map_io,
	.reserve	= msm8625_reserve,
	.init_irq	= msm8625_init_irq,
	.init_machine	= msm_qrd_init,
	.timer		= &msm_timer,
	.init_early	= qrd7627a_init_early,
	.handle_irq	= gic_handle_irq,
MACHINE_END
MACHINE_START(MSM8625_QRD5, "QRD MSM8625 SKU5")
	.boot_params	= 0x100,
	.map_io		= msm8625_map_io,
	.reserve	= msm7627a_reserve,
	.init_irq	= msm8625_init_irq,
	.init_machine	= msm_qrd_init,
	.timer		= &msm_timer,
	.init_early	= qrd7627a_init_early,
	.handle_irq	= gic_handle_irq,
MACHINE_END
MACHINE_START(MSM7X27A_QRD5A, "QRD MSM7x27A QRD5A")
	.boot_params	= 0x100,
	.map_io		= msm_common_io_init,
	.reserve	= msm7627a_reserve,
	.init_irq	= msm_init_irq,
	.init_machine	= msm_qrd_init,
	.timer		= &msm_timer,
	.init_early	= qrd7627a_init_early,
	.handle_irq	= vic_handle_irq,
MACHINE_END
MACHINE_START(MSM8625_QRD7, "QRD MSM8625 QRD7")
	.atag_offset	= 0x100,
	.map_io		= msm8625_map_io,
	.reserve	= msm8625_reserve,
	.init_irq	= msm8625_init_irq,
	.init_machine	= msm_qrd_init,
	.timer		= &msm_timer,
	.init_early	= qrd7627a_init_early,
	.handle_irq	= gic_handle_irq,
MACHINE_END
MACHINE_START(MSM8625_SKUB, "QRD MSM8625 SKUB")
	.boot_params	= 0x100,
	.map_io		= msm8625_map_io,
	.reserve	= msm7627a_reserve,
	.init_irq	= msm8625_init_irq,
	.init_machine	= msm_qrd_init,
	.timer		= &msm_timer,
	.init_early	= qrd7627a_init_early,
	.handle_irq	= gic_handle_irq,
MACHINE_END
MACHINE_START(MSM8625_EVT, "QRD MSM8625 EVT")
	.atag_offset	= 0x100,
	.map_io		= msm8625_map_io,
	.reserve	= msm8625_reserve,
	.init_irq	= msm8625_init_irq,
	.init_machine	= msm_qrd_init,
	.timer		= &msm_timer,
	.init_early	= qrd7627a_init_early,
	.handle_irq	= gic_handle_irq,
MACHINE_END
