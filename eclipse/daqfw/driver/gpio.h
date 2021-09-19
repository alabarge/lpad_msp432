#pragma once

#define  GPIO_OK               0x00

#define  GPIO_LED_ON           0
#define  GPIO_LED_OFF          1
#define  GPIO_LED_TOGGLE       2
#define  GPIO_LED_ALL_OFF      3
#define  GPIO_LED_ALL_ON       4

#define  GPIO_LED_1           ((GPIO_PORT_P1 << 8) | GPIO_PIN0)
#define  GPIO_LED_2           ((GPIO_PORT_P2 << 8) | GPIO_PIN0)
#define  GPIO_LED_3           ((GPIO_PORT_P2 << 8) | GPIO_PIN1)
#define  GPIO_LED_4           ((GPIO_PORT_P2 << 8) | GPIO_PIN2)

#define  GPIO_SW_1            ((GPIO_PORT_P1 << 8) | GPIO_PIN1)
#define  GPIO_SW_2            ((GPIO_PORT_P1 << 8) | GPIO_PIN4)

#define  GPIO_LED_HB           GPIO_LED_1
#define  GPIO_LED_COM          GPIO_LED_2
#define  GPIO_LED_DAQ          GPIO_LED_3
#define  GPIO_LED_PIPE         GPIO_LED_4

#define  GPIO_LED_ERR          GPIO_LED_4

#define  GPIO_KEY_0            1
#define  GPIO_KEY_1            2
#define  GPIO_KEY_ALL_OFF      0x03

uint32_t gpio_init(void);
void     gpio_isr(void *arg);
void     gpio_set_val(uint32_t led, uint32_t state);
uint8_t  gpio_key(void);

