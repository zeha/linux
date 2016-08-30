/* Copyright (c) 2011-2012, The Linux Foundation. All rights reserved.
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

#include <linux/init.h>
#include <mach/gpiomux.h>
#include <mach/board.h>
#include <mach/gpio.h>
#include "board-9615.h"
#ifdef CONFIG_MFD_WM8944
#include <linux/sierra_bsudefs.h>
#endif

static struct gpiomux_setting ps_hold = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

#ifdef CONFIG_SIERRA_INTERNAL_CODEC
static struct gpiomux_setting slimbus = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_KEEPER,
};
#endif /* CONFIG_SIERRA_INTERNAL_CODEC */

#ifdef CONFIG_SIERRA_GSBI4_UART
/* 4-pin UART settings */
static struct gpiomux_setting gsbi4 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};
#endif /* CONFIG_SIERRA_GSBI4_UART */

#ifdef CONFIG_SIERRA_GSBI5_I2C_UART
static struct gpiomux_setting swi_gsbi_i2c_suspended = {
    .func = GPIOMUX_FUNC_1,
    .drv = GPIOMUX_DRV_2MA,
    .pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting swi_gsbi_i2c_active = {
    .func = GPIOMUX_FUNC_1,
    .drv = GPIOMUX_DRV_8MA,
    .pull = GPIOMUX_PULL_NONE,
};
#endif /* I2C pin settings*/

#ifdef CONFIG_SIERRA_GSBI2_I2C_GPIO
static struct gpiomux_setting gsbi2 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};
#else
static struct gpiomux_setting gsbi2 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};
#endif /* CONFIG_SIERRA_GSBI2_I2C_GPIO */

#ifdef CONFIG_SIERRA_GSBI3_SPI
static struct gpiomux_setting gsbi3 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_NONE,
};
#endif /* CONFIG_SIERRA_GSBI3_SPI */

#if defined(CONFIG_SIERRA_GSBI5_I2C_UART) || defined(CONFIG_SIERRA_GSBI5_UART)
static struct gpiomux_setting gsbi5_uart = {
    .func = GPIOMUX_FUNC_1,
    .drv = GPIOMUX_DRV_8MA,
    .pull = GPIOMUX_PULL_NONE,
};
#else
static struct gpiomux_setting gsbi5 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};
#endif /* CONFIG_SIERRA_GSBI5_I2C_UART || CONFIG_SIERRA_GSBI5_UART */

#ifdef CONFIG_LTC4088_CHARGER
static struct gpiomux_setting ltc4088_chg_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};
#endif

#ifdef CONFIG_SIERRA_WIFI_SDCC2
static struct gpiomux_setting sdcc2_clk_actv_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_16MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting sdcc2_cmd_data_0_3_actv_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting sdcc2_suspend_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};
#endif

#ifdef CONFIG_SIERRA_INTERNAL_CODEC
static struct gpiomux_setting cdc_mclk = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};
#endif /* CONFIG_SIERRA_INTERNAL_CODEC */

#ifdef CONFIG_FB_MSM_EBI2
static struct gpiomux_setting ebi2_lcdc_a_d = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_12MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting ebi2_lcdc_cs = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_12MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting ebi2_lcdc_rs = {
	.func = GPIOMUX_FUNC_3,
	.drv = GPIOMUX_DRV_12MA,
	.pull = GPIOMUX_PULL_DOWN,
};
#endif

static struct gpiomux_setting wlan_active_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting wlan_suspend_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

#ifdef CONFIG_SIERRA_INTERNAL_CODEC
static struct msm_gpiomux_config msm9615_audio_codec_configs[] __initdata = {
	{
		.gpio = 24,
		.settings = {
			[GPIOMUX_SUSPENDED] = &cdc_mclk,
		},
	},
};
#endif /* CONFIG_SIERRA */

