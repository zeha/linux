/* kernel/include/linux/sierra_bsuproto.h
 *
 * Copyright (C) 2013 Sierra Wireless, Inc
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
 
#ifndef BS_UPROTO_H
#define BS_UPROTO_H

extern enum bshwtype bsgethwtype(void);
extern uint8_t bsgethwrev(void);
extern uint32_t bsgetmanufacturingcode(void);
extern bool bssupport(enum bsfeature feature);
extern uint32_t bsreadboottoappflag(void);
extern uint64_t bsgetgpioflag(void);
extern bool bsuart4modem(uint uart_num );
extern int8_t bsgetuartfun(uint);
extern int8_t bsgetriowner(void);
extern bool bsgpioresetenabled(void);
extern bool bsgpioenabled(int gpio);
#endif
