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
 */

#ifndef __ARCH_ARM_MACH_MSM_BOARD_9615_H
#define __ARCH_ARM_MACH_MSM_BOARD_9615_H

#include <mach/irqs.h>
#include <linux/mfd/pm8xxx/pm8018.h>
#include <linux/regulator/msm-gpio-regulator.h>

/*
 * MDM9x15 I2S.
 */
#ifdef CONFIG_I2C

#define I2C_SURF    (1)
#define I2C_FFA     (1 << 1)
#define I2C_RUMI    (1 << 2)
#define I2C_SIM     (1 << 3)
#define I2C_FLUID   (1 << 4)
#define I2C_LIQUID  (1 << 5)

struct i2c_registry {
	u8                     machs;
	int                    bus;
	struct i2c_board_info *info;
	int                    len;
};
#endif

/* Tabla slave address for I2C */
#define TABLA_I2C_SLAVE_ADDR            (0x0d)
#define TABLA_ANALOG_I2C_SLAVE_ADDR     (0x77)
#define TABLA_DIGITAL1_I2C_SLAVE_ADDR   (0x66)
#define TABLA_DIGITAL2_I2C_SLAVE_ADDR   (0x55)
#define MSM_9615_GSBI5_QUP_I2C_BUS_ID   (0x00)

/* wm8944 */
#define WM8944_I2C_SLAVE_ADDR           (0x1a)

/*
 * MDM9x15 I2S.
 */

/* Macros assume PMIC GPIOs and MPPs start at 1 */
#define PM8018_GPIO_BASE                NR_GPIO_IRQS
#define PM8018_GPIO_PM_TO_SYS(pm_gpio)  (pm_gpio - 1 + PM8018_GPIO_BASE)
#define PM8018_MPP_BASE                 (PM8018_GPIO_BASE + PM8018_NR_GPIOS)
#define PM8018_MPP_PM_TO_SYS(pm_gpio)   (pm_gpio - 1 + PM8018_MPP_BASE)
#define PM8018_IRQ_BASE                 (NR_MSM_IRQS + NR_GPIO_IRQS)
#define PM8018_MPP_IRQ_BASE             (PM8018_IRQ_BASE + NR_GPIO_IRQS)

/* Macro assumes SWIMCU GPIOs & ADCs start at 0 */
#define SWIMCU_GPIO_BASE                (PM8018_MPP_BASE + PM8018_NR_MPPS)
#define SWIMCU_GPIO_TO_SYS(mcu_gpio)    (mcu_gpio + SWIMCU_GPIO_BASE)
#define SWIMCU_NR_GPIOS                 8
#define SWIMCU_IS_GPIO(gpio)            ((gpio >= SWIMCU_GPIO_BASE) && (gpio < SWIMCU_GPIO_BASE + SWIMCU_NR_GPIOS))
#define SWIMCU_ADC_BASE                 2
#define SWIMCU_ADC_TO_SYS(mcu_adc)      (mcu_adc + SWIMCU_ADC_BASE)
#define SWIMCU_NR_ADCS                  2

extern struct pm8xxx_regulator_platform_data
	msm_pm8018_regulator_pdata[];

extern int msm_pm8018_regulator_pdata_len;

extern struct rpm_regulator_platform_data
	msm_rpm_regulator_9615_pdata;

#define GPIO_VREG_ID_EXT_2P95V  (0x00)

extern struct gpio_regulator_platform_data msm_gpio_regulator_pdata[];
uint32_t msm9615_rpm_get_swfi_latency(void);
int msm9615_init_gpiomux(void);
void msm9615_init_mmc(void);
void mdm9615_allocate_fb_region(void);
void mdm9615_init_fb(void);

/*******************************************************************************
 *
 * 				CF3 (WP75/WP85 based)
 *
 ******************************************************************************/

/* LowPower_RESET pin. For now, it is safe to assume that all CF3 platforms
   could handle this pin gracefully (e.g. it will have the same function, or
   would not have any function at all). */
#define GPIO_CF3_LOW_POWER_RESET_PIN    (66)

/* Setup for LowPower_RESET toggle. Returns 0 if everything is OK. */
int gpio_cf3_low_power_reset_toggle(void);

#endif
