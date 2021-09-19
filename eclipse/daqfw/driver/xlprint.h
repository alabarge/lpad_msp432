#pragma once

int  xlprint(const char *format, ...);
int  xlprints(char *buf, const char *format, ...);
void xlprint_open(uint32_t module, uint8_t ext_int);
void EUSCIA0_IRQHandler(void);
