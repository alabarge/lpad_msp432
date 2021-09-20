/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      FTDI BIT-BANG D2XX API Dynamic Link Library

   1.2 Functional Description

      This module provides an API to access the BIT-BANG D2XX Driver.

   1.3 Specification/Design Reference

      BITAPI_DLL.docx

   1.4 Module Test Specification Reference

      None

   1.5 Compilation Information

      Microsoft Visual Studio 2012 C/C++ using multithreaded CLIB.

   1.6 Notes

      NONE

   2  CONTENTS

      1 ABSTRACT
        1.1 Module Type
        1.2 Functional Description
        1.3 Specification/Design Reference
        1.4 Module Test Specification Reference
        1.5 Compilation Information
        1.6 Notes

      2 CONTENTS

      3 VOCABULARY

      4 EXTERNAL RESOURCES
        4.1  Include Files
        4.2  External Data Structures
        4.3  External Function Prototypes

      5 LOCAL CONSTANTS AND MACROS

      6 MODULE DATA STRUCTURES
        6.1  Local Function Prototypes
        6.2  Local Data Structures

      7 MODULE CODE
      7.1   bit_query()
      7.2   bit_init()
      7.3   bit_tx()
      7.4   bit_timer_callback()
      7.5   bit_final()

-----------------------------------------------------------------------------*/

// 3 VOCABULARY

// 4 EXTERNAL RESOURCES

// 4.1  Include Files

#include "stdafx.h"
#include "stdio.h"
#include "stdlib.h"
#include "atlbase.h"
#include "atlstr.h"

#include "cm_const.h"
#include "cmapi.h"
#include "daq_msg.h"
#include "bit.h"
#include "bitapi.h"
#include "ftd2xx.h"
#include "timer.h"
#include "trace.h"

#include "build.h"

// 4.2   External Data Structures

// 4.3 External Function Prototypes

// 5 LOCAL CONSTANTS AND MACROS

// 6 MODULE DATA STRUCTURES

// 6.1  Local Function Prototypes

static VOID CALLBACK bit_timer_callback(PVOID lpParam, BOOLEAN TimerOrWaitFired);

// 6.2  Local Data Structures

   static   FT_DEVICE_LIST_INFO_NODE m_devinfo[BIT_MAX_DEVICES] = {0};

   static   CHRTimer          m_timer;
   static   uint32_t          m_devcnt  = 0;
   static   uint8_t           m_dbus[1] = {0};
   static   uint8_t           m_com_port = 0;
   static   HANDLE            m_qtimer;
   static   HANDLE            m_htimer;
   static   CRITICAL_SECTION  m_mutex;


   static   HANDLE            m_bit = NULL;

// 7 MODULE CODE

// ===========================================================================

// 7.1

