#pragma once

#include "cm.h"

#define  UART_START_FRAME      0x7E
#define  UART_END_FRAME        0x7D
#define  UART_ESCAPE           0x7C
#define  UART_STUFFED_BIT      0x20

#define  UART_FLAG_ESCAPE      0x01
#define  UART_FLAG_START       0x02
#define  UART_FLAG_MSG         0x04

#define  UART_OSR              16

#define  UART_RX_IDLE          0
#define  UART_RX_MSG           2
#define  UART_RX_ESCAPE        3
#define  UART_RX_STORE         4

#define  UART_TX_IDLE          0
#define  UART_TX_MSG           1
#define  UART_TX_ESCAPE        2
#define  UART_TX_EOF           3
#define  UART_TX_LAST          4

#define  UART_OK               0x00000000
#define  UART_ERROR            0x80000001
#define  UART_ERR_MSG_NULL     0x80000002
#define  UART_ERR_LEN_NULL     0x80000004
#define  UART_ERR_LEN_MAX      0x80000008
#define  UART_ERR_FRAMING      0x80000010
#define  UART_ERR_OVERRUN      0x80000020
#define  UART_ERR_PARITY       0x80000040
#define  UART_ERR_TX_DROP      0x80000080
#define  UART_ERR_RX_DROP      0x80000100
#define  UART_ERR_CRC          0x80000200
#define  UART_ERR_OPEN         0x80000400

#define  UART_OP_START         0x00000001
#define  UART_OP_STOP          0x00000002
#define  UART_OP_BYPASS        0x00000004

#define  UART_MSGLEN_UINT8     512
#define  UART_MSGLEN_UINT32    (UART_MSGLEN_UINT8 >> 2)

#define  UART_PIPELEN_UINT8    1024

#define  UART_TX_QUE           8
#define  UART_RX_QUE           8

// Transmit Queue
typedef struct _uart_txq_t {
   uint8_t      *buf[UART_TX_QUE];
   uint8_t       tail;
   uint8_t       head;
   uint8_t       slots;
   uint8_t       state;
   uint16_t      msglen;
   uint16_t      count;
   uint8_t      *uint8;
} uart_txq_t, *puart_txq_t;

// Receive Queue
typedef struct _uart_rxq_t {
   uint8_t       state;
   uint16_t      count;
   uint8_t       slotid;
   pcmq_t        slot;
   pcm_msg_t     msg;
} uart_rxq_t, *puart_rxq_t;

uint32_t  uart_init(uint32_t module, uint8_t ext_int, uint8_t port);
void      EUSCIA2_IRQHandler(void);
void      uart_tx(pcm_msg_t msg, uint16_t msglen);
void      uart_cmio(uint8_t op_code, pcm_msg_t msg);
void      uart_msgtx(void);
void      uart_pipe(pcm_pipe_t pipe, uint16_t msglen);
void      uart_report(void);

