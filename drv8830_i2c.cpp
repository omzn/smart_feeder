#include "Arduino.h"
#include <Wire.h>
#include "drv8830_i2c.h"

drv8830_i2c::drv8830_i2c(uint8_t address) {
  _address = address;
}

void drv8830_i2c::startMotor(uint8_t val,uint8_t dir) {
  _power = val;
  _direction = dir;
  Wire.beginTransmission(_address);
  Wire.write(0x00);
  Wire.write(_power << 2 | _direction);
  Wire.endTransmission();
  delay(100);
}

void drv8830_i2c::stopMotor() {
  _power = 0;
  _direction = BREAK;
  Wire.beginTransmission(_address);
  Wire.write(0x00);
  Wire.write(_power << 2 | _direction);
  Wire.endTransmission();
  delay(100);
}

void drv8830_i2c::floatMotor() {
  _power = 0;
  _direction = FLOAT;
  Wire.beginTransmission(_address);
  Wire.write(0x00);
  Wire.write(_power << 2 | _direction);
  Wire.endTransmission();
  delay(100);
}

void drv8830_i2c::clearFault() {
  Wire.beginTransmission(_address);
  Wire.write(0x01);
  Wire.write(0x00);
  Wire.endTransmission();
}

