// Thư viện LCD
#include <LiquidCrystal.h>

// Thư viện LoRa
#include <RHDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

// Thư viện DHT
#include "DHT.h"

// Định nghĩa LoRa
#define COORD_ADDR  100
#define NODE_ADDR   101
#define NODE_ID     10001

// Định nghĩa DHT và độ ẩm đất
#define DHT11_PIN   3
#define HUMD_PIN    A3

// Biến LCD
LiquidCrystal lcd(9, 8, 7, 6, 5, 4);

// Biến LoRa
RH_RF95 driver;
RHDatagram manager(driver, NODE_ADDR);
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

// Biến DHT
DHT dht(DHT11_PIN, DHT11);

// Biến định thời
unsigned long prevMillis = 0, interval = 2000;

// Biến dữ liệu
float temp = 0, hum = 0, humd = 0;
char buff[17] = "";

// Hàm khởi tạo
void setup() {
  // Khởi tạo LCD
  lcd.begin(16, 2);
  // Khởi tạo Serial
  Serial.begin(9600);
  while (!Serial);
  // Khởi tạo LoRa
  manager.init();
  driver.setModemConfig(RH_RF95::Bw125Cr48Sf4096);
  driver.setTxPower(23);
  // Khởi tạo DHT
  dht.begin();
}

// Hàm lặp vô hạn
void loop() {
  // Nếu LoRa sẵn sàng nhận dữ liệu
  if (manager.available()) {
    // Biến đọc dữ liệu LoRa
    uint8_t len = sizeof(buf);
    uint8_t from;
    // Nếu nhận được dữ liệu LoRa
    if (manager.recvfrom(buf, &len, &from)) {
      // Nếu từ Coord thì
      if (from == COORD_ADDR) {
        // Khởi tạo chuỗi dữ liệu
        String dataStr = String(NODE_ID) + "," + String(temp, 1) + "," + String(hum, 1) + "," + String(humd, 0);
        // Biến mảng dữ liệu
        char dataArr[126];
        // Chuyển chuỗi dữ liệu sang mảng dữ liệu
        dataStr.toCharArray(dataArr, sizeof(dataArr));
        // Gửi dữ liệu LoRa
        manager.sendto((uint8_t *)dataArr, (uint8_t)sizeof(dataArr), from);
        // Chờ gửi LoRa thành công
        manager.waitPacketSent();
      }
    }
  }
  // Định thời với thời gian đặt trước
  unsigned long currMillis = millis();
  if (currMillis - prevMillis >= interval || prevMillis == 0) {
    prevMillis = currMillis;
    // Đọc giá trị nhiệt độ và độ ẩm
    hum = dht.readHumidity();
    temp = dht.readTemperature();
    // Đọc giá trị độ ẩm đất
    humd = ((1023 - analogRead(HUMD_PIN)) / 1023.0) * 100;
    // Hiển thị LCD
    lcd.setCursor(0, 0);
    lcd.print("T=");
    lcd.print(temp, 1);
    lcd.write(0xDF);
    lcd.print("  ");
    lcd.setCursor(9, 0);
    lcd.print("H=");
    lcd.print(hum, 1);
    lcd.print("%  ");
    lcd.setCursor(0, 1);
    lcd.print("HD=");
    lcd.print(humd, 1);
    lcd.print("%  ");
  }
}
