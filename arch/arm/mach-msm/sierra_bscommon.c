/* arch/arm/mach-msm/sierra_bscommon.c
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

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>


#include <mach/sierra_smem.h>
#include <mach/sierra_bsidefs.h>
#include <linux/sierra_bsudefs.h>

/* RAM Copies of HW type, rev, etc. */
/* note that there is a copy for the bootloader and another for the app */
bool bshwconfigread = false;
union bshwconfig bscfg;


/* Local structures and functions */
/************
 *
 * Name:     bsreadhwconfig
 *
 * Purpose:  To get the hardware configuration from gpio
 *
 * Parms:    none
 *
 * Return:   uint32 bitmask of hardware configuration
 *
 * Abort:    none
 *
 * Notes:
 *
 ************/
static ssize_t bsreadhwconfig(void)
{
  struct bcboottoappmsg *mp = (struct bcboottoappmsg *)BS_BOOT_APP_MSG_START;
  return mp->hwconfig;
}

/************
 *
 * Name:     bsreadboottoappflag
 *
 * Purpose:  To get the boot to app flags from SMD
 *
 * Parms:    none
 *
 * Return:   uint32 bitmask of flags 
 *
 * Abort:    none
 *
 * Notes:
 *
 ************/
uint32_t bsreadboottoappflag(void)
{
  struct bcboottoappmsg *mp = (struct bcboottoappmsg *)BS_BOOT_APP_MSG_START;
  return mp->flags;
}
EXPORT_SYMBOL(bsreadboottoappflag);

/************
 *
 * Name:     bsgethwtype
 *
 * Purpose:  Returns hardware type read from QFPROM /GPIO
 *
 * Parms:    none
 *
 * Return:   hardware type
 *
 * Abort:    none
 *
 * Notes:
 *
 ************/
enum bshwtype bsgethwtype(
  void)
{
  if (bshwconfigread == false)
  {
    bscfg.all = bsreadhwconfig();
    bshwconfigread = true;
  }

  return (enum bshwtype) bscfg.hw.type;
}
EXPORT_SYMBOL(bsgethwtype);

/************
 *
 * Name:     bsgethwrev
 *
 * Purpose:  Returns hardware revision read from QFPROM /GPIO
 *
 * Parms:    none
 *
 * Return:   hardware ID
 *
 * Abort:    none
 *
 * Notes:
 *
 ************/
uint8_t bsgethwrev(
  void)
{
  if (bshwconfigread == false)
  {
    bscfg.all = bsreadhwconfig();
    bshwconfigread = true;
  }

  return bscfg.hw.rev;
}
EXPORT_SYMBOL(bsgethwrev);

/************
 *
 * Name:     bsgetmanufacturingcode
 *
 * Purpose:  Returns the current coverage code
 *
 * Parms:    None
 *
 * Return:   manufacturing code
 *
 * Abort:    None
 *
 * Notes:    Bit states are inverted and lines are not necessarilly
 *           in the same register
 *
 *
 *           Code    MODE2   MODE1   MODE0         MANFMode
 *           -----------------------------------------------------
 *            000    high    high    high    Normal Mode (Default)
 *            110     low     low    high    AT on USB, Diag on UART
 *            111     low     low     low    AT on UART, Diag on USB
 *
 *
 ************/
uint32_t bsgetmanufacturingcode(
  void)
{
  if (bshwconfigread == false)
  {
    bscfg.all = bsreadhwconfig();
    bshwconfigread = true;
  }

  return bscfg.hw.mfgmode;
}
EXPORT_SYMBOL(bsgetmanufacturingcode);

/************
 *
 * Name:     bssupport
 *
 * Purpose:  To check if the hardware supports a particular feature
 *
 * Parms:    feature - feature to check
 *
 * Return:   true if hardware supports this feature
 *           false otherwise
 *
 * Abort:    none
 *
 * Notes:    This function is primarily designed to keep hardware variant
 *           checks to a central location.
 *
 ************/
bool bssupport(
  enum bsfeature feature)
{
  bool supported = false;
  enum bshwtype hwtype;

  hwtype = bsgethwtype();

