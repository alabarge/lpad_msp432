/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      DATA ACQUISITION (DAQ-I) APPLICATION

   1.2 Functional Description

      This module is responsible for implementing the main embedded
      application.

   1.3 Specification/Design Reference

      See fw_cfg.h under the share directory.

   1.4 Module Test Specification Reference

      None

   1.5 Compilation Information

      See fw_cfg.h under the share directory.

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
        7.1  main()
        7.2  SysTick_Handler()

-----------------------------------------------------------------------------*/

// 3 VOCABULARY

// 4 EXTERNAL RESOURCES

// 4.1  Include Files

#include "main.h"

// message string table
#include "msg_str.h"

// 4.2   External Data Structures

   // global control
   gc_t     gc;

   // configurable items
   ci_t     ci;

   // month table for date-time strings
   char  *month_table[] = {
            "JAN", "FEB", "MAR", "APR",
            "MAY", "JUN", "JUL", "AUG",
            "SEP", "OCT", "NOV", "DEC"
          };

// 4.3   External Function Prototypes

// 5 LOCAL CONSTANTS AND MACROS

// 6 MODULE DATA STRUCTURES

// 6.1  Local Function Prototypes

// 6.2  Local Data Structures

   // Heart Beat
   static   uint8_t  hb_led[] = {0, 0, 1, 1, 0, 0, 1, 1,
                                 1, 1, 1, 1, 1, 1, 1, 1};

   static   uint8_t  hb_cnt   =  0;
   static   uint8_t  led_cnt  =  0;

   static   char     clr_scrn[] = {0x1B, '[', '2', 'J', 0x00};
   static   char     cur_home[] = {0x1B, '[', 'H', 0x00};

// 7 MODULE CODE

// ===========================================================================

// 7.1

