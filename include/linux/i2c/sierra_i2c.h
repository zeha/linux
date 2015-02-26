/* arch/arm/mach-msm/sierra_i2c.h
 *
 * Copyright (C) 2013 Sierra Wireless, Inc
 * Author: Alex Tan <atan@sierrawireless.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef SIERRA_I2C_H
#define SIERRA_I2C_H
#include <linux/types.h>
#include <linux/i2c.h>

#define SWI_IOCTL_MAGIC_NUM 'I'
#define SWI_IOCTL_I2C_ADDR_CONFIG _IOW(SWI_IOCTL_MAGIC_NUM,0x1,int)
#define SWI_IOCTL_I2C_FREQ_CONFIG _IOW(SWI_IOCTL_MAGIC_NUM,0x2,int)

extern int swi_set_i2c_freq(struct i2c_adapter *adap,int freq);
#endif

