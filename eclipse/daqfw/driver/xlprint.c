/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      STDIO Interface

   1.2 Functional Description

      The xlprint routine is contained in this module.
      The stdio low level character routines are implemented in this module also.

   1.3 Specification/Design Reference

      See fw_cfg.h under the share directory.

   1.4 Module Test Specification Reference

      None

   1.5 Compilation Information

      See fwCfg.h under the share directory.

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
      7.1  xlprint()
      7.2  xlprint_open()
      7.3  EUSCIA0_IRQHandler()

-----------------------------------------------------------------------------*/

// 3 VOCABULARY

// 4 EXTERNAL RESOURCES

// 4.1  Include Files

#include <stdarg.h>

#include "main.h"

// 4.2   External Data Structures

// 4.3 External Function Prototypes

// 5 LOCAL CONSTANTS AND MACROS

   static const eUSCI_UART_ConfigV1 uart_config = {
           EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source, 48MHz
                                                    // N = 48M / 115200 = 416.667
           26,                                      // UCxBR  = INT(N/16) = 26
           0,                                       // UCxBRF = INT([(N/16) - INT(N/16)] X 16)
                                                    //        = INT((26.0416 - 26) x 16) = 0
           0xD6,                                    // UCxBRS = Table 24.4 given N - INT(N)
                                                    // N = 416.667, INT(N) = 416
                                                    // 416.667 - 416 = .667 (Table Entry 24 = 0xD6)
           EUSCI_A_UART_NO_PARITY,                  // No Parity
           EUSCI_A_UART_LSB_FIRST,                  // LSB First
           EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
           EUSCI_A_UART_MODE,                       // UART mode
           EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION,  // Oversampling
           EUSCI_A_UART_8_BIT_LEN                   // Word Length
   };

// 6 MODULE DATA STRUCTURES

// 6.1  Local Function Prototypes

   static void printchar(char **str, int c);
   static int  prints(char **out, const char *string, int width, int pad);
   static int  printi(char **out, int i, int b, int sg, int width, int pad, int letbase);
   static int  print_uart(char **out, const char *format, va_list args);

   static uint32_t  uart = 0;

// 6.2  Local Data Structures

// 7 MODULE CODE

// ===========================================================================

// 7.1

int xlprint(const char *format, ...) {

/* 7.1.1   Functional Description

   This routine will print formatted characters to the UART.

   7.1.2   Parameters:

   format   xlprint Format string
   ...      variable arguments

   7.1.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   va_list args;

// 7.1.5   Code

   va_start(args, format);
   return print_uart(0, format, args);

} // end xlprint()


// ===========================================================================

// 7.2

int xlprints(char *buf, const char *format, ...) {

/* 7.2.1   Functional Description

   This routine will print formatted characters to the UART.

   7.2.2   Parameters:

   buf      result to buf
   format   xlprint Format string
   ...      variable arguments

   7.2.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

   va_list args;

// 7.2.5   Code

   va_start(args, format);
   return print_uart(&buf, format, args);

} // end xlprints()


/*
   Copyright 2001, 2002 Georges Menie (www.menie.org)
   stdarg version contributed by Christian Ettinger

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

void xlprint_open(uint32_t module, uint8_t ext_int) {

   uint32_t    status;

   // selecting P1.2 and P1.3 in uart mode
   MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
           GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

   // init EUSCI_Ax 115200,8,1,N
   MAP_UART_initModule(module, &uart_config);

   // enable uart module
   MAP_UART_enableModule(module);

   // uart base address
   uart = module;

   // clear any pending interrupts
   status = MAP_UART_getEnabledInterruptStatus(uart);
   MAP_UART_clearInterruptFlag(uart, status);

   /* enable only the receive interrupt */
   MAP_Interrupt_enableInterrupt(ext_int);
   MAP_UART_enableInterrupt(uart, EUSCI_A_UART_RECEIVE_INTERRUPT);
}

static void printchar(char **str, int c) {
   if (str) {
      **str = c;
      ++(*str);
   }
   else if (uart != 0) {
      // Write character to the output buffer, blocking
      MAP_UART_transmitData(uart, c);
   }
}

#define PAD_RIGHT 1
#define PAD_ZERO  2

