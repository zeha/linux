/*
 * include/uapi/linux/spi/sierra_spi.h
 *
 * Copyright (C) 2015 Sierra Wireless, Inc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  */

#ifndef SWI_SPIDEV_H
#define SWI_SPIDEV_H

#include <linux/types.h>

/* User space versions of kernel symbols for SPI clocking modes,
 * matching <linux/spi/sierra_spi.h>
 */

/* SPI_IO_CONTROL fields */
#define SPI_IO_C_FORCE_CS             0x00000800
#define SPI_IO_C_CLK_IDLE_HIGH        0x00000400
#define SPI_IO_C_CLK_ALWAYS_ON        0x00000200
#define SPI_IO_C_MX_CS_MODE           0x00000100
#define SPI_IO_C_CS_N_POLARITY        0x000000F0
#define SPI_IO_C_CS_N_POLARITY_0      0x00000010
#define SPI_IO_C_CS_SELECT            0x0000000C
#define SPI_IO_C_TRISTATE_CS          0x00000002
#define SPI_IO_C_NO_TRI_STATE         0x00000001

#endif /* SPIDEV_H */