#ifdef CONFIG_SIERRA_WIFI_SDCC2
static struct msm_gpiomux_config msm9615_sdcc2_configs[] __initdata = {
	{
		/* SDC2_DATA_0 */
		.gpio      = 25,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdcc2_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdcc2_suspend_cfg,
		},
	},
	{
		/* SDC2_DATA_1 */
		.gpio      = 26,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdcc2_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdcc2_cmd_data_0_3_actv_cfg,
		},
	},
	{
		/* SDC2_DATA_2 */
		.gpio      = 27,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdcc2_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdcc2_suspend_cfg,
		},
	},
	{
		/* SDC2_DATA_3 */
		.gpio      = 28,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdcc2_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdcc2_suspend_cfg,
		},
	},
	{
		/* SDC2_CMD */
		.gpio      = 29,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdcc2_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdcc2_suspend_cfg,
		},
	},
	{
		/* SDC2_CLK */
		.gpio      = 30,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdcc2_clk_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdcc2_suspend_cfg,
		},
	},
};
#endif

struct msm_gpiomux_config msm9615_ps_hold_config[] __initdata = {
	{
		.gpio = 83,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ps_hold,
		},
	},
};

static struct gpiomux_setting sd_card_det = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

struct msm_gpiomux_config sd_card_det_config[] __initdata = {
	{
		.gpio = 80,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sd_card_det,
			[GPIOMUX_SUSPENDED] = &sd_card_det,
		},
	},
};

/*
 * Low power reset configuration
 *
 * Some boards (like Sierra Wireless MangOH) have two reset signals (reset in
 * and application processor GPIO driven signal) ANDed to form active low
 * system reset signal. When this signal is applied, it will reset number of
 * peripherals including Arduino.
 * Active configuration starts as low output. It needs to stay like that for
 * at least 100ms. After that time, LowPower_RESET signal must be pulled high,
 * and must stay in that state while device is operational.
 * Required sink/source current is less than 1mA (1.8V/47K), so using 2mA drive
 * setup is OK.
 */
#ifdef CONFIG_SIERRA
static struct gpiomux_setting low_power_reset_active_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting low_power_reset_suspend_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_OUT_HIGH,
};

static struct msm_gpiomux_config low_power_reset_configs[] __initdata = {
	{
		.gpio     = GPIO_CF3_LOW_POWER_RESET_PIN,/* Low Power Reset */
		.settings = {
			[GPIOMUX_ACTIVE] = &low_power_reset_active_config,
			[GPIOMUX_SUSPENDED] = &low_power_reset_suspend_config,
		},
	},
};

static struct gpio_map {
	int ext_num;
	int int_num;
} user_gpio_map_cf3[] = {
	{2, 59},
	{6, 66},
	{7, 79},
	{8, 29},
	{13,84},
	{16,62},
	{21,50},
	{22,49},
	{23,54},
	{24,61},
	{25,73},
	{32,30},
	{33,78},
	{42,80},
};

static struct gpiomux_setting ext_gpio_init_setting = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct msm_gpiomux_config ext_gpio_init_configs[] = {
	{
		.settings = {
				[GPIOMUX_SUSPENDED] = &ext_gpio_init_setting,
		},
	},
};
#endif /* CONFIG_SIERRA */

#ifdef CONFIG_LTC4088_CHARGER
static struct msm_gpiomux_config
	msm9615_ltc4088_charger_config[] __initdata = {
	{
		.gpio = 4,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ltc4088_chg_cfg,
		},
	},
	{
		.gpio = 6,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ltc4088_chg_cfg,
		},
	},
	{
		.gpio = 7,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ltc4088_chg_cfg,
		},
	},
};
#endif

/* Configuration conflict checking 1: Single I2C port */
#if defined(CONFIG_SIERRA_GSBI2_I2C_GPIO) && defined(CONFIG_SIERRA_GSBI5_I2C_UART)
#error  "Conflict configuration of the I2C port \n"
#endif

/* Configuration conflict checking 2: One configuration per each GSBI */
#if defined(CONFIG_SIERRA_GSBI5_UART) && defined(CONFIG_SIERRA_GSBI5_I2C_UART)
#error  "Conflict configuration on GSBI5\n"
#endif

