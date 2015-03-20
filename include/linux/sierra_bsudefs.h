/* kernel/include/linux/sierra_bsudefs.h
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

#ifndef BS_UDEFS_H
#define BS_USDEFS_H
/************
 *
 * Name:     bshwtype
 *
 * Purpose:  To enumerate hardware types
 *
 * Members:  See below
 *
 * Notes:    If adding items to this enum, make sure arrays using these enums
 *           are updated.
 *
 ************/
enum bshwtype
{
  BSQCTMTP,                 /* Qualcomm MTP */
  BSHWNONE,                 /* HW type NONE (Fuse has not been blown yet) */
  BSAC770S,                 /* AirCard 770S (HANSEL) */
  BSMC7355,                 /* MiniCard 7355 */
  BSAR7550,                 /* Automotive 7550 */
  BSAR7552,                 /* Automotive 7552 */
  BSAR7554,                 /* Automotive 7554 */
  BSEM7355,                 /* Embedded Module 7355 */
  BSAC340U,                 /* AirCard 340U (GRADY) */
  BSWP7100,                 /* WP7100 (for Verizon) */
  BSWP7102,                 /* WP7102 (for AT&T) */
  BSWP7104,                 /* WP7104 (for EU&APAC) */
  BSMC7305,                 /* MiniCard 7305 */
  BSEM7305,                 /* Embedded Module 7305 */
  BSAC342U,                 /* AirCard 342U (OSSO) */
  BSAC341U,                 /* AirCard 341U */
  BSEM7655,                 /* EM7655 (TFF version of EM7355) */
  BSMC8805,                 /* MiniCard 8805 */
  BSEM8805,                 /* Embeddded Module 8805 */
  BSAC771S,                 /* AirCard 771S */
  BSYW7X55,                 /* LGA module - experimental */
  BSWP7100_INSIM,           /* WP7100 (with In-SIM) */
  BSWP7102_INSIM,           /* WP7102 (with In-SIM) */
  BSWP7104_INSIM,           /* WP7104 (with In-SIM) */
  BSMC7350,                 /* MiniCard 7350 (for Verizon & Sprint) */
  BSMC7350L,                /* MiniCard 7350-L (for Verizon)*/
  BSMC7802,                 /* MiniCard 7802 (for AT&T) */
  BSMC7304,                 /* MiniCard 7304 (for EU) */
  BSWP7100_LARGER_MEMORY,   /* WP7100 (with Larger Memory Design) */
  BSWP7102_LARGER_MEMORY,   /* WP7102 (with Larger Memory Design) */
  BSWP7104_LARGER_MEMORY,   /* WP7104 (with Larger Memory Design) */
  BSEM7330,                 /* Embedded Module 7330 */
  BSMC7330,                 /* MiniCard 7330 */
  BSAC343U,                 /* AirCard 343U */
  BSMC7371,                 /* MiniCard 7371 */
  BSAC778S,                 /* AirCard 778S */
  BSAR7550_LARGER_MEMORY,   /* Automotive 7550  (with Larger Memory Design) */
  BSAR7552_LARGER_MEMORY,   /* Automotive 7552  (with Larger Memory Design) */
  BSAR7554_LARGER_MEMORY,   /* Automotive 7554  (with Larger Memory Design) */
  BSWP7100_NEW,             /* WP7100 with large memory based on AR PCB */
  BSWP7102_NEW,             /* WP7102 with large memory based on AR PCB */
  BSWP7104_NEW,             /* WP7104 with large memory based on AR PCB */
  BSMC7354,                 /* MiniCard 7354 */
  BSAR7558_LARGER_MEMORY,   /* Automotive 7558  (with Larger Memory Design) */
  BSAR7556,                 /* Automotive 7556 */
  BSWP75XX,                 /* WP75xx - WP75 family, RF board unknown */
  BSWP85XX,                 /* WP85xx - WP85 family, RF board unknown */
  BSWP8548,                 /* WP8548 */
  BSWP8548G,                /* WP8548G */
  BSAR8652,                 /* Automotive 8652 */
  BSAR7556_LARGER_MEMORY,   /* Automotive 7556 (with Larger Memory Design) */
  BSAR7554RD,               /* Automotive 7554 RD */
  BSAR7552RD,               /* Automotive 7552 RD */
  BSHWUNKNOWN,              /* Unknown HW */
  BSHWINVALID = 0xFF        /* Invalid HW */
};

/************
 *
 * Name:     bsproctype
 *
 * Purpose:  Enumerate processor types
 *
 * Members:  BSPROC_UNKNOWN    - unknown
 *           BSPROC_APPS       - Application processor
 *           BSPROC_MODEM      - Modem processor
 *           BSPROC_MAX        - Used for bounds checking
 *
 * Notes:    None
 *
 ************/
enum bsproctype
{
  BSPROC_UNKNOWN = 0,
  BSPROC_APPS,
  BSPROC_MODEM,

  BSPROC_MAX
};

/************
 *
 * Name:     bshwrev
 *
 * Purpose:  To enumerate hardware revisions
 *
 * Members:  See below
 *
 * Notes:
 *          For DV2.1, the HW rev in FSN should be "02" and the GPIO40/45 should both be low
 *            then BSHWDV2 = 2 << 2, i.e. 8
 *
 *          For DV3.1, the HW rev in FSN should be "03" and the GPIO40/45 should both be low
 *            then BSHWDV3 = 3 << 2, i.e. 12
 *
 ************/
