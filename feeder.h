#ifndef FEEDER_H
#define FEEDER_H

#include "Arduino.h"
#include "drv8830_i2c.h"
#include <EEPROM.h>

class drumFeeder : public drv8830_i2c {
  public:
    drumFeeder(uint8_t address,uint8_t pin);
    void reset();
    uint8_t doFeed(int n);
    int rotated();
    int fed();
    void rotated(int v);
    void fed(int v);
    void incRotated(int v);
    void incFed(int v);

    int  enable();
    void enable(int v);
    void schedule(int v, int ft_h, int ft_m, int ft_r);
    void control(int hh, int mm);

    String nextFeedTime();

    int feedtime_h(int v);
    int feedtime_m(int v);
    int feedtime_r(int v);

  protected:
    uint8_t _enable_schedule;
    uint8_t _rotate_sw_pin;
    int num_rotated;
    int num_fed;
    int ft_h[3];
    int ft_m[3];
    int ft_r[3];    
};

#endif
