#ifndef __I2C_INA219_H
#define	__I2C_INA219_H

#include "c_types.h"
#include "ets_sys.h"
#include "osapi.h"

//#define INA219_ADDRESS 0x80
#define SD1306_ADDRESS 0x78



bool SD1306_Init(void);
int32_t SD1306_GetVal(uint8 mode);

#endif