BIT_API uint32_t bit_query(bit_dev_info_t **devinfo) {

/* 7.1.1   Functional Description

   This routine will query the FTD2XX driver to acquire all the BIT-BANG
   devices attached to the system.

   7.1.2   Parameters:

   devcnt   Pointer to the attached device count
   devinfo  Pointer to the pbit_dev_info_t strucure

   7.1.3   Return Values:

   devcnt  FTDI Device Count

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   FT_STATUS   status;
   DWORD       devices;

// 7.1.5   Code

   // Query for attached devices
   status = FT_CreateDeviceInfoList(&devices);
   if (status == FT_OK) m_devcnt = (uint32_t)devices;

   // Query for Device Info
   if (m_devcnt <= BIT_MAX_DEVICES) {
      status = FT_GetDeviceInfoList((FT_DEVICE_LIST_INFO_NODE *)m_devinfo, &devices);
      if (status == FT_OK) m_devcnt = (uint32_t)devices;
   }

   *devinfo = (bit_dev_info_t *)m_devinfo;

   return m_devcnt;

}  // end bit_query()


// ===========================================================================

// 7.2

BIT_API int32_t bit_init(uint8_t com_port) {

/* 7.2.1   Functional Description

   This routine will open access to the BIT device (VID=0x0403,PID=0x6001).

   NOTE: Only a single device is currently supported.

   7.2.2   Parameters:

   com_port FTDI Port

   7.2.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

   uint32_t    result = BIT_OK;
   FT_STATUS   status;
   DWORD       sent;
   uint32_t    i;

// 7.2.5   Code

   // close BIT if opened
   if (m_bit != NULL) FT_Close(m_bit);

   // Init the Mutex
   InitializeCriticalSection (&m_mutex);

   // record com_port
   m_com_port = com_port;

   // Okay to Go
   if (m_devcnt != 0) {
      // check for valid BIT interface
      for (i=0;i<m_devcnt;i++) {
         if (m_devinfo[i].SerialNumber[0] == 'O' && m_devinfo[i].SerialNumber[1] == '4') {
            // Open Selected Available port
            if (m_com_port == i) {
               status = FT_OpenEx((PVOID)m_devinfo[i].SerialNumber, FT_OPEN_BY_SERIAL_NUMBER, &m_bit);
               // Configure Device characteristics
               if (status == FT_OK) {
                  status |= FT_ResetDevice(m_bit);
                  status |= FT_Purge(m_bit, FT_PURGE_RX | FT_PURGE_TX);
                  status |= FT_ResetDevice(m_bit);
                  status |= FT_SetUSBParameters(m_bit, 32768, 32768);
                  status |= FT_SetChars(m_bit, FALSE, 0, FALSE, 0);
                  status |= FT_SetLatencyTimer(m_bit, BIT_LATENCY);
                  status |= FT_SetTimeouts(m_bit, BIT_RX_TIMEOUT, BIT_TX_TIMEOUT);
                  // 1Mbps baudrate, determines data strobe width, not used here
                  status |= FT_SetBaudRate(m_bit, 1000000);
                  status |= FT_SetFlowControl(m_bit, FT_FLOW_NONE, 0, 0);
                  status |= FT_SetDataCharacteristics(m_bit, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
                  // set async bit-bang mode, reset device first
                  status |= FT_SetBitMode(m_bit, 0x00, 0x00);
                  // all data bus is outputs
                  status |= FT_SetBitMode(m_bit, 0xFF, 0x01);
                  // Check return status from BIT
                  if (status != FT_OK) result |= BIT_ERR_OPEN;
               }
               break;
            }
         }
      }
      if (m_devcnt == i) result = BIT_ERR_DEV_CNT;
   }

   // Device Opened
   if (result == BIT_OK && m_bit != NULL) {

      // Empty the TX and RX Queues
      status |= FT_Purge(m_bit, FT_PURGE_RX | FT_PURGE_TX);

      // clear the data bus
      status |= FT_Write(m_bit, m_dbus, 1, &sent);


      // Start Periodic Timer, every 250 milliseconds
      m_qtimer = CreateTimerQueue();
      CreateTimerQueueTimer(&m_htimer, m_qtimer, bit_timer_callback,
            NULL, 250, 250, 0);

      // Reset Hi-Res Timer
      m_timer.Start();

   }
   else {
      FT_Close(m_bit);
   }

   return result;

}  // end bit_init()


// ===========================================================================

// 7.3

BIT_API void bit_tx(uint8_t dbus, uint8_t state) {

/* 7.3.1   Functional Description

   This routine will transmit the data bus value.

   7.3.2   Parameters:

   dbus    Data Bus Value to set
   state   Value

   7.3.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.3.4   Data Structures

   DWORD    sent;

// 7.3.5   Code

   // verify connection
   if (m_bit == NULL) return;

   // Enter Critical Section
   EnterCriticalSection(&m_mutex);

   // ON, OFF or WRITE
   switch (state) {
      case BIT_ON :
         m_dbus[0] = m_dbus[0] | dbus;
         break;
      case BIT_OFF :
         m_dbus[0] = m_dbus[0] & ~dbus;
         break;
      case BIT_WR :
         m_dbus[0] = dbus;
         break;
   }

   FT_Write(m_bit, m_dbus, 1, &sent);

   // Leave Critical Section
   LeaveCriticalSection(&m_mutex);

} // end bit_tx()


// ===========================================================================

// 7.4

VOID CALLBACK bit_timer_callback(PVOID lpParam, BOOLEAN TimerOrWaitFired) {

/* 7.4.1   Functional Description

   This is the timer callback function for handling periodic events.

   NOTE: Timing intervals are not precise for this callback.

   7.4.2   Parameters:

   lpParam
   TimerOrWaitFired

   7.4.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.4.4   Data Structures

   DWORD    sent;

// 7.4.5   Code

   // Enter Critical Section
   EnterCriticalSection(&m_mutex);

   m_dbus[0] = (m_dbus[0] & BIT_TP0) ? m_dbus[0] & ~BIT_TP0 : m_dbus[0] | BIT_TP0;

   FT_Write(m_bit, m_dbus, 1, &sent);

   // Leave Critical Section
   LeaveCriticalSection(&m_mutex);

} // end bit_timer_callback()


// ===========================================================================

// 7.5

BIT_API void bit_final(void) {

/* 7.5.1   Functional Description

   This routine will clean-up any allocated resources.

   7.5.2   Parameters:

   NONE

   7.5.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.5.4   Data Structures

   DWORD    sent;

// 7.5.5   Code

   // reset dbus
   m_dbus[0] = 0x00;
   FT_Write(m_bit, m_dbus, 1, &sent);
   FT_SetBitMode(m_bit, 0x00, 0x00);

   // Close BIT
   FT_Close(m_bit);
   m_bit = NULL;

   // Delete Timer Queue
   DeleteTimerQueue(m_qtimer);

} // end bit_final()

