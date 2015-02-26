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
 * Members:  BSQCTMTP    - Qualcomm MTP
 *           BSHWNONE    - HW type NONE (Fuse has not been blown yet)
 *           BSAC770S    - AirCard 770S (HANSEL)
 *           BSMC7355    - MiniCard 7355
 *           BSAR7550    - Automotive 7550
 *           BSAR7552    - Automotive 7552
 *           BSAR7554    - Automotive 7554
 *           BSEM7355    - Embedded Module 7355
 *           BSAC340U    - AirCard 340U (GRADY)
 *           BSWP7100    - WP7100 (for Verizon)
 *           BSWP7102    - WP7102 (for AT&T)
 *           BSWP7104    - WP7104 (for EU&APAC)
 *           BSMC7305    - MiniCard 7305
 *           BSEM7305    - Embedded Module 7305
 *           BSAC342U    - AirCard 342U (OSSO)
 *           BSEM7655    - EM7655 (TFF version of EM7355)
 *           BSMC8805    - MiniCard 8805
 *           BSEM8805    - Embeddded Module 8805
 *           BSWP7100_INSIM    - WP7100 (with In-SIM)
 *           BSWP7102_INSIM    - WP7102 (with In-SIM)
 *           BSWP7104_INSIM    - WP7104 (with In-SIM)
 *           BSMC7350    - MiniCard 7350 (for Verizon & Sprint)
 *           BSMC7350L   - MiniCard 7350 LTE Only (for Verizon)
 *           BSMC7802    - MiniCard 7802 (for AT&T)
 *           BSMC7304    - MiniCard 7304 (for EU)
 *           BSWP7100_LARGER_MEMORY    - WP7100 (with Larger Memory Design)
 *           BSWP7102_LARGER_MEMORY    - WP7102 (with Larger Memory Design)
 *           BSWP7104_LARGER_MEMORY    - WP7104 (with Larger Memory Design)
 *           BSEM7330    - Embedded Module 7330
 *           BSMC7330    - MiniCard 7330
 *           BSAC343U    - AirCard 343U (HERMES WORLD MODE)
 *           BSMC7371    - MiniCard 7371
 *           BSAC778S    - Aircard 778S 
 *           BSAR7550_LARGER_MEMORY    - Automotive 7550 (with Larger Memory Design) 
 *           BSAR7552_LARGER_MEMORY    - Automotive 7552 (with Larger Memory Design) 
 *           BSAR7554_LARGER_MEMORY    - Automotive 7554 (with Larger Memory Design) 
 *           BSAR7558_LARGER_MEMORY    - Automotive 7558 (with Larger Memory Design)
 *           BSWP7100_NEW    - WP7100 with large memory and share same PCB with BSAR7550_LARGER_MEMORY
 *           BSWP7102_NEW    - WP7100 with large memory and share same PCB with BSAR7552_LARGER_MEMORY 
 *           BSWP7104_NEW    - WP7100 with large memory and share same PCB with BSAR7554_LARGER_MEMORY
 *           BSMC7354    - MiniCard 7354              
 *           BSHWUNKNOWN - Unknown HW
 *           BSHWINVALID - Invalid HW
 *
 * Notes:    None
 *
 ************/
