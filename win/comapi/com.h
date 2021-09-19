#pragma once

#define  COM_SOF              0x7E
#define  COM_EOF              0x7D
#define  COM_ESC              0x7C
#define  COM_BIT              0x20

#define  COM_RX_TIMEOUT       100
#define  COM_TX_TIMEOUT       100
#define  COM_THREAD_TIMEOUT   50
#define  COM_RX_Q_SIZE        1024
#define  COM_TX_Q_SIZE        1024

#define  COM_EPID_NONE        0x00
#define  COM_EPID_NEXT        0x20
#define  COM_EPID_CTL         0x40
#define  COM_EPID_PIPE        0x80
