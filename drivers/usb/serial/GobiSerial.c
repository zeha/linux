/*===========================================================================
FILE:
   GobiSerial.c

DESCRIPTION:
   Linux Qualcomm Serial USB driver Implementation

PUBLIC DRIVER FUNCTIONS:
   GobiProbe
   GobiOpen
   GobiClose
   GobiReadBulkCallback (if kernel is less than 2.6.25)
   GobiSerialSuspend
   GobiSerialResume (if kernel is less than 2.6.24)

Copyright (c) 2011, Code Aurora Forum. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Code Aurora Forum nor
      the names of its contributors may be used to endorse or promote
      products derived from this software without specific prior written
      permission.

Alternatively, provided that this notice is retained in full, this software
may be relicensed by the recipient under the terms of the GNU General Public
License version 2 ("GPL") and only version 2, in which case the provisions of
the GPL apply INSTEAD OF those given above.  If the recipient relicenses the
software under the GPL, then the identification text in the MODULE_LICENSE
macro must be changed to reflect "GPLv2" instead of "Dual BSD/GPL".  Once a
recipient changes the license terms to the GPL, subsequent recipients shall
not relicense under alternate licensing terms, including the BSD or dual
BSD/GPL terms.  In addition, the following license statement immediately
below and between the words START and END shall also then apply when this
software is relicensed under the GPL:

START

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License version 2 and only version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

END

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
==========================================================================*/
//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------

#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/slab.h>

//---------------------------------------------------------------------------
// Global variables and definitions
//---------------------------------------------------------------------------

// Version Information
#define DRIVER_VERSION "2013-03-06/SWI_2.9"
#define DRIVER_AUTHOR "Qualcomm Innovation Center"
#define DRIVER_DESC "GobiSerial"

#define NUM_BULK_EPS         1
#define MAX_BULK_EPS         6

#define SET_CONTROL_LINE_STATE_REQUEST_TYPE        0x21
#define SET_CONTROL_LINE_STATE_REQUEST             0x22
#define CONTROL_DTR                     0x01
#define CONTROL_RTS                     0x02


// Debug flag
static int debug;
// flow control flag
static int flow_control = 1;
// allow port open to success even when GPS control message failed
static int ignore_gps_start_error = 1;

// Number of serial interfaces
static int nNumInterfaces;

// Global pointer to usb_serial_generic_close function
// This function is not exported, which is why we have to use a pointer
// instead of just calling it.
   void (* gpClose)( struct usb_serial_port * );

