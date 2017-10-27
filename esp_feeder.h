#ifndef ESP_FEEDER_H
#define ESP_FEEDER_H

#define PIN_SDA          (5)
#define PIN_SCL          (4)
#define PIN_FEED_SW      (14)
//#define PIN_FEED_SW      (2) // pin 2 is hw pulled up

//#define PIN_LIGHT        (2)
//#define PIN_BUTTON       (0)
//#define PIN_LED         (15)
//#define PIN_TFT_DC       (4)
//#define PIN_TFT_CS      (15)
//#define PIN_SD_CS       (16)
//#define PIN_SPI_CLK   (14)
//#define PIN_SPI_MOSI  (13)
//#define PIN_SPI_MISO  (12)

#define DRV8830_1_ADDRESS      (0x64)

#define EEPROM_SSID_ADDR             (0)
#define EEPROM_PASS_ADDR            (32)
#define EEPROM_MDNS_ADDR            (96)
#define EEPROM_SCHEDULE_ADDR       (128)
#define EEPROM_FEEDER_ADDR         (138)
#define EEPROM_DAYRESET_ADDR          (140)
#define EEPROM_LAST_ADDR           (141)

#define UDP_LOCAL_PORT      (2390)
#define NTP_PACKET_SIZE       (48)
#define SECONDS_UTC_TO_JST (32400)

//#define DEBUG

#endif
