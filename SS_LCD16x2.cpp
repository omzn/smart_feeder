#include "SS_LCD16x2.h"

LCD16x2::LCD16x2() {
  cursor_x = cursor_y = 0;
}

void LCD16x2::begin(int contrast) {
  delay(100);
  cmd(0x34);
  delay(5);
  cmd(0x34);
  delay(5);
  cmd(0x34);
  delay(40);

  Wire.beginTransmission(LCD_ADDR);
  Wire.write(0x00); // CO = 0,RS = 0
  Wire.write(0x35);
  Wire.write(0x41);
  Wire.write(0x80 | contrast);
  Wire.write(0xC0 | contrast);
  Wire.write(0x34);
  Wire.endTransmission();

  cmd(0x01);
  delay(400);
  cmd(0x0C);
  cmd(0x06);
  delay(500);  
}

void LCD16x2::cmd(unsigned char x) {
  Wire.beginTransmission(LCD_ADDR);
  Wire.write(0b00000000); // CO = 0,RS = 0
  Wire.write(x);
  Wire.endTransmission();  
}


void LCD16x2::data(unsigned char x){
  Wire.beginTransmission(LCD_ADDR);
  Wire.write(0b01000000); // CO = 0, RS = 1
  Wire.write(x ^ 0x80);
  Wire.endTransmission();
}

void LCD16x2::print(int x) {
  char s[20];
  sprintf(s,"%d",x);
  print(s);
}

void LCD16x2::print(const char *s) {
  int i = 0;
  Wire.beginTransmission(LCD_ADDR);
  while (*s) {
    if (*(s + 1)) {
      Wire.write(0b11000000); // CO = 1, RS = 1
      Wire.write(*s ^ 0x80);
    } else {
      Wire.write(0b01000000); // CO = 0, RS = 1
      Wire.write(*s ^ 0x80);
    }
    s++;
    i++;
  }
  Wire.endTransmission();
  cursor_x += i;
}
 
void LCD16x2::setCursor(unsigned char x, unsigned char y) {
  cmd(0x80 | (y * 0x40 + x));
  cursor_x = x; 
  cursor_y = y;
}