enum bshwtype
{
  BSQCTMTP,
  BSHWNONE,
  BSAC770S,
  BSMC7355,
  BSAR7550,
  BSAR7552,
  BSAR7554,
  BSEM7355,
  BSAC340U,
  BSWP7100,
  BSWP7102,
  BSWP7104,
  BSMC7305,
  BSEM7305,
  BSAC342U,
  BSAC341U,
  BSEM7655,
  BSMC8805,
  BSEM8805,
  BSAC771S,
  BSYW7X55,
  BSWP7100_INSIM,   /* this is kept to align with BS_QFPROM_PROD_ID */
  BSWP7102_INSIM,   /* this is kept to align with BS_QFPROM_PROD_ID */
  BSWP7104_INSIM,   /* this is kept to align with BS_QFPROM_PROD_ID */
  BSMC7350,
  BSMC7350L,
  BSMC7802,
  BSMC7304,
  BSWP7100_LARGER_MEMORY,
  BSWP7102_LARGER_MEMORY,
  BSWP7104_LARGER_MEMORY, 
  BSEM7330,
  BSMC7330,
  BSAC343U,
  BSMC7371,
  BSAC778S,
  BSAR7550_LARGER_MEMORY,
  BSAR7552_LARGER_MEMORY,
  BSAR7554_LARGER_MEMORY,
  BSWP7100_NEW,
  BSWP7102_NEW,
  BSWP7104_NEW,  
  BSMC7354,                 /* MiniCard 7354 */
  BSAR7558_LARGER_MEMORY,
  BSHWUNKNOWN,
  BSHWINVALID = 0xFF
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
 * Members:  BSHWREV0              - Revision 0
 *           BSHWDV1               - HW DV1
 *           BSHWDV2               - HW DV2
 *           BSHWDV3               - HW DV3
 *           BSHWAC770SDV1         - Hansel DV1
 *           BSHWREVMAX            - maximum possible HW revision
 *           BSHWREVUNKNOWN - unknown revision
 *
 * Notes:
 *          still keep BSHWREV0 for a while for backward compatible
 *
 *          In the early development phase, the HWREV of DV1 for some device may not be 4
 *            define device specific enum here if needed, e.g. BSHWAC770SDV1
 *          
 *          For DV2.1, the HW rev in FSN should be "02" and the GPIO40/45 should both be low
 *            then BSHWDV2 = 2 << 2, i.e. 8
 *
 *          For DV3.1, the HW rev in FSN should be "03" and the GPIO40/45 should both be low
 *            then BSHWDV3 = 3 << 2, i.e. 12
 * 
 ************/
enum bshwrev
{
  BSHWREV0 = 0,
  BSHWDV1 = 4,
  BSHWDV2 = 8,
  BSHWDV3 = 12,
  BSHWPP  = 40,

  BSHWAC770SDV1 = 44,  /* AC770S DV1 */
  
  BSHWREVMAX = 59,
  BSHWREVUNKNOWN = 0xFF
};

/************
 *
 * Name:     bsfeature
 *
 * Purpose:  Enumerated list of different features supported by different hardware variants
 *
 * Members:  BSFEATURE_MINICARD  - if the hardware is a MiniCard
 *           BSFEATURE_MINICARD_M2M - if the HW support MC M2M features
 *           BSFEATURE_USB       - if the hardware is a USB dongle
 *           BSFEATURE_MHS       - if the hardware is a Mobile Hotspot product
 *           BSFEATURE_AR        - if the hardware is an AR product
 *           BSFEATURE_WP        - if the hardware is a WP product
 *           BSFEATURE_W_DISABLE - if W_DISABLE is supported
 *           BSFEATURE_SD        - if SD is supported
 *           BSFEATURE_VOICE     - if voice is supported
 *           BSFEATURE_HSUPA     - if the hardware supports HSUPA
 *           BSFEATURE_GPIOSAR  -  if GPIO controlled SAR backoff is supported
 *           BSFEATURE_RMAUTOCONNECT - if auto-connect feature is device centric
 *           BSFEATURE_UART      - if the hardware support UART
 *           BSFEATURE_ANTSEL    - if the hardware supports ANTSEL
 *           BSFEATURE_INSIM     - Internal SIM supported (eSIM)
 *           BSFEATURE_OOBWAKE   - if has OOB_WAKE GPIO
 *           BSFEATURE_CDMA      - if the hardware supports CDMA/1x
 *           BSFEATURE_GSM       - if the hardware supports GSM/EDGE
 *           BSFEATURE_WCDMA     - if the hardware supports WCDMA
 *           BSFEATURE_LTE       - if the hardware supports LTE
 *           BSFEATURE_EM        - if device is EM product
 *           BSFEATURE_SVC_PIN_DLOAD - if service pin for DL mode is supported
 *           BSFEATURE_BUZZER    - if the hardware supports Buzzer
 *           BSFEATURE_MAX       - Used for bounds checking
 *
 * Notes:    None
 *
 ************/
enum bsfeature
{
  BSFEATURE_MINICARD,
  BSFEATURE_MINICARD_M2M,
  BSFEATURE_USB,
  BSFEATURE_MHS,
  BSFEATURE_AR,
  BSFEATURE_WP,
  BSFEATURE_W_DISABLE,
  BSFEATURE_SD,
  BSFEATURE_VOICE,
  BSFEATURE_HSUPA,
  BSFEATURE_GPIOSAR,
  BSFEATURE_RMAUTOCONNECT,
  BSFEATURE_UART,
  BSFEATURE_ANTSEL,
  BSFEATURE_INSIM,
  BSFEATURE_OOBWAKE,
  BSFEATURE_CDMA,
  BSFEATURE_GSM,
  BSFEATURE_WCDMA,
  BSFEATURE_LTE,
  BSFEATURE_EM,
  BSFEATURE_SVC_PIN_DLOAD,
  BSFEATURE_BUZZER,
  BSFEATURE_MAX
};


#endif
