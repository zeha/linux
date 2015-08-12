/* arch/arm/mach-msm/sierra_smem.h
 *
 * Copyright (C) 2012 Sierra Wireless, Inc
 * Author: Brad Du <bdu@sierrawireless.com>
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

#ifndef SIERRA_SMEM_H
#define SIERRA_SMEM_H

#include <mach/msm_iomap.h>
#include <asm/stacktrace.h>

#define BS_SECURE_RAM_SIZE             0x6000
#define SIERRA_SMEM_SIZE               (0x5000 + BS_SECURE_RAM_SIZE)     /* same as BSRAM_SIZE_SMI */
#define SIERRA_SMEM_BASE               (MSM_SHARED_RAM_BASE + MSM_SHARED_RAM_SIZE - SIERRA_SMEM_SIZE)
/* this is actually BS_ER_ABORT_DATA_MODEM_START */
#define SIERRA_SMEM_ERR_DUMP_START     (SIERRA_SMEM_BASE + SIERRA_SMEM_SIZE - 0x1828)
#define BS_BOOT_APP_MSG_START          (SIERRA_SMEM_BASE + SIERRA_SMEM_SIZE - 0x1020)

/* Cooperative mode message */
#define BS_COWORK_MSG_START            (SIERRA_SMEM_BASE + SIERRA_SMEM_SIZE - 0xAEAC)

/* Local constants and enumerated types */
#define ERROR_START_MARKER  0x4552 /* "ER" in ASCII */
#define ERROR_END_MARKER    0x4552 /* "ER" in ASCII */
#define ERROR_USER          0x0101
#define ERROR_EXCEPTION     0x0202
#define ERROR_FATAL_ERROR   0x0404
#define ERROR_LOCK_MARKER   0x0303
#define ERROR_START_GLOBALTIME_MARKER   0x47744774
#define ERROR_END_GLOBALTIME_MARKER     0x47744774

#define MAX_SERIAL_LEN      20  /* must be larger than (NV_UE_IMEI_SIZE-1)*2 */
#define MAX_VER_LEN         22
#define DATE_TIME_LEN       16

#define ERROR_STRING_LEN    64
#define MAX_STACK_DATA      32
#define MAX_TASK_NAME       12
#define MAX_ARM_REGISTERS   15
#define MAX_EXT_REGISTERS   17
#define QDSP6_REG_SP        (29 - MAX_ARM_REGISTERS)  /* R29 = SP */
#define QDSP6_REG_FP        (30 - MAX_ARM_REGISTERS)  /* R30 = FP */
#define QDSP6_REG_LR        (31 - MAX_ARM_REGISTERS)  /* R31 = LR */

#define MAX_FORMAT_PARAM    4

#define DUMP_SET_FLAG       0x0001

#define BC_VALID_BOOT_MSG_MARKER           0xBABECAFEU   /* indicates message from Boot to App */
#define BC_MSG_MARKER_M                    0xFFFF0000U
#define BCBOOTAPPFLAG_DLOAD_MODE_M         0x00000008

#define BC_VALID_COWORK_MSG_MARKER         0xCD3AE0B5U  /*cooperation mode message start & end marker*/

#define ERDUMP_SAVE_CMD_START              0xFF00
#define ERDUMP_SAVE_CMD_ERRSTR             0xFF01
#define ERDUMP_SAVE_CMD_ERRDATA            0xFF02
#define ERDUMP_SAVE_CMD_FMTSTR             0xFF03
#define ERDUMP_SAVE_CMD_FMTDATA            0xFF04
#define ERDUMP_SAVE_CMD_REGISTERS          0xFF05
#define ERDUMP_SAVE_CMD_FRAME              0xFF06
#define ERDUMP_SAVE_CMD_END                0xFF0F


/* Structures */

/*************
 *
 * Name:     sER_DATA - ER Data structure
 *
 * Purpose:  Contains ER data dumps
 *
 * Members:  start_marker    - buffer pointer
 *           error_source    - user or exception vector
 *           error_data      - passed error data
 *           error_string[]  - Null-terminated string
 *           task_name[]     -
 *           time_stamp      - modem up time in seconds since power up
 *           registers[]     - currently visible register set, may be exception
 *                             registers rather than normal register
 *           program_counter - code location error occurred
 *           cpsr            - Current Program Status Register
 *           stack_data[]    -
 *           end_marker      - end marker to mark the availability of crucial info
 *           app_ver         - APPL release at the time of crash
 *           boot_ver        - BOOT release at the time of crash
 *           swoc_ver        - SWoC release at the time of crash
 *           serial_num      - modem IMEI
 *           date_time       - date/time at the time of crash
 *           flags           - flag to indicate whether the crash info sent to host or not
 *           unused          - reserved for future use
 *           ext_marker      - end marker to mark the availability of extended crash info
 *           ext_registers   - extra register set for QDSP6 (R15-R31)
 *                             valid if R29 R30 R31 (SP FP LR) are non-zero
 *           format_string   - error_string/error_data will store error file name/line number
 *                             so the error string and params will be actually stored in
 *                             format_string and param
 *                             SWI_TBD BD:09:12:16 - need add format string to XML message 
 *           param           - params for format_string, current 3 possible param for ERR_FATAL 
 *           reserved        - reserved for future use
 *           format_marker   - marker to indicate that field format_string and after are valid
 *
 * Notes:    make sure uint32 fields is 4-byte aligned
*           IMPORTANT: if offset of registers/program_counter/cpsr/stack_data or
*           ext_registers is changed, need to update hardcoded offset in erqdsp.s
 *
 **************/
