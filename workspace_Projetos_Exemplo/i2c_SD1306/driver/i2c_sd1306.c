/* reaper7 */

#include "../include/driver/i2c_sd1306.h"

#include "driver/i2c.h"

static uint32_t _voltage, _current, _shuntvoltage, _power;

bool ICACHE_FLASH_ATTR
SD1306_writereg16(uint8 reg, uint16_t data)
{
  i2c_start(); //Start i2c
  i2c_writeByte(SD1306_ADDRESS);
  if (!i2c_check_ack()) {
    //os_printf("-%s-%s START slave not ack... return \r\n", __FILE__, __func__);
    i2c_stop();
    return(0);
  }
  i2c_writeByte(reg);
  if(!i2c_check_ack()){
    //os_printf("-%s-%s REG slave not ack... return \r\n", __FILE__, __func__);
    i2c_stop();
    return(0);
  }
  
  uint8_t dlv;
  dlv = (uint8_t)data;
  data >>= 8;
  
  //os_printf("-%s-%s DATA H[%d] L[%d] \r\n", __FILE__, __func__, (uint8_t)data, dlv);
  
  i2c_writeByte((uint8_t)data);
  if(!i2c_check_ack()){
    //os_printf("-%s-%s HDATA slave not ack... return \r\n", __FILE__, __func__);
    i2c_stop();
    return(0);
  }
  
  i2c_writeByte(dlv);
  if(!i2c_check_ack()){
    //os_printf("-%s-%s LDATA slave not ack... return \r\n", __FILE__, __func__);
    i2c_stop();
    return(0);
  }
  
  i2c_stop();
    
  return(1); 
}


bool ICACHE_FLASH_ATTR
SD1306_Init()
{
  if (!SD1306_writereg16(1, 2))
    return 0;
    
  return 1;
}