static int prints(char **out, const char *string, int width, int pad) {
   register int pc = 0, padchar = ' ';

   if (width > 0) {
      register int len = 0;
      register const char *ptr;
      for (ptr = string; *ptr; ++ptr) ++len;
      if (len >= width) width = 0;
      else width -= len;
      if (pad & PAD_ZERO) padchar = '0';
   }
   if (!(pad & PAD_RIGHT)) {
      for ( ; width > 0; --width) {
         printchar (out, padchar);
         ++pc;
      }
   }
   for ( ; *string ; ++string) {
      printchar (out, *string);
      ++pc;
   }
   for ( ; width > 0; --width) {
      printchar (out, padchar);
      ++pc;
   }

   return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, int i, int b, int sg, int width, int pad, int letbase) {
   char print_buf[PRINT_BUF_LEN];
   register char *s;
   register int t, neg = 0, pc = 0;
   register uint32_t u = i;

   if (i == 0) {
      print_buf[0] = '0';
      print_buf[1] = '\0';
      return prints (out, print_buf, width, pad);
   }

   if (sg && b == 10 && i < 0) {
      neg = 1;
      u = -i;
   }

   s = print_buf + PRINT_BUF_LEN-1;
   *s = '\0';

   while (u) {
      t = u % b;
      if( t >= 10 )
         t += letbase - '0' - 10;
      *--s = t + '0';
      u /= b;
   }

   if (neg) {
      if( width && (pad & PAD_ZERO) ) {
         printchar (out, '-');
         ++pc;
         --width;
      }
      else {
         *--s = '-';
      }
   }

   return pc + prints (out, s, width, pad);
}

static int print_uart(char **out, const char *format, va_list args) {
   register int width, pad;
   register int pc = 0;
   char scr[2];

   for (; *format != 0; ++format) {
      if (*format == '%') {
         ++format;
         width = pad = 0;
         if (*format == '\0') break;
         if (*format == '%') goto out;
         if (*format == '-') {
            ++format;
            pad = PAD_RIGHT;
         }
         while (*format == '0') {
            ++format;
            pad |= PAD_ZERO;
         }
         for ( ; *format >= '0' && *format <= '9'; ++format) {
            width *= 10;
            width += *format - '0';
         }
         if( *format == 's' ) {
            register char *s = (char *)va_arg( args, int );
            pc += prints (out, s?s:"(null)", width, pad);
            continue;
         }
         if( *format == 'd' ) {
            pc += printi (out, va_arg( args, int ), 10, 1, width, pad, 'a');
            continue;
         }
         if( *format == 'x' ) {
            pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'a');
            continue;
         }
         if( *format == 'X' ) {
            pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'A');
            continue;
         }
         if( *format == 'u' ) {
            pc += printi (out, va_arg( args, int ), 10, 0, width, pad, 'a');
            continue;
         }
         if( *format == 'c' ) {
            /* char are converted to int then pushed on the stack */
            scr[0] = (char)va_arg( args, int );
            scr[1] = '\0';
            pc += prints (out, scr, width, pad);
            continue;
         }
      }
      else {
      out:
         // CR-LF Fix, 25.MAR.04 [AEL]
         if (*format == '\n') printchar(out, '\r');
         printchar (out, *format);
         ++pc;
      }
   }
   if (out) **out = '\0';
   va_end( args );
   return pc;
}


// ===========================================================================

// 7.3

void EUSCIA0_IRQHandler(void) {

/* 7.3.1   Functional Description

   This routine will service the UART Interrupt.

   7.3.2   Parameters:

   NONE

   7.3.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.3.4   Data Structures

   uint32_t status = MAP_UART_getEnabledInterruptStatus(uart);

   uint8_t  c;

// 7.3.5   Code

   //
   // RX INTERRUPT
   //
   if (status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG) {
      // Clear the Interrupt Status
      MAP_UART_clearInterruptFlag(uart, EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG);
      // Always read the RX Register
      c = MAP_UART_receiveData(uart);
      // printable characters only
      if (isprint(c) || c == 0x0a)
         xlprint("%c", c);
      // send character to CLI
      cli_put(&gc.cli, (char)c);
   }

} // end EUSCIA0_IRQHandler()
