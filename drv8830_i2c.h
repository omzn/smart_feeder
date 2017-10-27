#ifndef DRV8830_I2C_H
#define DRV8830_I2C_H

#include "Arduino.h"
#include <Wire.h>

#define FLOAT 0x00
#define CW    0x01
#define CCW   0x02
#define BREAK 0x03

class drv8830_i2c {
  public:
    drv8830_i2c(uint8_t address);
    void    startMotor(uint8_t val,uint8_t dir);
    void    stopMotor();
    void    floatMotor();
    void    clearFault();
  protected:
    int8_t _address;
    uint8_t _power;
    uint8_t _direction;
};

#endif