  switch (feature)
  {
    case BSFEATURE_MINICARD:
      switch (hwtype)
      {
        case BSMC7355:
        case BSEM7355:
        case BSEM7655:
        case BSMC7354:
        case BSMC7305:
        case BSEM7305:
        case BSMC8805:
        case BSEM8805:
        case BSMC7350:
        case BSMC7350L:
        case BSMC7802:
        case BSMC7304:
          supported = true;
          break;

        default:
          supported = false;
          break;
      }
      break;

    case BSFEATURE_MINICARD_M2M:
      switch (hwtype)
      {
        case BSMC7802:
        case BSMC7354:
        case BSMC7350:
        case BSMC7350L:
        case BSMC7330:
        case BSMC7304:
          supported = true;
          break;

        default:
          supported = false;
          break;
      }
      break;

    case BSFEATURE_AR:
      switch (hwtype)
      {
        case BSAR7550:
        case BSAR7552:
        case BSAR7554: 
        case BSAR7556:
        case BSAR7550_LARGER_MEMORY:
        case BSAR7552_LARGER_MEMORY:
        case BSAR7554_LARGER_MEMORY:     
        case BSAR7558_LARGER_MEMORY:
        case BSWP7100_NEW:
        case BSWP7102_NEW:
        case BSWP7104_NEW:             
          supported = true;
          break;
          
        default:
          supported = false;
          break;
      }
      break;

    case BSFEATURE_WP:
      switch (hwtype)
      {
        case BSWP7100:
        case BSWP7102:
        case BSWP7104:
        case BSWP7100_LARGER_MEMORY:
        case BSWP7102_LARGER_MEMORY:
        case BSWP7104_LARGER_MEMORY:          
          supported = true;
          break;

        default:
          supported = false;
          break;
      }
      break;

    case BSFEATURE_W_DISABLE:
      switch (hwtype)
      {
        case BSMC7355:
        case BSEM7355:
        case BSEM7655:
        case BSMC7354:
        case BSMC7305:
        case BSEM7305:
        case BSMC8805:
        case BSEM8805:
        case BSMC7350:
        case BSMC7350L:
        case BSMC7802:
        case BSMC7304:
          supported = true;
          break;

        default:
          supported = false;
          break;
      }
      break;

    case BSFEATURE_SD:
      switch (hwtype)
      {
        case BSWP7100:
        case BSWP7102:
        case BSWP7104:
        case BSWP7100_LARGER_MEMORY:
        case BSWP7102_LARGER_MEMORY:
        case BSWP7104_LARGER_MEMORY:                        
          supported = true;
          break;

        default:
          supported = false;
          break;
      }
      break;

    case BSFEATURE_VOICE:
      switch (hwtype)
      {
        case BSAR7550:      
        case BSAR7552:
        case BSAR7554:   
        case BSAR7556:
        case BSAR7550_LARGER_MEMORY:      
        case BSAR7552_LARGER_MEMORY:
        case BSAR7554_LARGER_MEMORY:      
        case BSAR7558_LARGER_MEMORY:          
        case BSWP7100:
        case BSWP7102:
        case BSWP7104:
        case BSWP7100_LARGER_MEMORY:
        case BSWP7102_LARGER_MEMORY:
        case BSWP7104_LARGER_MEMORY:
        case BSMC7354:
        case BSMC7350:
        case BSMC7350L:
        case BSMC7802:
        case BSMC7304:
        case BSWP7100_NEW:
        case BSWP7102_NEW:
        case BSWP7104_NEW: 
          supported = true;
          break;
          
        default:
          supported = false;
          break;
      }
      break;

    case BSFEATURE_HSUPA:
      switch (hwtype)
      {
        case BSMC7350:
        case BSMC7350L:
          supported = false;
          break;

        default:
          supported = true;
          break;
      }
      break;

    case BSFEATURE_GPIOSAR:
      switch (hwtype)
      {
        case BSMC7355:
        case BSEM7355:
        case BSMC7354:
        case BSMC7305:
        case BSEM7305:
        case BSEM7655:
        case BSMC8805:
        case BSEM8805:
        case BSMC7350:
        case BSMC7802:
        case BSMC7304:
          supported = true;
          break;

        default:
          supported = false;
          break;
      }
      break;

    case BSFEATURE_RMAUTOCONNECT:
      supported = true;
      break;

    case BSFEATURE_UART:
      switch (hwtype)
      {
        case BSAR7550:
        case BSAR7552:
        case BSAR7554:
        case BSAR7556:
        case BSAR7550_LARGER_MEMORY:      
        case BSAR7552_LARGER_MEMORY:
        case BSAR7554_LARGER_MEMORY:   
        case BSAR7558_LARGER_MEMORY:      
        case BSWP7100:
        case BSWP7102:
        case BSWP7104:
        case BSWP7100_LARGER_MEMORY:
        case BSWP7102_LARGER_MEMORY:
        case BSWP7104_LARGER_MEMORY:
        case BSWP7100_NEW:
        case BSWP7102_NEW:
        case BSWP7104_NEW: 
          supported = true;
          break;

        default:
          supported = false;
          break;
      }
      break;

    case BSFEATURE_ANTSEL:
      switch (hwtype)
      {
        case BSMC7355:
        case BSEM7355:
        case BSEM7655:
        case BSMC7354:
        case BSMC7305:
        case BSEM7305:
        case BSMC8805:
        case BSEM8805:
        case BSWP7100:
        case BSWP7102:
        case BSWP7104:
        case BSWP7100_LARGER_MEMORY:
        case BSWP7102_LARGER_MEMORY:
        case BSWP7104_LARGER_MEMORY: 		
        case BSAR7550:
        case BSAR7552:
        case BSAR7554:
        case BSAR7556:
        case BSAR7550_LARGER_MEMORY:
        case BSAR7552_LARGER_MEMORY:
        case BSAR7554_LARGER_MEMORY:    
        case BSAR7558_LARGER_MEMORY:    
        case BSMC7350:
        case BSMC7350L:
        case BSMC7802:
        case BSMC7304:
        case BSWP7100_NEW:
        case BSWP7102_NEW:
        case BSWP7104_NEW:       
          supported = true;
          break;
            
        default:
          supported = false;
          break;
      }
      break;

    case BSFEATURE_INSIM:
      /* Not supported any more in WP710x, so returns false for all devices */
      supported = false;
      break;

    case BSFEATURE_OOBWAKE:
      switch (hwtype)
      {
        case BSMC7355:
        case BSEM7355:
        case BSMC7354:
        case BSMC7305:
        case BSEM7305:
        case BSEM7655:
        case BSMC8805:
        case BSEM8805:
        case BSMC7350:
        case BSMC7350L:
        case BSMC7802:
        case BSMC7304:
          supported = true;
          break;

        default:
          supported = false;
          break;
      }
      break;

    case BSFEATURE_CDMA:
      switch (hwtype)
      {
        case BSMC7355:
        case BSEM7355:
        case BSEM7655:
        case BSMC7354:
        case BSAR7550:
        case BSAR7550_LARGER_MEMORY:        
        case BSAR7558_LARGER_MEMORY:
        case BSWP7100:
        case BSWP7100_LARGER_MEMORY:        
        case BSMC7350:
        case BSWP7100_NEW:
          supported = true;
          break;

        default:
          supported = false;
          break;
      }
      break;

    case BSFEATURE_GSM:
      switch (hwtype)
      {
        case BSMC7350:
        case BSMC7350L:
          supported = false;
          break;

        default:
          supported = true;
          break;
      }
      break;

    case BSFEATURE_WCDMA:
      switch (hwtype)
      {
        case BSMC7350:
        case BSMC7350L:
          supported = false;
          break;

        default:
          supported = true;
          break;
      }
      break;

    case BSFEATURE_LTE:
      switch (hwtype)
      {
        case BSMC8805:
        case BSEM8805:
          supported = false;
          break;

        default:
          supported = true;
          break;
      }
      break;

    case BSFEATURE_TDSCDMA:
       switch (hwtype)
       {
         case BSAR7556:
           supported = true;
           break;
    
         default:
           supported = false;
           break;
       }
       break;

    case BSFEATURE_EM:
      switch (hwtype)
      {
        case BSEM7355:
        case BSEM7305:
        case BSEM7655:
        case BSEM8805:
          supported = true;
          break;

        default:
          supported = false;
          break;
      }
      break;

    case BSFEATURE_GPSSEL:
      switch (hwtype)
      {
        case BSEM7305:
        case BSMC7305:
        case BSEM7330:
        case BSMC7330:
        case BSEM7355:
        case BSMC7355:
        case BSEM7655:
        case BSMC7371:
          supported = true;
          break;

        default:
          supported = false;
          break;
      }
      break;

    case BSFEATURE_SVC_PIN_DLOAD:
      switch (hwtype)
      {
        case BSAR7550:
        case BSAR7552:
        case BSAR7554:
        case BSAR7556:
        case BSAR7550_LARGER_MEMORY:      
        case BSAR7552_LARGER_MEMORY:
        case BSAR7554_LARGER_MEMORY:    
        case BSAR7558_LARGER_MEMORY:     
        case BSWP7100:
        case BSWP7102:
        case BSWP7104:
        case BSWP7100_LARGER_MEMORY:
        case BSWP7102_LARGER_MEMORY:
        case BSWP7104_LARGER_MEMORY:  
        case BSWP7100_NEW:
        case BSWP7102_NEW:
        case BSWP7104_NEW:                 
          supported = true;
          break;

        default:
          supported = false;
          break;
      }
      break;

    case BSFEATURE_BUZZER:
      switch (hwtype)
      {         
        case BSWP7100:
        case BSWP7102:
        case BSWP7104:
        case BSWP7100_LARGER_MEMORY:
        case BSWP7102_LARGER_MEMORY:
        case BSWP7104_LARGER_MEMORY:                  
          supported = true;
          break;
          
        default:
          supported = false;
          break;
      }
      break;

    case BSFEATURE_SIMHOTSWAP:
      switch (hwtype)
      {
        case BSEM7355:
        case BSEM7305:
        case BSEM8805:
        case BSEM7330:
        case BSAR7550:
        case BSAR7552:
        case BSAR7554:
        case BSAR7556:
        case BSAR7550_LARGER_MEMORY:
        case BSAR7552_LARGER_MEMORY:
        case BSAR7554_LARGER_MEMORY:
        case BSAR7558_LARGER_MEMORY:
        case BSWP7100:
        case BSWP7102:
        case BSWP7104:
        case BSWP7100_LARGER_MEMORY:
        case BSWP7102_LARGER_MEMORY:
        case BSWP7104_LARGER_MEMORY:
        case BSWP7100_NEW:
        case BSWP7102_NEW:
        case BSWP7104_NEW:
          supported = true;
          break;

        default:
          supported = false;
          break;
      }
      break;

    default:
      pr_err("Unknown feature %X", (uint32_t)feature);
      break;
  }