struct msm_gpiomux_config msm9615_gsbi_configs[] __initdata = {
#ifdef CONFIG_SIERRA_GSBI2_I2C_GPIO
	{
		.gpio      = 4,	/* GSBI2 I2C QUP SCL */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi2,
			[GPIOMUX_ACTIVE] = &gsbi2,
		},
	},
	{
		.gpio      = 5,	/* GSBI2 I2C QUP SDA */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi2,
		},
	},
#endif /* CONFIG_SIERRA_GSBI2_I2C_GPIO */

#ifdef CONFIG_SIERRA_GSBI3_SPI
	{
		.gpio      = 8,		/* GSBI3 QUP SPI_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi3,
		},
	},
	{
		.gpio      = 9,		/* GSBI3 QUP SPI_CS_N */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi3,
		},
	},
	{
		.gpio      = 10,	/* GSBI3 QUP SPI_DATA_MISO */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi3,
		},
	},
	{
		.gpio      = 11,	/* GSBI3 QUP SPI_DATA_MOSI */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi3,
		},
	},
#endif /*  CONFIG_SIERRA_GSBI3_SPI */

#ifdef CONFIG_SIERRA_GSBI4_UART
	{
		.gpio      = 12,	/* GSBI4 UART */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi4,
		},
	},
	{
		.gpio      = 13,	/* GSBI4 UART */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi4,
		},
	},
	{
		.gpio      = 14,	/* GSBI4 UART */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi4,
		},
	},
	{
		.gpio      = 15,	/* GSBI4 UART */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi4,
		},
	},
#endif /* CONFIG_SIERRA_GSBI4_UART */

#if defined(CONFIG_SIERRA_GSBI5_I2C_UART)
	{
		.gpio     = 16,    /* GSBI5 QUP I2C SCL */
		.settings = {
			[GPIOMUX_SUSPENDED] = &swi_gsbi_i2c_suspended,
			[GPIOMUX_ACTIVE]    = &swi_gsbi_i2c_active,
		},
	},
	{
		.gpio      = 17,    /* GSBI5 QUP I2C SDA */
		.settings = {
			[GPIOMUX_SUSPENDED] = &swi_gsbi_i2c_suspended,
		},
	},
#else
	{
		.gpio      = 16,	/* GSBI5 I2C QUP SCL */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi5,
			[GPIOMUX_ACTIVE] = &gsbi5,
		},
	},
	{
		.gpio      = 17,	/* GSBI5 I2C QUP SDA */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi5,
		},
	},
#endif  /* CONFIG_SIERRA_GSBI5_I2C_UART */

#if defined(CONFIG_SIERRA_GSBI5_UART) || defined(CONFIG_SIERRA_GSBI5_I2C_UART)
    {
        .gpio      = 18,    /* GSBI5 UART */
        .settings = {
            [GPIOMUX_SUSPENDED] = &gsbi5_uart,
        },
    },
    {
        .gpio      = 19,    /* GSBI5 UART */
        .settings = {
            [GPIOMUX_SUSPENDED] = &gsbi5_uart,
        },
    },
#else
#if 0
	{
		/* GPIO 19 can be used for I2C/UART on GSBI5 */
		.gpio      = 19,	/* GSBI3 QUP SPI_CS_1 */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi3_cs1_config,
		},
	},
#endif
#endif /* CONFIG_SIERRA_GSBI5_UART || CONFIG_SIERRA_GSBI5_I2C_UART */
};

#ifdef CONFIG_SIERRA_INTERNAL_CODEC
static struct msm_gpiomux_config msm9615_slimbus_configs[] __initdata = {
	{
		.gpio      = 20,	/* Slimbus data */
		.settings = {
			[GPIOMUX_SUSPENDED] = &slimbus,
		},
	},
	{
		.gpio      = 23,	/* Slimbus clk */
		.settings = {
			[GPIOMUX_SUSPENDED] = &slimbus,
		},
	},
};
#endif /* CONFIG_SIERRA_INTERNAL_CODEC */

