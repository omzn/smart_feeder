#include "esp_feeder.h"
#include "feeder.h"

drumFeeder::drumFeeder(uint8_t address, uint8_t pin) : drv8830_i2c(address) {
  _address = address;
  _rotate_sw_pin = pin;
  num_rotated = 0;
  num_fed = 0;
  for (int i = 0; i < 3; i++) {
    ft_h[i] = -1;
    ft_m[i] = -1;
    ft_r[i] = 0;
  }
}

void drumFeeder::reset() {
  num_rotated = 0;
  num_fed = 0;
  EEPROM.write(EEPROM_FEEDER_ADDR, char(num_rotated));
  EEPROM.write(EEPROM_FEEDER_ADDR + 1, char(num_fed));
  EEPROM.commit();
#ifdef DEBUG
  Serial.println("reset fed & rotated");
#endif
}

void drumFeeder::rotated(int v) {
  num_rotated = v;
}

void drumFeeder::fed(int v) {
  num_fed = v;
}

int drumFeeder::rotated() {
  return num_rotated;
}

int drumFeeder::fed() {
  return num_fed;
}

void drumFeeder::incRotated(int v) {
  num_rotated += v;
  EEPROM.write(EEPROM_FEEDER_ADDR, char(num_rotated));
  EEPROM.commit();
#ifdef DEBUG
  Serial.print("saved rotated:");
  Serial.println(num_rotated);
#endif
}

void drumFeeder::incFed(int v) {
  num_fed += v;
  EEPROM.write(EEPROM_FEEDER_ADDR + 1, char(num_fed));
  EEPROM.commit();
#ifdef DEBUG
  Serial.print("saved num_fed:");
  Serial.println(num_fed);
#endif
}

uint8_t drumFeeder::doFeed(int n) {
  int32_t count = 0;
  uint8_t success = 0;
  clearFault();
  floatMotor();
  delay(100);
  startMotor(0x3e, CW);
  for (int i = 0; i < n; i++) {
    success++;
    while (digitalRead(_rotate_sw_pin) == 0) { // skip SW == 0
      count++;
      if (count > 1500) { // timeout 15s
        success = 0;
        break;
      }
      delay(10);
    }
    count = 0;
    while (digitalRead(_rotate_sw_pin) == 1) { // wait until SW == 0
      count++;
      if (count > 1500) { // timeout 15s
        success = 0;
        break;
      }
      delay(10);
    }
    if (success == 0) {
      break;
    }
//    delay(10000);
  }
  floatMotor();
  delay(100);
  stopMotor();

  incFed(1);
  incRotated(success);
  return success;
}

int drumFeeder::feedtime_h(int v) {
  return ft_h[v];
}
int drumFeeder::feedtime_m(int v) {
  return ft_m[v];
}
int drumFeeder::feedtime_r(int v) {
  return ft_r[v];
}

String drumFeeder::nextFeedTime() {
  char s[6];
  String ret;  
  sprintf(s,"%02d:%02d",ft_h[num_fed < 3 ? num_fed : 0],ft_m[num_fed < 3 ? num_fed : 0]);
  ret = s;
  return ret;
}

int drumFeeder::enable() {
  return _enable_schedule;
}


// スケジュールを有効(1)・無効(0)にする．
void drumFeeder::enable(int v) {
  _enable_schedule = (v == 0 ? 0 : 1);
}

// スケジュールの時刻を設定する．3スロット
void drumFeeder::schedule(int v, int h, int m, int r) {
  if (h >= 0 && h <= 23) {
    ft_h[v] = h;
  } else {
    ft_h[v] = -1;
  }
  if (m >= 0 && m <= 59) {
    ft_m[v] = m;
  } else {
    ft_m[v] = -1;
  }
  if (r >= 1) {
    ft_r[v] = r;
  } else {
    ft_r[v] = 0;
  }
}

// hh:mm時点でfeedすべきかどうかを判断する．1秒毎に呼び出される．
void drumFeeder::control(int hh, int mm) {
  if (_enable_schedule) {
    for (int i = num_fed; i < 3; i++) {
      if (ft_h[i] >= 0 && ft_h[i] < 24 && ft_m[i] >= 0 && ft_m[i] < 60 && ft_r[i] > 0) {
        if (hh > ft_h[i] || hh >= ft_h[i] && mm >= ft_m[i]) {
#ifdef DEBUG
          Serial.print("do feed ");
          Serial.println(ft_r[i]);
#endif
          doFeed(ft_r[i]);
          break;
        }
      }
    }
  }
}

