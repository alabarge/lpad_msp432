/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      GPIO and DIP Switch Driver.

   1.2 Functional Description

      The GPIO Interface routines are contained in this module.

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
      7.1   gpio_init()
      7.2   gpio_isr()
      7.3   gpio_val_set()
      7.4   gpio_key()

-----------------------------------------------------------------------------*/

// 3 VOCABULARY

// 4 EXTERNAL RESOURCES

// 4.1  Include Files

#include "main.h"

// 4.2   External Data Structures

// 4.3 External Function Prototypes

// 5 LOCAL CONSTANTS AND MACROS

// 6 MODULE DATA STRUCTURES

// 6.1  Local Function Prototypes

// 6.2  Local Data Structures

// 7 MODULE CODE

// ===========================================================================

// 7.1

uint32_t gpio_init(void) {

/* 7.1.1   Functional Description

   This routine is responsible for initializing the driver hardware.

   7.1.2   Parameters:

   NONE

   7.1.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   uint32_t   result = CFG_STATUS_OK;

// 7.1.5   Code

   // LEDs
   MAP_GPIO_setAsOutputPin(GPIO_LED_1 >> 8, GPIO_LED_1 & 0xFF);
   MAP_GPIO_setAsOutputPin(GPIO_LED_2 >> 8, GPIO_LED_2 & 0xFF);
   MAP_GPIO_setAsOutputPin(GPIO_LED_3 >> 8, GPIO_LED_3 & 0xFF);
   MAP_GPIO_setAsOutputPin(GPIO_LED_4 >> 8, GPIO_LED_4 & 0xFF);

   // Switches
   MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_SW_1 >> 8, GPIO_SW_1 & 0xFF);
   MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_SW_2 >> 8, GPIO_SW_2 & 0xFF);

   // GPIO
   MAP_GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN6);
   MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN6);
   MAP_GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN5);
   MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN5);

   return result;

}  // end gpio_init()


// ===========================================================================

// 7.2

void gpio_isr(void *arg) {

/* 7.2.1   Functional Description

   This routine will service the GPX Interrupt.

   7.2.2   Parameters:

   NONE

   7.2.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

// 7.2.5   Code

} // end gpio_isr()


// ===========================================================================

// 7.3

void gpio_set_val(uint32_t gpio, uint32_t state) {

/* 7.3.1   Functional Description

   This routine will set the selected GPIO data bits.

   7.3.2   Parameters:

   gpio     GPIO to change
   state    ON/OFF/TOGGLE state

   7.3.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.3.4   Data Structures

// 7.3.5   Code

   switch(state) {
      case GPIO_LED_OFF :
         MAP_GPIO_setOutputLowOnPin(gpio >> 8, gpio & 0xFF);
         break;
      case GPIO_LED_ON :
         MAP_GPIO_setOutputHighOnPin(gpio >> 8, gpio & 0xFF);
         break;
      case GPIO_LED_TOGGLE :
         MAP_GPIO_toggleOutputOnPin(gpio >> 8, gpio & 0xFF);
         break;
      case GPIO_LED_ALL_OFF :
         MAP_GPIO_setOutputLowOnPin(GPIO_LED_1 >> 8, GPIO_LED_1 & 0xFF);
         MAP_GPIO_setOutputLowOnPin(GPIO_LED_2 >> 8, GPIO_LED_2 & 0xFF);
         MAP_GPIO_setOutputLowOnPin(GPIO_LED_3 >> 8, GPIO_LED_3 & 0xFF);
         MAP_GPIO_setOutputLowOnPin(GPIO_LED_4 >> 8, GPIO_LED_4 & 0xFF);
         break;
      case GPIO_LED_ALL_ON :
         MAP_GPIO_setOutputHighOnPin(GPIO_LED_1 >> 8, GPIO_LED_1 & 0xFF);
         MAP_GPIO_setOutputHighOnPin(GPIO_LED_2 >> 8, GPIO_LED_2 & 0xFF);
         MAP_GPIO_setOutputHighOnPin(GPIO_LED_3 >> 8, GPIO_LED_3 & 0xFF);
         MAP_GPIO_setOutputHighOnPin(GPIO_LED_4 >> 8, GPIO_LED_4 & 0xFF);
         break;
   }

}  // end gpio_val_set()


// ===========================================================================

// 7.4

uint8_t gpio_key(void) {

/* 7.4.1   Functional Description

   This routine will return the current state of the User KEY 0-3 Switches
   and the slide switches SW 0-9.

   7.4.2   Parameters:

   NONE

   7.4.3   Return Values:

   result   KEY 0-13 Switch states

-----------------------------------------------------------------------------
*/

// 7.4.4   Data Structures

   uint8_t  key = 0;

// 7.4.5   Code

   key |= MAP_GPIO_getInputPinValue(GPIO_SW_1 >> 8, GPIO_SW_1 & 0xFF) ? 1 : 0;
   key |= MAP_GPIO_getInputPinValue(GPIO_SW_2 >> 8, GPIO_SW_2 & 0xFF) ? 2 : 0;

   return key;

}  // end gpio_key()