#ifdef CONFIG_FB_MSM_EBI2
static struct msm_gpiomux_config msm9615_ebi2_lcdc_configs[] __initdata = {
	{
		.gpio      = 21,	/* a_d */
		.settings = {
			[GPIOMUX_SUSPENDED] = &ebi2_lcdc_a_d,
		},
	},
	{
		.gpio      = 22,	/* cs */
		.settings = {
			[GPIOMUX_SUSPENDED] = &ebi2_lcdc_cs,
		},
	},
	{
		.gpio      = 24,	/* rs */
		.settings = {
			[GPIOMUX_SUSPENDED] = &ebi2_lcdc_rs,
		},
	},
};
#endif

static struct msm_gpiomux_config msm9615_wlan_configs[] __initdata = {
	{
		.gpio      = 21,/* WLAN_RESET_N */
		.settings = {
			[GPIOMUX_ACTIVE] = &wlan_active_config,
			[GPIOMUX_SUSPENDED] = &wlan_suspend_config,
		},
	},
};


int __init msm9615_init_gpiomux(void)
{
	int rc;
	int i;

	rc = msm_gpiomux_init(NR_GPIO_IRQS);
	if (rc) {
		pr_err(KERN_ERR "msm_gpiomux_init failed %d\n", rc);
		return rc;
	}
	msm_gpiomux_install(msm9615_gsbi_configs,
			ARRAY_SIZE(msm9615_gsbi_configs));

#ifdef CONFIG_SIERRA_INTERNAL_CODEC
	if(bssupport(BSFEATURE_WM8944) == false)
		msm_gpiomux_install(msm9615_slimbus_configs,
				ARRAY_SIZE(msm9615_slimbus_configs));
#endif /* CONFIG_SIERRA_INTERNAL_CODEC */

	msm_gpiomux_install(msm9615_ps_hold_config,
			ARRAY_SIZE(msm9615_ps_hold_config));

	msm_gpiomux_install(sd_card_det_config,
			ARRAY_SIZE(sd_card_det_config));
#ifdef CONFIG_SIERRA_WIFI_SDCC2
	msm_gpiomux_install(msm9615_sdcc2_configs,
			ARRAY_SIZE(msm9615_sdcc2_configs));
#endif /* CONFIG_SIERRA_WIFI_SDCC2 */

#ifdef CONFIG_LTC4088_CHARGER
	msm_gpiomux_install(msm9615_ltc4088_charger_config,
			ARRAY_SIZE(msm9615_ltc4088_charger_config));
#endif

#ifdef CONFIG_SIERRA_INTERNAL_CODEC
	if(bssupport(BSFEATURE_CF3) == false)
		msm_gpiomux_install(msm9615_audio_codec_configs,
				ARRAY_SIZE(msm9615_audio_codec_configs));
#endif /* CONFIG_SIERRA_INTERNAL_CODEC */

	msm_gpiomux_install(msm9615_wlan_configs,
			ARRAY_SIZE(msm9615_wlan_configs));

#ifdef CONFIG_FB_MSM_EBI2
	msm_gpiomux_install(msm9615_ebi2_lcdc_configs,
			ARRAY_SIZE(msm9615_ebi2_lcdc_configs));
#endif

#ifdef CONFIG_SIERRA
	if (bssupport(BSFEATURE_CF3))
	{
		/* initialize user gpios as input no pull */
		for (i = 0; i < ARRAY_SIZE(user_gpio_map_cf3); i++)
		{
			if (bsgpioenabled(user_gpio_map_cf3[i].ext_num))
			{
				ext_gpio_init_configs[0].gpio = user_gpio_map_cf3[i].int_num;
				msm_gpiomux_install(ext_gpio_init_configs,
						ARRAY_SIZE(ext_gpio_init_configs));
			}
		}

		/*
		* Install LowPower_RESET only for WP85xx device family. This also includes
		* at least one platform which is not from WP85 family. For complete list of
		* BSFEATURE_CF3 platforms please, take a look at the
		* arch/arm/mach-msm/board-9615.c:bssupport().
		*/
		if (bsgpioresetenabled())
		{
			msm_gpiomux_install(low_power_reset_configs,
					ARRAY_SIZE(low_power_reset_configs));
		}
	}
#endif

	/* This call will fail if module family is not WP85, but we do not care. */
	gpio_cf3_low_power_reset_toggle();

	return 0;
}
