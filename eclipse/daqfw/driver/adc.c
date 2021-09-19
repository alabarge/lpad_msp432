/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      MSP432 ADC Driver

   1.2 Functional Description

      The ADC Interface routines are contained in this module.

   1.3 Specification/Design Reference

      See fw_cfg.h under the BOOT/SHARE directory.

   1.4 Module Test Specification Reference

      None

   1.5 Compilation Information

      See fw_cfg.h under the BOOT/SHARE directory.

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
      7.1   adc_init()
      7.2   DMA_INT1_IRQHandler()
      7.3   adc_run()
      7.4   adc_sam_ptr()

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

   static   DMA_ControlTable MSP_EXP432P401RLP_DMAControlTable[32];

   #pragma  DATA_ALIGN(MSP_EXP432P401RLP_DMAControlTable, 1024)

   static   uint16_t *switch_data;
   static   uint32_t  adc_input[] = {ADC_INPUT_A13, ADC_INPUT_A8,
                                     ADC_INPUT_A9, ADC_INPUT_A11};

   static   uint16_t  data_array1[DAQ_MAX_LEN];
   static   uint16_t  data_array2[DAQ_MAX_LEN];

   static   uint16_t  seq_id0 = 0, seq_id1 = 0, seq_id2 = 0, seq_id3 = 0;
   static   adc_ctl_t adc_ctl = {0};


   /* Timer_A PWM Configuration Parameter */
   static Timer_A_PWMConfig pwm_config = {
      TIMER_A_CLOCKSOURCE_SMCLK,
      TIMER_A_CLOCKSOURCE_DIVIDER_1,
      (SMCLK_FREQUENCY / SAMPLE_FREQUENCY),
      TIMER_A_CAPTURECOMPARE_REGISTER_1,
      TIMER_A_OUTPUTMODE_SET_RESET,
      (SMCLK_FREQUENCY / SAMPLE_FREQUENCY) / 2
   };

// 7 MODULE CODE

// ===========================================================================

// 7.1

uint32_t adc_init(void) {

/* 7.1.1   Functional Description

   This routine is responsible for initializing the driver hardware.

   7.1.2   Parameters:

   NONE

   7.1.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   uint32_t    result = CFG_STATUS_OK;

// 7.1.5   Code

   adc_ctl.channel    = 0;
   adc_ctl.packets    = 0;
   adc_ctl.packet_cnt = 0;

   // Initializing ADC (MCLK/1/1)
   ADC14_enableModule();
   ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, 0);

   // Configuring Timer_A to have a period of approximately 500ms and
   // an initial duty cycle of 10% of that (3200 ticks)
   Timer_A_generatePWM(TIMER_A0_BASE, &pwm_config);
   MAP_Interrupt_enableInterrupt(INT_DMA_INT1);
   MAP_Interrupt_enableMaster();

   ADC14_setSampleHoldTrigger(ADC_TRIGGER_SOURCE1, false);

   // Configure GPIO (4.7 A6)
   GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN0,
                                              GPIO_TERTIARY_MODULE_FUNCTION);
   // Configure GPIO (4.5 A8)
   GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN5,
                                              GPIO_TERTIARY_MODULE_FUNCTION);
   // Configure GPIO (4.4 A9)
   GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN4,
                                              GPIO_TERTIARY_MODULE_FUNCTION);
   // Configure GPIO (4.2 A11)
   GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN2,
                                              GPIO_TERTIARY_MODULE_FUNCTION);
   // Configure ADC Memory
   //MEM0: A6
   ADC14_configureSingleSampleMode(ADC_MEM0, true);
   ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                   ADC_INPUT_A13, false);

   // Set ADC result format to signed binary
   ADC14_setResultFormat(ADC_UNSIGNED_BINARY);

   // Configure DMA module
   DMA_enableModule();
   DMA_setControlBase(MSP_EXP432P401RLP_DMAControlTable);
   DMA_disableChannelAttribute(DMA_CH7_ADC14,
                               UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
                               UDMA_ATTR_HIGH_PRIORITY |
                               UDMA_ATTR_REQMASK);

   /* Setting Control Indexes. In this case we will set the source of the
    * DMA transfer to ADC14 Memory 0
    *  and the destination to the
    * destination data array. */
   MAP_DMA_setChannelControl(
       UDMA_PRI_SELECT | DMA_CH7_ADC14,
       UDMA_SIZE_16 | UDMA_SRC_INC_NONE |
       UDMA_DST_INC_16 | UDMA_ARB_1);
   MAP_DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH7_ADC14,
                              UDMA_MODE_PINGPONG, (void*)&ADC14->MEM[0],
                              data_array1, DAQ_MAX_LEN);

   MAP_DMA_setChannelControl(
       UDMA_ALT_SELECT | DMA_CH7_ADC14,
       UDMA_SIZE_16 | UDMA_SRC_INC_NONE |
       UDMA_DST_INC_16 | UDMA_ARB_1);
   MAP_DMA_setChannelTransfer(UDMA_ALT_SELECT | DMA_CH7_ADC14,
                              UDMA_MODE_PINGPONG, (void*)&ADC14->MEM[0],
                              data_array2, DAQ_MAX_LEN);

   // Assigning Interrupts
   MAP_DMA_assignInterrupt(DMA_INT1, 7);
   MAP_DMA_assignChannel(DMA_CH7_ADC14);
   MAP_DMA_clearInterruptFlag(7);

   // Print Hardware Version to Serial Port
   if (gc.trace & CFG_TRACE_ID) {
      xlprint("%-13s base:irq %08X:%d\n", "/dev/adc", ADC14_BASE, INT_DMA_INT1);
   }

   return result;

}  // end adc_init()