enum bshwrev
{
  BSHWREV0 = 0,         /* Revision 0 */
  BSHWDV1  = 4,         /* DV1.1 */
  BSHWDV2  = 8,         /* DV2.1 */
  BSHWDV3  = 12,        /* DV3.1 */
  BSHWDV4  = 16,        /* DV4.1 */
  BSHWDV5  = 20,        /* DV5.1 */
  BSHW10   = 40,        /* HW 1.0 */
  BSHWPP = BSHW10,      /* Production HW; sync with naming convension for now */
  BSHW11   = 44,        /* HW 1.1 */

  BSHWREVMAX = 63,      /* maximum possible HW revision */
  BSHWREVUNKNOWN = 0xFF /* unknown revision */
};

/************
 *
 * Name:     bsfeature
 *
 * Purpose:  Enumerated list of different features supported by different hardware variants
 *
 * Members:  See below
 *
 * Notes:    None
 *
 ************/
enum bsfeature
{
  BSFEATURE_MINICARD,       /* if the hardware is a MiniCard */
  BSFEATURE_MINICARD_M2M,   /* if the HW support MC M2M features */
  BSFEATURE_EM,             /* if the device is EM product */
  BSFEATURE_AR,             /* if the hardware is an AR product */
  BSFEATURE_WP,             /* if the hardware is a WP71 product */
  BSFEATURE_CF3,            /* if the hardware is a CF3 product */
  BSFEATURE_W_DISABLE,      /* if W_DISABLE is supported */
  BSFEATURE_SD,             /* if SD is supported */
  BSFEATURE_VOICE,          /* if voice is supported */
  BSFEATURE_HSUPA,          /* if the hardware supports HSUPA */
  BSFEATURE_GPIOSAR,        /* if GPIO controlled SAR backoff is supported */
  BSFEATURE_RMAUTOCONNECT,  /* if auto-connect feature is device centric */
  BSFEATURE_UART,           /* if the hardware support UART */
  BSFEATURE_ANTSEL,         /* if the hardware supports ANTSEL */
  BSFEATURE_INSIM,          /* Internal SIM supported (eSIM) */
  BSFEATURE_OOBWAKE,        /* if has OOB_WAKE GPIO */
  BSFEATURE_CDMA,           /* if the hardware supports CDMA/1x */
  BSFEATURE_GSM,            /* if the hardware supports GSM/EDGE */
  BSFEATURE_WCDMA,          /* if the hardware supports WCDMA */
  BSFEATURE_LTE,            /* if the hardware supports LTE */
  BSFEATURE_TDSCDMA,        /* if the hardware supports TDSCDMA */
  BSFEATURE_GPSSEL,         /* if GPS antenna selection is supported */
  BSFEATURE_SVC_PIN_DLOAD,  /* if service pin for DL mode is supported */
  BSFEATURE_BUZZER,         /* if the hardware supports Buzzer */
  BSFEATURE_OSA,            /* if Open SIM Access supported */
  BSFEATURE_ATPORTSW,       /* if the hardware support at port switch */
  BSFEATURE_VDDMIN_MPP1,    /* If use PMIC MPP01 for USB wakeup */
  BSFEATURE_SIMHOTSWAP,     /* if the hardware supports SIM detection via GPIO */
  BSFEATURE_DR,             /* if Data Reliability is supported */
  BSFEATURE_WM8944,         /* if WM8944 codec */
  BSFEATURE_MAX
};

/************
 *
 * Name:     bsuartfunc
 *
 * Purpose:  Enumerated list of different functions supported by App processor
 *
 * Members:  BSUARTFUNC_INVALID  - UART unavilable for APP
 *           BSUARTFUNC_DM - UART reserved for DM service
 *           BSUARTFUNC_CONSOLE - UART reserved for CONSOLE service
 *           BSUARTFUNC_APP - UART open for all application usage
 *
 * Notes:    None
 *
 ************/
enum bsuartfunc
{
  BSUARTFUNC_INVALID = 0,
  BSUARTFUNC_DM      = 2,
  BSUARTFUNC_CONSOLE = 16,
  BSUARTFUNC_APP     = 17,
};

/************
 *
 * Members:  BS_UART1_LINE  - line number of UART1
 *           BS_UART2_LINE - line number of UART2
 *
 * Notes:    None
 *
 ************/
#define BS_UART1_LINE  0
#define BS_UART2_LINE  1

/************
 *
 * Members:  BS_RESET_TYPE_NONE    - the module not reset
 *           BS_RESET_TYPE_UNKNOW  - unknown reset of module
 *           BS_RESET_TYPE_SW      - software reset
 *           BS_RESET_TYPE_HW      - hardware reset
 *           BS_RESET_TYPE_CRASH   - modem crash
 * Notes:    None
 *
 ************/
#define BS_RESET_TYPE_NONE     0
#define BS_RESET_TYPE_UNKNOW   1
#define BS_RESET_TYPE_SOFT     2
#define BS_RESET_TYPE_HW       3
#define BS_RESET_TYPE_CRASH    4

#include "sierra_bsuproto.h"
#endif
