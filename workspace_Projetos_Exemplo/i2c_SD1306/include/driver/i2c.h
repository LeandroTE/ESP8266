/*
    I2C driver for the ESP8266 
    Copyright (C) 2014 Rudy Hardeman (zarya) 

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef __I2C_H__
#define __I2C_H__

#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"

#define I2C_SLEEP_TIME 10 

// SDA on GPIO4
#define I2C_SDA_MUX PERIPHS_IO_MUX_GPIO4_U
#define I2C_SDA_FUNC FUNC_GPIO4
#define I2C_SDA_PIN 4

// SCK on GPIO14
#define I2C_SCK_MUX PERIPHS_IO_MUX_MTMS_U
#define I2C_SCK_FUNC FUNC_GPIO14
#define I2C_SCK_PIN 14

#if 1
#define I2C_MASTER_GPIO_SET(pin)  \
    gpio_output_set(1<<pin,0,1<<pin,0)

#define I2C_MASTER_GPIO_CLR(pin) \
    gpio_output_set(0,1<<pin,1<<pin,0)

#define I2C_MASTER_GPIO_OUT(pin,val) \
    if(val) I2C_MASTER_GPIO_SET(pin);\
    else I2C_MASTER_GPIO_CLR(pin)
#endif

#define I2C_MASTER_SDA_HIGH_SCL_HIGH()  \
    gpio_output_set(1<<I2C_MASTER_SDA_GPIO | 1<<I2C_MASTER_SCL_GPIO, 0, 1<<I2C_MASTER_SDA_GPIO | 1<<I2C_MASTER_SCL_GPIO, 0)

#define I2C_MASTER_SDA_HIGH_SCL_LOW()  \
    gpio_output_set(1<<I2C_MASTER_SDA_GPIO, 1<<I2C_MASTER_SCL_GPIO, 1<<I2C_MASTER_SDA_GPIO | 1<<I2C_MASTER_SCL_GPIO, 0)

#define I2C_MASTER_SDA_LOW_SCL_HIGH()  \
    gpio_output_set(1<<I2C_MASTER_SCL_GPIO, 1<<I2C_MASTER_SDA_GPIO, 1<<I2C_MASTER_SDA_GPIO | 1<<I2C_MASTER_SCL_GPIO, 0)

#define I2C_MASTER_SDA_LOW_SCL_LOW()  \
    gpio_output_set(0, 1<<I2C_MASTER_SDA_GPIO | 1<<I2C_MASTER_SCL_GPIO, 1<<I2C_MASTER_SDA_GPIO | 1<<I2C_MASTER_SCL_GPIO, 0)

//SCK on GPIO0 (untested)
//#define I2C_SCK_MUX PERIPHS_IO_MUX_GPIO0_U
//#define I2C_SCK_PIN 0
//#define I2C_SCK_FUNC FUNC_GPIO0

#define i2c_read() GPIO_INPUT_GET(GPIO_ID_PIN(I2C_SDA_PIN)); 

void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_send_ack(uint8 state);
uint8 i2c_check_ack(void);
uint8 i2c_readByte(void);
void i2c_writeByte(uint8 data);

#endif