struct __packed sER_DATA
{
    uint16_t start_marker;
    uint16_t error_source;
    uint32_t error_data;
    char     error_string[ERROR_STRING_LEN];
    char     task_name[MAX_TASK_NAME];
    uint32_t time_stamp;
    uint32_t registers[MAX_ARM_REGISTERS];
    uint32_t program_counter;
    uint32_t cpsr;
    uint32_t stack_data[MAX_STACK_DATA];
    uint16_t end_marker;
    char     app_ver[MAX_VER_LEN];
    char     boot_ver[MAX_VER_LEN];
    char     swoc_ver[MAX_VER_LEN];
    char     serial_num[MAX_SERIAL_LEN];
    char     date_time[DATE_TIME_LEN];
    uint16_t flags;
    uint16_t ext_marker;
    uint32_t ext_registers[MAX_EXT_REGISTERS];
    char     format_string[ERROR_STRING_LEN];
    uint32_t param[MAX_FORMAT_PARAM];
    uint32_t reserved[MAX_STACK_DATA];
    uint16_t format_marker;
};

/*************
 *
 * Name:     bcboottoappmsg - Boot Loader to Application message structure
 *
 * Purpose:  To provide a structure which allows information exchange between
 *           the boot loader and the application.
 *
 * Members:
 *           bcstartmarker  - marker indicating the start of this structure
 *           flashdevdp     - pointer to flash device descriptor
 *           launchcode     - ASCII code used to detect that
 *                            application is to be launched
 *           usbdescp       - pointer to USB descriptors structure
 *           loopback       - bitfield for messages passed from the application
 *                            back to itself over a warm boot
 *           hwconfig       - hardware configuration
 *           flags          - boot -> app flags, bitmasked
 *           partisz        - sizes of APP partitions 
 *
 *
 *************/
struct __packed bcboottoappmsg
{
  uint32_t bcstartmarker;                /* indicates start of structure */
  void *   flashdevdp;                   /* pointer to flash device descriptor */
  uint32_t launchcode;                   /* launch code */
  void *   usbdescp; /* pointer to USB descriptors */
  uint32_t loopback;                     /* App->App messages */
  uint32_t hwconfig;                     /* hardware configuration bits */
  uint32_t flags;                        /* boot -> app messages */
  uint32_t partisz[10];                  /* total size of 5 APP partitions, read from user partition */
  uint32_t bcendmarker;                  /* indicates end of structure */
};

/*************
 *
 * Name:     bccoworkmsg - Coopertive work message structure
 *
 * Purpose:  To provide a structure to share the resoure assigned state .
 *
 * Members:  See inline comments below
 *
 * Note:     1. Both markers must contain BC_VALID_BOOT_MSG_MARKER for the
 *              contents to be considered valid.
 *              Otherwise, the structure's contents are undefined.
 *           2. The total size of this structure is small and must reside in
 *              RAM that is never initialized by boot loader at startup.
 *
 *************/
struct __packed bccoworkmsg
{
  uint32_t bcstartmarker;    /* indicates start of structure */
  uint16_t bcgpioflag;       /* external gpio owner flag. */
  uint8_t  bcuartfun[2];     /* UART1 and UART2 function */
  uint8_t  bcriowner;        /* RI owner */
  uint8_t  bcsleepind;       /* Sleep inidcation function */
  uint8_t  bcresettype;      /* reset type */
  uint32_t bcgpioflag_ext;   /* Extension of External GPIO owner flags (bits 16-47) */
  uint8_t  bcreserved_u8[1]; /* The unused memory for uint 8 */
  uint32_t bcreserved[11];   /* The unused memory */
  uint32_t bcendmarker;      /* indicates end of structure */
};

void sierra_smem_errdump_save_start(void);
void sierra_smem_errdump_save_timestamp(uint32_t time_stamp);
void sierra_smem_errdump_save_errstr(char *errstrp);
void sierra_smem_errdump_save_fmtstr(char *errstrp);
void sierra_smem_errdump_save_frame(void *taskp, struct stackframe *framep);
int sierra_smem_get_download_mode(void);

#endif /* SIERRA_SMEM_H */
