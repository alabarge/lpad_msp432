#pragma once

#define  ADC_OK              0x0000

#define  SMCLK_FREQUENCY     48000000
#define  SAMPLE_FREQUENCY    4000

typedef struct _adc_ctl_t {
    uint32_t   channel;
    uint32_t   packets;
    uint32_t   packet_cnt;
} adc_ctl_t;

uint32_t  adc_init(void);
void      DMA_INT1_IRQHandler(void);
void      adc_run(uint32_t opcode, uint32_t packets);
uint16_t *adc_sam_ptr(void);
