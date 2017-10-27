#ifndef SS_LCD16X2_H
#define SS_LCD16X2_H

#include "Arduino.h"
#include <Wire.h>

#define LCD_ADDR 0x3A

class LCD16x2 {
  public:
    LCD16x2();
    void begin(int contrast);
    void setCursor(unsigned char x, unsigned char y);
    void print(const char *s);    
    void print(int x);    
    
  protected:
    unsigned char cursor_x;
    unsigned char cursor_y;
    
    void cmd(unsigned char x);
    void data(unsigned char x);
};

#endif