int main() {

/* 7.1.1   Functional Description

   This is the main entry point for the embedded application, it is called
   by the alt_main() function from HAL.

   7.1.2   Parameters:

   NONE

   7.1.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

// 7.1.5   Code

   // halt WDT
   MAP_WDT_A_holdTimer();

   // disable master interrupts
   MAP_Interrupt_disableMaster();

   // set the core voltage level to VCORE1
   MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);

   // set 2 flash wait states for Flash bank 0 and 1
   MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
   MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);

   // init clock system
   MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
   MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
   MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
   MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
   MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

   // set SystemCoreClock
   SystemCoreClockUpdate();

   // init SysTick module
   MAP_SysTick_enableModule();
   MAP_SysTick_setPeriod(SystemCoreClock / CFG_SYS_TICK);
   MAP_SysTick_enableInterrupt();

   // init Timer32 as free running, COUNTS DOWN from 2^32
   // used by clk_sleep() and clk_time()
   MAP_Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1,
                          TIMER32_32BIT,  TIMER32_FREE_RUN_MODE);
   MAP_Timer32_setCount(TIMER32_0_BASE, 4294967295);
   MAP_Timer32_startTimer(TIMER32_0_BASE, false);

   // configure watchdog timer, 1 second at 32K
   MAP_SysCtl_setWDTTimeoutResetType(SYSCTL_SOFT_RESET);
   MAP_WDT_A_initWatchdogTimer(WDT_A_CLOCKSOURCE_ACLK, WDT_A_CLOCKITERATIONS_32K);

   // Open Debug Port
   xlprint_open(EUSCI_A0_BASE, INT_EUSCIA0);

   // Clear the Terminal Screen and Home the Cursor
   xlprint(clr_scrn);
   xlprint(cur_home);

   // Display the Startup Banner
   xlprint("\nDAQ-I MSP432, %s\n\n", BUILD_HI);

   xlprint("__TI_base__:  %08X\n", &__TI_static_base__);
   xlprint("__stack:      %08X\n", &__stack);
   xlprint("__STACK_END:  %08X\n", &__STACK_END);
   xlprint("__STACK_SIZE: %08X\n\n", &__STACK_SIZE);

   // Clear GC
   memset(&gc, 0, sizeof(gc_t));

   // Initialize GC
   gc.feature   = 0;
   gc.trace     = 0;
   gc.debug     = 0;
   gc.status    = CFG_STATUS_INIT;
   gc.error     = CFG_ERROR_CLEAR;
   gc.devid     = CM_DEV_DE0;
   gc.winid     = CM_DEV_WIN;
   gc.com_port  = CM_PORT_COM0;
   gc.int_flag  = FALSE;
   gc.sw_reset  = FALSE;
   gc.sys_time  = 0;
   gc.ping_time = MAP_Timer32_getValue(TIMER32_0_BASE);
   gc.ping_cnt  = 0;
   gc.led_cycle = CFG_LED_CYCLE;
   gc.month     = month_table;
   gc.msg_table = msg_table;
   gc.msg_table_len = DIM(msg_table);
   gc.reset     = MAP_ResetCtl_getSoftResetSource();

   sprintf(gc.dev_str, "DAQ-I MSP432, %s", BUILD_STR);

   // Reset Source
   xlprint("reset source: %08X\n", gc.reset);

   // Initialize the Configurable Items DataBase
   // and read the stored CIs
   gc.error |= ci_init();
   gc.error |= ci_read();

   //
   // INIT THE HARDWARE
   //

   // GPIO Init
   gc.error |= gpio_init();

   // CM Init
   gc.error |= cm_init();

   // UART Init
   gc.error |= uart_init(EUSCI_A2_BASE, INT_EUSCIA2, gc.com_port);

   // ADC Init
   gc.error |= adc_init();

   // Report Push Button 0-1 setting
   gc.key = gpio_key();
   xlprint("keys:  %02X\n", gc.key);

   // Report CPU
   xlprint("cpu: %s\n", "msp432p401r");

   // Report CPU Frequency
   xlprint("cpu.freq: %d.%d MHz\n", SystemCoreClock / 1000000,
           SystemCoreClock % 1000000);

   // Report all clocks
   xlprint("aclk:   %d Hz\n", CS_getACLK());
   xlprint("bclk:   %d Hz\n", CS_getBCLK());
   xlprint("mclk:   %d Hz\n", CS_getMCLK());
   xlprint("smclk:  %d Hz\n", CS_getSMCLK());
   xlprint("hsmclk: %d Hz\n", CS_getHSMCLK());

   // Partial FLASH Dump, (0x0000.0000)
   xlprint("\nflash partial ...\n\n");
   dump((uint8_t *)FLASH_BASE, 64, LIB_ADDR | LIB_ASCII, 0);

   // Partial SRAM Dump, (0x2000.0000)
   xlprint("\nsram partial ...\n\n");
   dump((uint8_t *)SRAM_BASE, 64, LIB_ADDR | LIB_ASCII, 0);

   // Print Status and Error Results to Serial Port
   if (gc.trace & CFG_TRACE_POST) {
      xlprint("trace   :  %08X\n", gc.trace);
      xlprint("feature :  %08X\n", gc.feature);
      xlprint("status  :  %08X\n", gc.status);
      xlprint("error   :  %08X\n", gc.error);
   }

   //
   // START THE SERVICES
   //

   // Control Panel (CP)
   gc.error |= cp_hal_init();
   gc.error |= cp_init();

   // DAQ Controller (DAQ)
   gc.error |= daq_hal_init();
   gc.error |= daq_init();

   // All LEDs Off
   gpio_set_val(0, GPIO_LED_ALL_OFF);

   // Initialization Finished so
   // start Running
   gc.status &= ~CFG_STATUS_INIT;
   gc.status |=  CFG_STATUS_RUN;

   // Enable master interrupts
   MAP_Interrupt_enableMaster();

   // Init the Command Line Interpreter
   cli_gen();

   // start the watchdog timer
   MAP_WDT_A_startTimer();

   //
   // BACKGROUND PROCESSING
   //
   // NOTE: All Background thread operations begin
   //       from this for-loop! Further, all foreground
   //       processing not done in the interrupt must
   //       start through this for-loop!
   //
   for (;;) {
      //
      // CM THREAD
      //
      cm_thread();
      //
      // CP THREAD
      //
      cp_thread();
      //
      // DAQ THREAD
      //
      daq_thread();
      //
      // CLI THREAD
      //
      cli_process(&gc.cli);
      //
      // WATCHDOG
      //
      MAP_WDT_A_clearTimer();
   }

} // end main()

// ===========================================================================

// 7.2

void SysTick_Handler(void) {

/* 7.2.1   Functional Description

   This is the main system timer callback function for handling background
   periodic events.

   7.2.2   Parameters:

   NONE

   7.2.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

   uint8_t     key;
   uint32_t    i;

// 7.2.5   Code

   // System Time Tick
   gc.sys_time++;

   // Period Service Ticks
   cp_tick();
   daq_tick();
   cm_tick();

   // Activity Indicator
   if (++led_cnt >= gc.led_cycle) {
      led_cnt = 0;
      // Heart Beat
      gpio_set_val(GPIO_LED_HB, hb_led[(hb_cnt++ & 0xF)]);
      // COM Indicator Off
      gpio_set_val(GPIO_LED_COM, GPIO_LED_OFF);
      // PIPE Indicator Off
      gpio_set_val(GPIO_LED_PIPE, GPIO_LED_OFF);
      // Set Fault LED for Errors
      gpio_set_val(GPIO_LED_ERR, gc.error ? GPIO_LED_ON : GPIO_LED_OFF);
      // Set Fault LED when Running
      gpio_set_val(GPIO_LED_ERR, (gc.status & CFG_STATUS_DAQ_RUN) ? GPIO_LED_ON : GPIO_LED_OFF);
   }

   // Read the User Pushbutton Switches, de-bounced by timer
   key = gc.key ^ gpio_key();
   gc.key = gpio_key();
   if (key != 0) {
      for (i=0;i<4;i++) {
         switch (key & (1 << i)) {
         case GPIO_KEY_0 :
            // PB Down
            if ((gc.key & GPIO_KEY_0) == 0) {
               xlprint("key0 pressed\n");
            }
            break;
         case GPIO_KEY_1 :
            // PB Down
            if ((gc.key & GPIO_KEY_1) == 0) {
               xlprint("key1 pressed\n");
            }
            break;
         }
      }
   }

} // end timer()
