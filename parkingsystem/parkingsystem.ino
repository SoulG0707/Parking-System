#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const int irSensorPin = D5; // GPIO14 cho cảm biến IR
const int servoPin = D6;    // GPIO12 cho servo

Servo gateServo;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Địa chỉ I2C của màn hình LCD: 0x27, kích thước 16x2

const char *ssid = "HongBuiHouse Tret"; // Thay bằng tên và mật khẩu WiFi của bạn
const char *password = "@66668888";
const char *URL = "http://192.168.1.21:7777"; // Địa chỉ URL để gửi dữ liệu

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600, 60000); // GMT+7, cập nhật mỗi 60 giây

unsigned long lastDetectionTime = 0; // Biến để lưu thời điểm cảm biến phát hiện xe cuối cùng
unsigned long doorOpenTime = 0;      // Biến để lưu thời điểm cửa được mở lần cuối

bool isDoorOpen = false; // Biến để theo dõi trạng thái cửa (đóng hoặc mở)
bool serverRequestOpen = false; // Biến để lưu trạng thái mở cửa theo yêu cầu từ server

int lastSensorValue = HIGH; // Biến để lưu giá trị cảm biến IR trước đó

WiFiClient client;
HTTPClient http;

void setup() {
  Serial.begin(115200);
  pinMode(irSensorPin, INPUT);

  // Khởi động servo
  gateServo.attach(servoPin);
  gateServo.write(0); // Cửa đóng ban đầu

  // Khởi động LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Parking System");

  // Kết nối WiFi
  Serial.println();
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Bắt đầu NTP client
  timeClient.begin();
}

void loop() {
  timeClient.update();         // Cập nhật thời gian từ NTP
  checkDoorStatusFromServer(); // Kiểm tra trạng thái cửa từ server

  int sensorValue = digitalRead(irSensorPin);
  
  // Chỉ in ra giá trị cảm biến khi có thay đổi
  if (sensorValue != lastSensorValue) {
    Serial.print("Sensor Value: ");
    Serial.println(sensorValue);
    lastSensorValue = sensorValue;
  }

  if (sensorValue == LOW) {
    // Có vật cản (xe vào/ra)
    lcd.setCursor(0, 1);
    lcd.print("Car detected!   ");
    lastDetectionTime = millis();
    
    if (!isDoorOpen) {
      gateServo.write(180); // Mở cửa
      isDoorOpen = true;
      doorOpenTime = millis();
      postJsonData(true);
    }
  } else {
    // Không có vật cản
    lcd.setCursor(0, 1);
    lcd.print("No car detected!");
  }

  // Kiểm tra yêu cầu mở cửa từ server
  if (serverRequestOpen && !isDoorOpen) {
    gateServo.write(180); // Mở cửa
    isDoorOpen = true;
    doorOpenTime = millis();
    postJsonData(true);
  }

  // Kiểm tra nếu cửa đang mở và không có vật cản trong 5 giây
  if (isDoorOpen && (millis() - lastDetectionTime >= 5000) && (millis() - doorOpenTime >= 5000) && !serverRequestOpen) {
    gateServo.write(0); // Đóng cửa
    isDoorOpen = false;
    postJsonData(false);
  }

  delay(500); // Đợi một chút trước khi kiểm tra lại
}

String getFormattedDateTime() {
  time_t rawTime = timeClient.getEpochTime();
  struct tm *timeInfo = localtime(&rawTime);

  char buffer[20];
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
           timeInfo->tm_year + 1900,
           timeInfo->tm_mon + 1,
           timeInfo->tm_mday,
           timeInfo->tm_hour,
           timeInfo->tm_min,
           timeInfo->tm_sec);

  return String(buffer);
}

void postJsonData(bool isDoorOpened) {
  if (WiFi.status() == WL_CONNECTED) {
    if (http.begin(client, String(URL) + "/add")) {
      StaticJsonDocument<200> doc;
      doc["status"] = isDoorOpened ? "opened" : "closed";
      doc["atTime"] = getFormattedDateTime();
      char output[256];
      serializeJson(doc, output);
      http.addHeader("Content-Type", "application/json");
      int httpCode = http.POST(output);
      Serial.println(httpCode);
      http.end();
    }
  }
}

void checkDoorStatusFromServer() {
  if (WiFi.status() == WL_CONNECTED) {
    if (http.begin(client, String(URL) + "/door/status")) {
      int httpCode = http.GET();
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (!error) {
          const char *action = doc["action"];
          if (strcmp(action, "open") == 0) {
            serverRequestOpen = true;
          } else if (strcmp(action, "close") == 0) {
            serverRequestOpen = false;
          }
        }
      }
      http.end();
    }
  }
}