// DBG macro
#define DBG( format, arg... ) \
   if (debug == 1)\
   { \
      printk( KERN_INFO "GobiSerial::%s " format, __FUNCTION__, ## arg ); \
   } \

/*=========================================================================*/
// Function Prototypes
/*=========================================================================*/

// Attach to correct interfaces
static int GobiProbe(
   struct usb_serial * pSerial,
   const struct usb_device_id * pID );

// Start GPS if GPS port, run usb_serial_generic_open
   int GobiOpen(
      struct tty_struct *        pTTY,
      struct usb_serial_port *   pPort );

// Stop GPS if GPS port, run usb_serial_generic_close
   void GobiClose( struct usb_serial_port * );

// Set reset_resume flag
int GobiSerialSuspend(
   struct usb_interface *     pIntf,
   pm_message_t               powerEvent );

/*============================================================================*/
// Blacklisted Interface Lists - used to filter out non serial device interfaces
/*============================================================================*/
#define BLACKLISTED_INTERFACE_LIST_TERMINATOR  (signed char)(-1)
static const signed char qmi_non_serial_interfaces[]  =
   {8, 19, 20, BLACKLISTED_INTERFACE_LIST_TERMINATOR};
static const signed char gobi_non_serial_interfaces[] =
   {0, BLACKLISTED_INTERFACE_LIST_TERMINATOR};

/*=========================================================================*/
// Qualcomm Gobi 3000 VID/PIDs
/*=========================================================================*/
static struct usb_device_id GobiVIDPIDTable[] =
{
   { USB_DEVICE(0x05c6, 0x920c) },   // Gobi 3000 QDL
   { USB_DEVICE(0x05c6, 0x920d) },   // Gobi 3000 Composite
   /* Sierra Wireless QMI VID/PID */
   { USB_DEVICE(0x1199, 0x68A2),
      .driver_info = (kernel_ulong_t)&qmi_non_serial_interfaces
   },
   /* Sierra Wireless G5K Application VID/PID */
   { USB_DEVICE(0x1199, 0x9041),
      .driver_info = (kernel_ulong_t)&qmi_non_serial_interfaces
   },
   /* Sierra Wireless G3K Boot VID/PID */
   { USB_DEVICE(0x1199, 0x9010) },
   /* Sierra Wireless G3K Device Application VID/PID */
   { USB_DEVICE(0x1199, 0x9011),
      .driver_info = (kernel_ulong_t)&gobi_non_serial_interfaces
   },
   /* Sierra Wireless G3K Boot VID/PID */
   { USB_DEVICE(0x1199, 0x9012) },
   /* Sierra Wireless G3K Application VID/PID */
   { USB_DEVICE(0x1199, 0x9013),
      .driver_info = (kernel_ulong_t)&gobi_non_serial_interfaces
   },
   /* Sierra Wireless G3K Boot VID/PID */
   { USB_DEVICE(0x1199, 0x9014) },
   /* Sierra Wireless G3K Application VID/PID */
   { USB_DEVICE(0x1199, 0x9015),
      .driver_info = (kernel_ulong_t)&gobi_non_serial_interfaces
   },
   /* Sierra Wireless G3K Boot VID/PID */
   { USB_DEVICE(0x1199, 0x9018) },
   /* Sierra Wireless G3K Application VID/PID */
   { USB_DEVICE(0x1199, 0x9019),
      .driver_info = (kernel_ulong_t)&gobi_non_serial_interfaces
   },
   /* G3K Boot VID/PID */
   { USB_DEVICE(0x03F0, 0x361D) },
   /* G3K Application VID/PID */
   { USB_DEVICE(0x03F0, 0x371D),
      .driver_info = (kernel_ulong_t)&gobi_non_serial_interfaces
   },
   { }  // Terminating entry
};
MODULE_DEVICE_TABLE( usb, GobiVIDPIDTable );

/* per port private data */
struct sierra_port_private {
   /* Settings for the port */
   int rts_state;    /* Handshaking pins (outputs) */
   int dtr_state;
};

static int Gobi_calc_interface(struct usb_serial *serial)
{
   int interface;
   struct usb_interface *p_interface;
   struct usb_host_interface *p_host_interface;
   dev_dbg(&serial->dev->dev, "%s\n", __func__);

   /* Get the interface structure pointer from the serial struct */
   p_interface = serial->interface;

   /* Get a pointer to the host interface structure */
   p_host_interface = p_interface->cur_altsetting;

   /* read the interface descriptor for this active altsetting
    * to find out the interface number we are on
    */
   interface = p_host_interface->desc.bInterfaceNumber;

   return interface;
}

static int Gobi_send_setup(struct usb_serial_port *port)
{
   struct usb_serial *serial = port->serial;
   struct sierra_port_private *portdata;
   __u16 interface = 0;
   int val = 0;
   int retval;

   dev_dbg(&port->dev, "%s\n", __func__);

   portdata = usb_get_serial_port_data(port);

   if (portdata->dtr_state)
      val |= CONTROL_DTR;
   if (portdata->rts_state)
      val |= CONTROL_RTS;

   /* obtain interface for usb control message below */
   if (serial->num_ports == 1) {
      interface = Gobi_calc_interface(serial);
   }
   else {
      dev_err(&port->dev, 
            "flow control is not supported for %d serial port\n",
            serial->num_ports);
      return -ENODEV;
   }

   retval = usb_autopm_get_interface(serial->interface);
   if (retval < 0)
   {
      return retval;
   }

   retval = usb_control_msg(serial->dev, usb_rcvctrlpipe(serial->dev, 0),
         SET_CONTROL_LINE_STATE_REQUEST,
         SET_CONTROL_LINE_STATE_REQUEST_TYPE,
         val, interface, NULL, 0, USB_CTRL_SET_TIMEOUT);
   usb_autopm_put_interface(serial->interface);

   return retval;
}

static void Gobi_dtr_rts(struct usb_serial_port *port, int on)
{
   struct usb_serial *serial = port->serial;
   struct sierra_port_private *portdata;

   portdata = usb_get_serial_port_data(port);
   portdata->rts_state = on;
   portdata->dtr_state = on;

   /* only send down the usb control message if enabled */
   if (serial->dev && flow_control) {
      mutex_lock(&serial->disc_mutex);
      if (!serial->disconnected)
      {
         Gobi_send_setup(port);
      }
      mutex_unlock(&serial->disc_mutex);
   }
}

static int Gobi_startup(struct usb_serial *serial)
{
   struct usb_serial_port *port = NULL;
   struct sierra_port_private *portdata = NULL;
   int i;

   dev_dbg(&serial->dev->dev, "%s\n", __func__);

   if (serial->num_ports) {
      /* Note: One big piece of memory is allocated for all ports 
       * private data in one shot. This memory is split into equal 
       * pieces for each port. 
       */
      portdata = (struct sierra_port_private *)kzalloc
         (sizeof(*portdata) * serial->num_ports, GFP_KERNEL);
      if (!portdata) {
         dev_dbg(&serial->dev->dev, "%s: No memory!\n", __func__);
         return -ENOMEM;
      }
   }

   /* Now setup per port private data */
   for (i = 0; i < serial->num_ports; i++, portdata++) {
      port = serial->port[i];

      /* Set the port private data pointer */
      usb_set_serial_port_data(port, portdata);
   }

   return 0;
}

static void Gobi_release(struct usb_serial *serial)
{
   int i;
   struct usb_serial_port *port;
   struct sierra_intf_private *intfdata = serial->private;

   dev_dbg(&serial->dev->dev, "%s\n", __func__);

   if (serial->num_ports > 0) {
      port = serial->port[0];
      if (port)
      {
         /* Note: The entire piece of memory that was allocated 
          * in the startup routine can be released by passing
          * a pointer to the beginning of the piece.
          * This address corresponds to the address of the chunk
          * that was given to port 0.
          */
         kfree(usb_get_serial_port_data(port));
      }
   }

   for (i = 0; i < serial->num_ports; ++i) {
      port = serial->port[i];
      if (!port)
      {
         continue;
      }
      usb_set_serial_port_data(port, NULL);
   }
   kfree(intfdata);
}

/*=========================================================================*/
// Struct usb_serial_driver
/*=========================================================================*/
static struct usb_serial_driver gGobiDevice =
{
   .driver =
   {
      .owner     = THIS_MODULE,
      .name      = "GobiSerial driver",
   },
   .description         = "GobiSerial",
   .id_table            = GobiVIDPIDTable,
   .num_ports           = NUM_BULK_EPS,
   .probe               = GobiProbe,
   .open                = GobiOpen,
   .dtr_rts             = Gobi_dtr_rts,
   .attach              = Gobi_startup,
   .release             = Gobi_release,
#ifdef CONFIG_PM
   .suspend    = GobiSerialSuspend,
   .resume     = usb_serial_resume,
#else
   .suspend    = NULL,
   .resume     = NULL,
#endif
};

static struct usb_serial_driver * const serial_drivers[] = {
   &gGobiDevice, NULL
};

//---------------------------------------------------------------------------
// USB serial core overridding Methods
//---------------------------------------------------------------------------

/*===========================================================================
METHOD:
   InterfaceIsBlacklisted (Free Method)

DESCRIPTION:
   Check whether an interface is blacklisted

PARAMETERS:
   ifnum           [ I ] - interface number
   pblklist        [ I ] - black listed interface list

RETURN VALUE:
   bool - true if the interface is blacklisted
          false otherwise
===========================================================================*/
static bool InterfaceIsBlacklisted(
   const signed char ifnum,
   const signed char *pblklist)
{
   if (pblklist != NULL)
   {
      while(*pblklist != BLACKLISTED_INTERFACE_LIST_TERMINATOR)
      {
         if (*pblklist == ifnum)
             return true;
         pblklist ++;
      }
   }
   return false;
}

/*===========================================================================
METHOD:
   GobiProbe (Free Method)

DESCRIPTION:
   Attach to correct interfaces

PARAMETERS:
   pSerial    [ I ] - Serial structure
   pID        [ I ] - VID PID table

RETURN VALUE:
   int - negative error code on failure
         zero on success
===========================================================================*/
static int GobiProbe(
   struct usb_serial * pSerial,
   const struct usb_device_id * pID )
{
   // Assume failure
   int nRetval = -ENODEV;
   int nInterfaceNum;
   struct usb_host_endpoint * pEndpoint;
   int endpointIndex;
   int numEndpoints;

   DBG( "\n" );

   // Test parameters
   if ( (pSerial == NULL)
   ||   (pSerial->dev == NULL)
   ||   (pSerial->dev->actconfig == NULL)
   ||   (pSerial->interface == NULL)
   ||   (pSerial->interface->cur_altsetting == NULL)
   ||   (pSerial->type == NULL) )
   {
      DBG( "invalid parameter\n" );
      return -EINVAL;
   }

   nNumInterfaces = pSerial->dev->actconfig->desc.bNumInterfaces;
   DBG( "Num Interfaces = %d\n", nNumInterfaces );
   nInterfaceNum = pSerial->interface->cur_altsetting->desc.bInterfaceNumber;
   DBG( "This Interface = %d\n", nInterfaceNum );

   if (nNumInterfaces == 1)
   {
      // QDL mode?
      if ((nInterfaceNum == 0) || (nInterfaceNum == 1))
      {
         DBG( "QDL port found\n" );
         nRetval = usb_set_interface( pSerial->dev,
                                      nInterfaceNum,
                                      0 );
         if (nRetval < 0)
         {
            DBG( "Could not set interface, error %d\n", nRetval );
         }
      }
      else
      {
         DBG( "Incorrect QDL interface number\n" );
      }
   }
   else if (nNumInterfaces > 1)
   {
      /* Composite mode */
      if( InterfaceIsBlacklisted((signed char)nInterfaceNum,
          (const signed char *)pID->driver_info) )
      {
         DBG( "Ignoring blacklisted interface #%d\n", nInterfaceNum );
         return -ENODEV;
      }
      else
      {
         nRetval = usb_set_interface( pSerial->dev,
                                      nInterfaceNum,
                                      0 );
         if (nRetval < 0)
         {
            DBG( "Could not set interface, error %d\n", nRetval );
         }

         // Check for recursion
         if (pSerial->type->close != GobiClose)
         {
            // Store usb_serial_generic_close in gpClose
            gpClose = pSerial->type->close;
            pSerial->type->close = GobiClose;
         }
      }
   }
   if (nRetval == 0 && nNumInterfaces > 1 )
   {
      // Clearing endpoint halt is a magic handshake that brings
      // the device out of low power (airplane) mode
      // NOTE: FCC verification should be done before this, if required
      numEndpoints = pSerial->interface->cur_altsetting
                         ->desc.bInterfaceNumber;

      for (endpointIndex = 0; endpointIndex < numEndpoints; endpointIndex++)
      {
         pEndpoint = pSerial->interface->cur_altsetting->endpoint
                   + endpointIndex;

         if (pEndpoint != NULL
         &&  usb_endpoint_dir_out( &pEndpoint->desc ) == true)
         {
            int pipe = usb_sndbulkpipe( pSerial->dev,
                                        pEndpoint->desc.bEndpointAddress );
            nRetval = usb_clear_halt( pSerial->dev, pipe );

            // Should only be one
            break;
         }
      }
   }

   return nRetval;
}

/*===========================================================================
METHOD:
   IsGPSPort (Free Method)

DESCRIPTION:
   Determines whether the interface is GPS port

PARAMETERS:
   pPort   [ I ] - USB serial port structure

RETURN VALUE:
   bool- true if this is a GPS port
       - false otherwise
===========================================================================*/
bool IsGPSPort(struct usb_serial_port *   pPort )
{
   DBG( "Product=0x%x, Interface=0x%x\n",
        cpu_to_le16(pPort->serial->dev->descriptor.idProduct),
        pPort->serial->interface->cur_altsetting->desc.bInterfaceNumber);

   switch (cpu_to_le16(pPort->serial->dev->descriptor.idProduct))
   {
      case 0x68A2:  /* Sierra Wireless QMI */
         if (pPort->serial->interface->cur_altsetting->desc.bInterfaceNumber == 2)
            return true;
         break;

      case 0x9011:  /* Sierra Wireless G3K */
      case 0x9013:  /* Sierra Wireless G3K */
      case 0x9015:  /* Sierra Wireless G3K */
      case 0x9019:  /* Sierra Wireless G3K */
      case 0x371D:  /* G3K */
         if (pPort->serial->interface->cur_altsetting->desc.bInterfaceNumber == 3)
            return true;
         break;

      default:
         return false;
         break;
  }
  return false;
}

/*===========================================================================
METHOD:
   GobiOpen (Free Method)

DESCRIPTION:
   Start GPS if GPS port, run usb_serial_generic_open

PARAMETERS:
   pTTY    [ I ] - TTY structure (only on kernels <= 2.6.26)
   pPort   [ I ] - USB serial port structure
   pFilp   [ I ] - File structure (only on kernels <= 2.6.31)

RETURN VALUE:
   int - zero for success
       - negative errno on error
===========================================================================*/
int GobiOpen(
   struct tty_struct *        pTTY,
   struct usb_serial_port *   pPort )
{
   const char startMessage[] = "$GPS_START";
   int nResult;
   int bytesWrote;

   DBG( "\n" );

   // Test parameters
   if ( (pPort == NULL)
   ||   (pPort->serial == NULL)
   ||   (pPort->serial->dev == NULL)
   ||   (pPort->serial->interface == NULL)
   ||   (pPort->serial->interface->cur_altsetting == NULL) )
   {
      DBG( "invalid parameter\n" );
      return -EINVAL;
   }

   // Is this the GPS port?
   if ((IsGPSPort(pPort)) == true)
   {
      // Send startMessage, 1s timeout
      nResult = usb_bulk_msg( pPort->serial->dev,
                              usb_sndbulkpipe( pPort->serial->dev,
                                               pPort->bulk_out_endpointAddress ),
                              (void *)&startMessage[0],
                              sizeof( startMessage ),
                              &bytesWrote,
                              1000 );
      if (nResult != 0)
      {
         DBG( "error %d sending startMessage\n", nResult );
         if (!ignore_gps_start_error)
         {
            return nResult;
         }
      }
      if (bytesWrote != sizeof( startMessage ))
      {
         DBG( "invalid write size %d, %u\n",
              bytesWrote,
              sizeof( startMessage ) );
         if (!ignore_gps_start_error)
         {
            return -EIO;
         }
      }
   }

   // Clear endpoint halt condition
   if( nNumInterfaces > 1 )
   {
      nResult = usb_clear_halt(pPort->serial->dev,
                               usb_sndbulkpipe(pPort->serial->dev,
                               pPort->bulk_in_endpointAddress) | USB_DIR_IN );
      if (nResult != 0)
      {
         DBG( "usb_clear_halt return value = %d\n", nResult );
      }
   }

   // Pass to usb_serial_generic_open
   return usb_serial_generic_open( pTTY, pPort );
}

/*===========================================================================
METHOD:
   GobiClose (Free Method)

DESCRIPTION:
   Stop GPS if GPS port, run usb_serial_generic_close

PARAMETERS:
   pTTY    [ I ] - TTY structure (only if kernel > 2.6.26 and <= 2.6.29)
   pPort   [ I ] - USB serial port structure
   pFilp   [ I ] - File structure (only on kernel <= 2.6.30)
===========================================================================*/
void GobiClose( struct usb_serial_port * pPort )
{
   const char stopMessage[] = "$GPS_STOP";
   int nResult;
   int bytesWrote;

   DBG( "\n" );

   // Test parameters
   if ( (pPort == NULL)
   ||   (pPort->serial == NULL)
   ||   (pPort->serial->dev == NULL)
   ||   (pPort->serial->interface == NULL)
   ||   (pPort->serial->interface->cur_altsetting == NULL) )
   {
      DBG( "invalid parameter\n" );
      return;
   }

   // Is this the GPS port?
   if ((IsGPSPort(pPort)) == true)
   {
      // Send stopMessage, 1s timeout
      nResult = usb_bulk_msg( pPort->serial->dev,
                              usb_sndbulkpipe( pPort->serial->dev,
                                               pPort->bulk_out_endpointAddress ),
                              (void *)&stopMessage[0],
                              sizeof( stopMessage ),
                              &bytesWrote,
                              1000 );
      if (nResult != 0)
      {
         DBG( "error %d sending stopMessage\n", nResult );
      }
      if (bytesWrote != sizeof( stopMessage ))
      {
         DBG( "invalid write size %d, %u\n",
              bytesWrote,
              sizeof( stopMessage ) );
      }
   }

   // Pass to usb_serial_generic_close
   if (gpClose == NULL)
   {
      DBG( "NULL gpClose\n" );
      return;
   }

   gpClose( pPort );
}

#ifdef CONFIG_PM
/*===========================================================================
METHOD:
   GobiSerialSuspend (Public Method)

DESCRIPTION:
   Set reset_resume flag

PARAMETERS
   pIntf          [ I ] - Pointer to interface
   powerEvent     [ I ] - Power management event

RETURN VALUE:
   int - 0 for success
         negative errno for failure
===========================================================================*/
int GobiSerialSuspend(
   struct usb_interface *     pIntf,
   pm_message_t               powerEvent )
{
   struct usb_serial * pDev;

   if (pIntf == 0)
   {
      return -ENOMEM;
   }

   pDev = usb_get_intfdata( pIntf );
   if (pDev == NULL)
   {
      return -ENXIO;
   }

   // Unless this is PM_EVENT_SUSPEND, make sure device gets rescanned
   if ((powerEvent.event & PM_EVENT_SUSPEND) == 0)
   {
      pDev->dev->reset_resume = 1;
   }

   // Run usb_serial's suspend function
   return usb_serial_suspend( pIntf, powerEvent );
}
#endif /* CONFIG_PM*/

module_usb_serial_driver(serial_drivers, GobiVIDPIDTable);

MODULE_VERSION( DRIVER_VERSION );
MODULE_AUTHOR( DRIVER_AUTHOR );
MODULE_DESCRIPTION( DRIVER_DESC );
MODULE_LICENSE( "Dual BSD/GPL" );

module_param(debug, int, S_IRUGO | S_IWUSR );
MODULE_PARM_DESC(debug, "Debug enabled or not" );
module_param(flow_control, int, S_IRUGO | S_IWUSR );
MODULE_PARM_DESC(flow_control, "flow control enabled or not" );
module_param(ignore_gps_start_error, int, S_IRUGO | S_IWUSR );
MODULE_PARM_DESC(ignore_gps_start_error, 
   "allow port open to success even when GPS control message failed");
