#ifdef BIT_EXPORTS
#define BIT_API __declspec(dllexport)
#else
#define BIT_API __declspec(dllimport)
#endif

#pragma once

// Device Info
typedef struct _bit_dev_info_t {
   uint32_t    flags;
   uint32_t    type;
   uint32_t    id;
   uint32_t    locid;
   char        serial[16];
   char        desc[64];
   void       *handle;
} bit_dev_info_t, *pbit_dev_info_t;

#define  BIT_VID              0x0403
#define  BIT_PID              0x6001
#define  BIT_VID_PID_STR      "VID_0403&PID_6001"
#define  BIT_VID_PID          0x04036001

#define  BIT_VID_ALT          0x0403
#define  BIT_PID_ALT          0x6014
#define  BIT_VID_PID_STR_ALT  "VID_0403&PID_6014"
#define  BIT_VID_PID_ALT      0x04036014

#define  BIT_OK               0x00000000
#define  BIT_ERROR            0x80000001
#define  BIT_ERR_MSG_NULL     0x80000002
#define  BIT_ERR_LEN_NULL     0x80000004
#define  BIT_ERR_LEN_MAX      0x80000008
#define  BIT_ERR_FRAMING      0x80000010
#define  BIT_ERR_OVERRUN      0x80000020
#define  BIT_ERR_PARITY       0x80000040
#define  BIT_ERR_TX_DROP      0x80000080
#define  BIT_ERR_RX_DROP      0x80000100
#define  BIT_ERR_CRC          0x80000200
#define  BIT_ERR_OPEN         0x80000400
#define  BIT_ERR_RESP         0x80000800
#define  BIT_ERR_THREAD       0x80001000
#define  BIT_ERR_INFO         0x80002000
#define  BIT_ERR_DEV          0x80004000
#define  BIT_ERR_DEV_CNT      0x80008000
#define  BIT_ERR_POOL         0x80010000

#define  BIT_MAX_DEVICES      8

#define  BIT_TP0              0x01
#define  BIT_TP1              0x02
#define  BIT_TP2              0x04
#define  BIT_TP3              0x08
#define  BIT_TP4              0x10
#define  BIT_TP5              0x20
#define  BIT_TP6              0x40
#define  BIT_TP7              0x80

#define  BIT_ON               0x01
#define  BIT_OFF              0x02
#define  BIT_WR               0x03

BIT_API uint32_t bit_query(bit_dev_info_t **devinfo);
BIT_API int32_t  bit_init(uint8_t com_port);
BIT_API void     bit_tx(uint8_t dbus, uint8_t state);
BIT_API void     bit_final(void);