// ===========================================================================

// 7.2

void DMA_INT1_IRQHandler(void) {

/* 7.2.1   Functional Description

   This routine will service the hardware interrupt; the completion interrupt
   for ADC14 MEM0.

   7.2.2   Parameters:

   NONE

   7.2.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

// 7.2.5   Code

    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN6);

   // Switch between primary and alternate buffers with DMA's PingPong mode
   if(DMA_getChannelAttribute(7) & UDMA_ATTR_ALTSELECT) {
       DMA_setChannelControl(
       UDMA_PRI_SELECT | DMA_CH7_ADC14,
       UDMA_SIZE_16 | UDMA_SRC_INC_NONE |
       UDMA_DST_INC_16 | UDMA_ARB_1);
       DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH7_ADC14,
                         UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[0],
                         data_array1, DAQ_MAX_LEN);
       switch_data = data_array1;
   }
   else {
       DMA_setChannelControl(
          UDMA_ALT_SELECT | DMA_CH7_ADC14,
          UDMA_SIZE_16 | UDMA_SRC_INC_NONE |
          UDMA_DST_INC_16 | UDMA_ARB_1);
       DMA_setChannelTransfer(UDMA_ALT_SELECT | DMA_CH7_ADC14,
                             UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[0],
                             data_array2, DAQ_MAX_LEN);
       switch_data = data_array2;
   }

   // Rotate ADC channels
   ADC14->CTL0 &= ~BIT1;
   if (++adc_ctl.channel == DAQ_MAX_CH) adc_ctl.channel = 0;
   ADC14->MCTL[0] = adc_input[adc_ctl.channel];
   ADC14->CTL0 |= BIT1;

   // Always Send the Packet Ready Interrupt
   cm_local(CM_ID_DAQ_SRV, DAQ_INT_IND, DAQ_INT_FLAG_PKT, DAQ_OK);

   // Send Done Interrupt
   if (++adc_ctl.packet_cnt == adc_ctl.packets) {
      cm_local(CM_ID_DAQ_SRV, DAQ_INT_IND, DAQ_INT_FLAG_DONE, DAQ_OK);
      MAP_DMA_disableChannel(7);
      MAP_ADC14_disableConversion();
   }

   MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN6);

} // end DMA_INT1_IRQHandler()


// ===========================================================================

// 7.3

void adc_run(uint32_t opcode, uint32_t packets) {

/* 7.3.1   Functional Description

   This routine will start or stop the ADC acquisition.

   7.3.2   Parameters:

   opcode   Start/Stop flags
   packets  Number of packets to acquire

   7.3.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.3.4   Data Structures

   uint32_t    i;

// 7.3.5   Code

   adc_ctl.channel    = 0;
   adc_ctl.packets    = packets;
   adc_ctl.packet_cnt = 0;

   for (i=0;i<DAQ_MAX_LEN;i+=4) {
      data_array1[i+0] = seq_id0;
      data_array1[i+1] = seq_id1;
      data_array1[i+2] = seq_id2;
      data_array1[i+3] = seq_id3;
      data_array2[i+0] = seq_id0++;
      data_array2[i+1] = seq_id1++;
      data_array2[i+2] = seq_id2++;
      data_array2[i+3] = seq_id3++;
   }

   //
   // Start ADC Acquisition
   //
   if (opcode & DAQ_CMD_RUN) {
       MAP_DMA_enableChannel(7);
       MAP_ADC14_enableConversion();
   }
   //
   // Stop ADC Acquisition
   //
   else {
       MAP_DMA_disableChannel(7);
       MAP_ADC14_disableConversion();
   }

} // end adc_run()


// ===========================================================================

// 7.4

uint16_t *adc_sam_ptr(void) {

/* 7.4.1   Functional Description

   This routine will return the sample data pointer.

   7.4.2   Parameters:

   NONE

   7.4.3   Return Values:

   return   Sample Pointer

-----------------------------------------------------------------------------
*/

// 7.4.4   Data Structures

// 7.4.5   Code

   return switch_data;

} // end adc_sam_ptr()