  return supported;

}
EXPORT_SYMBOL(bssupport);

/************
 *
 * Name:     bscheckcoworkmsgmsk()
 *
 * Purpose:  Use to check the cooperative mode message mask
 *
 * Parms:    none
 *
 * Return:   true: the start and end mask is valid 
 *           false: the start or end mask is invalid.
 *
 * Abort:    none
 *
 * Notes:
 *
 ************/
bool bscheckcoworkmsgmsk(void)
{
  volatile struct bccoworkmsg *mp = (volatile struct bccoworkmsg *)BS_COWORK_MSG_START; 
  
  if ((mp->bcstartmarker == BC_VALID_COWORK_MSG_MARKER) &&
      (mp->bcendmarker == BC_VALID_COWORK_MSG_MARKER))
  {
    return true;
  }
  else
  {
    pr_err("Cooperative mode message marker is invalid.");
    return false;
  }
}

/************
 *
 * Name:     bsgetgpioflag()
 *
 * Purpose:  Returns the external gpio owner flag
 *
 * Parms:    none
 *
 * Return:   Extern GPIO owner flag
 *
 * Abort:    none
 *
 * Notes:
 *
 ************/
uint16_t bsgetgpioflag(void)
{
  volatile struct bccoworkmsg *mp = (volatile struct bccoworkmsg *)BS_COWORK_MSG_START;

  if (bscheckcoworkmsgmsk() == true)
  {
    return mp->bcgpioflag;
  }
  else
  {
    pr_err("Cooperative mode message read procedure failed.");
    return 0;
  }
}
EXPORT_SYMBOL(bsgetgpioflag);

/************
 *
 * Name:     bsgetuartfun()
 *
 * Purpose:  Provide to get UARTs function seting
 *
 * Parms:    uart Number
 *
 * Return:   UART function
 *
 * Abort:    none
 *
 * Notes:
 *
 ************/
int8_t bsgetuartfun(uint uart_num )
{
  volatile struct bccoworkmsg *mp = (volatile struct bccoworkmsg *)BS_COWORK_MSG_START;
   
  if (uart_num > 1) 
  {
    return -1;
  }

  if (bscheckcoworkmsgmsk() == true)
  {
    return (int8_t)mp->bcuartfun[uart_num];
  }
  else
  {
    pr_err("Cooperative mode message read procedure failed.");
    return -1;
  }
}
EXPORT_SYMBOL(bsgetuartfun);
