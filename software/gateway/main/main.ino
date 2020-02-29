// Thư viện LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Thư viện Wifi
#include "ESP8266WiFi.h"

// Thư viện Wifi Manager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

// Thự viện MQTT
#include <PubSubClient.h>

// Thư viện LoRa
#include <RHDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

// Định nghĩa nút cài đặt
#define SETTING_INPUT       0

// Định nghĩa điểm truy cập Wifi
#define AP_SSID             "IOT LoRa Smart Garden"
#define AP_PASS             "1234567890"

// Định nghĩa MQTT
#define MQTT_SERVER         "broker.hivemq.com"
#define MQTT_SERVERPORT     1883

// Định nghĩa chủ đề MQTT
#define TOPIC_PUBLISH       "phatvu1294/iot-lora-smart-garden/publish"
#define TOPIC_SUBSCRIBE     "phatvu1294/iot-lora-smart-garden/subscribe"

// Định nghĩa địa chỉ LoRa
#define GATEWAY_ADDR        100
#define NODE1_ADDR          101
#define NODE2_ADDR          102

// LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

// MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// LoRa
RH_RF95 driver(16, 15);
RHDatagram manager(driver, GATEWAY_ADDR);
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

// Biến đếm Node LoRa
int nodeCounter = 0;

// Biến định thời
unsigned long prevMillis = 0, interval = 5000;

// Hàm khởi tạo
void setup() {
    // Điều hướng vào ra
    pinMode(SETTING_INPUT, INPUT);

    // Khởi tạo Serial
    Serial.begin(9600);
    while (!Serial);

    // Khởi tạo LCD
    lcd.begin();

    // Hiển thị đang khởi động
    Serial.println(); Serial.println(); Serial.println();
    Serial.println("> IOT LoRa Smart Garden dang khoi dong...");
    Serial.println("> Neu muon cau hinh WiFi hay nhan nut cai dat");
    Serial.println("=============================================");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Dang k.dong...");

    // Trong vòng 2 giây nếu nhấn nút thì vào chế độ cấu hình
    for (int i = 0; i < 2000; i++) {
        delay(1);
        if (digitalRead(SETTING_INPUT) == LOW) {
            delay(20);
            if (digitalRead(SETTING_INPUT) == LOW) {
                // Hiển thị chế độ cấu hình WiFi
                Serial.println("> Dang o che do cau hinh WiFi");
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Che do cau hinh");
                lcd.setCursor(0, 1);
                lcd.print("WiFi");
                // Cấu hình WiFi với WiFi Manager
                WiFiManager wifiManager;
                wifiManager.resetSettings();
                if (!wifiManager.startConfigPortal(AP_SSID, AP_PASS)) {
                    delay(3000);
                    ESP.reset();
                    delay(5000);
                }
            }
        }
    }

    // Kết nối tới Wifi
    WifiConnect();
    
    // Đặt các thông số MQTT
    client.setServer(MQTT_SERVER, MQTT_SERVERPORT);
    
    // Khởi tạo LoRa
    manager.init();
    driver.setModemConfig(RH_RF95::Bw125Cr48Sf4096);
    driver.setTxPower(23);
}

// Hàm lặp vô hạn
void loop() {
    // Nếu MQTT bị mất kết nối
    if (!client.connected()) {
        // Kết nối tới MQTT
        MQTTConnect();
    }
    
    // Lặp MQTT
    client.loop();
    
    // Nếu LoRa sẵn sàng nhận dữ liệu
    if (manager.available()) {
        // Biến đọc dữ liệu LoRa
        uint8_t len = sizeof(buf);
        uint8_t from;
        // Nếu nhận được dữ liệu LoRa
        if (manager.recvfrom(buf, &len, &from)) {
            // Khởi tạo chuỗi dữ liệu bộ đệm
            String bufStr = (char *)buf;
            
            // In chuỗi nhận được
            Serial.println(bufStr);

            // Gửi dữ liệu nhận được từ LoRa tới MQTT
            client.publish(TOPIC_PUBLISH, bufStr.c_str(), true);
        }
    }
    
    // Định thời với thời gian đặt trước
    unsigned long currMillis = millis();
    if (currMillis - prevMillis >= interval || prevMillis == 0) {
        prevMillis = currMillis;
        // Biến địa chỉ LoRa và dữ liệu gửi đi
        uint8_t nodeAddr = 0;
        uint8_t nodeReq[] = "REQ";
        // Nếu tăng bộ đếm LoRa vượt quá số lượng
        if (++nodeCounter > 1) {
            // Đặt lại bộ đếm
            nodeCounter = 0;
        }
        
        // Nếu bộ đếm LoRa bằng 0
        if (nodeCounter == 0) {
            // Địa chỉ LoRa là Node 1
            nodeAddr = NODE1_ADDR;
            // Nếu bộ đếm LoRa bằng 1
        } else if (nodeCounter == 1) {
            // Địa chỉ LoRa là Node 2
            nodeAddr = NODE2_ADDR;
            // Nếu bộ đếm LoRa bằng 2
        }
        // Gửi dữ liệu LoRa
        manager.sendto(nodeReq, sizeof(nodeReq), (uint8_t)nodeAddr);
    }
}

// Hàm kết nối Wifi
void WifiConnect() {
    delay(10);
    // Hiển thị đang kết nối đến WiFi
    Serial.print("> Dang ket noi toi ");
    Serial.print(WiFi.SSID().c_str());
    Serial.println("...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Dang ket noi toi");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.SSID().c_str());
    lcd.print("...");
    // Kết nối WiFi cấu hình từ EEPROM
    WiFi.begin(WiFi.SSID().c_str(), WiFi.psk().c_str());
    // Trong khi WiFi chưa được kết nối
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    // Hiển thị đã kết nối WiFi
    Serial.print("> ");
    Serial.print(WiFi.SSID().c_str());
    Serial.println(" da duoc ket noi");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(WiFi.SSID().c_str());
    lcd.setCursor(0, 1);
    lcd.print("da duoc ket noi");
    delay(1000);
    // Hiển thị địa chỉ IP
    Serial.print("> Dia chi IP: ");
    Serial.println(WiFi.localIP());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Dia chi IP:");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    // Sau 2 giây xoá màn hình
    delay(2000);
    lcd.clear();
    // Random
    randomSeed(micros());
}

// Hàm kết nối MQTT
void MQTTConnect() {
    // Hiển thị đang kết nối đến Broker MQTT
    Serial.println("> Dang ket noi toi Broker MQTT...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Dang ket noi toi");
    lcd.setCursor(0, 1);
    lcd.print("Broker MQTT...  ");
    // Hiển thị thông tin đang kết nối MQTT
    while (!client.connected()) {
        // Khởi tạo chuỗi ID
        String clientId = "iot-lora-smart-garden-";
        // Tạo chuỗi ID ngẫu nhiên
        clientId += String(random(0xffff), HEX);
        // Nếu MQTT kết nối thành công
        if (client.connect(clientId.c_str())) {
            // Hiển thị đã kết nối WiFi
            Serial.println("> Broker MQTT da duoc ket noi");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Broker MQTT...  ");
            lcd.setCursor(0, 1);
            lcd.print("da duoc ket noi!");
            // Sau 1 giây xoá màn hình
            delay(1000);
            lcd.clear();
            // Đăng ký chủ đề MQTT
            client.subscribe(TOPIC_SUBSCRIBE);
        } else { // Ngược lại
            delay(5000);
        }
    }
}
