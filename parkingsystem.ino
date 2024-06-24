#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const int irSensorPin = D5;  // GPIO14 cho cảm biến IR
const int servoPin = D6;     // GPIO12 cho servo

Servo gateServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Địa chỉ I2C của màn hình LCD: 0x27, kích thước 16x2

const char *ssid = "HSU_Students";  // Thay bằng tên và mật khẩu WiFi của bạn
const char *password = "dhhs12cnvch";
const char *URL = "http://10.106.12.109:7777";  // Địa chỉ URL để gửi dữ liệu

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600, 60000); // GMT+7, cập nhật mỗi 60 giây

unsigned long doorCloseTime = 0;  // Biến để lưu thời điểm cửa được mở lần cuối

bool isDoorOpen = false;  // Biến để theo dõi trạng thái cửa (đóng hoặc mở)

WiFiClient client;
HTTPClient http;

void setup() {
  Serial.begin(115200);
  pinMode(irSensorPin, INPUT);

  // Khởi động servo
  gateServo.attach(servoPin);
  gateServo.write(0);  // Cửa đóng ban đầu

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
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Khởi động NTP
  timeClient.begin();
}

void loop() {
  timeClient.update(); // Cập nhật thời gian từ NTP
  checkDoorStatusFromServer(); // Kiểm tra trạng thái cửa từ server

  int sensorValue = digitalRead(irSensorPin);

  if (sensorValue == LOW) {
    // Có vật cản (xe vào/ra)
    lcd.setCursor(0, 1);
    lcd.print("Car detected!   ");

    // Mở cửa nếu chưa được mở
    if (!isDoorOpen) {
      gateServo.write(180);  // Mở cửa
      isDoorOpen = true;
      doorCloseTime = millis();  // Ghi lại thời điểm mở cửa

      // Gửi dữ liệu lên server khi mở cửa
      postJsonData(true);
    }
  } else {
    // Không có vật cản
    lcd.setCursor(0, 1);
    lcd.print("No car detected!");

    // Kiểm tra nếu cửa đang mở và đã quá 5 giây kể từ lần cửa được mở cuối cùng
    if (isDoorOpen && millis() - doorCloseTime >= 5000) {
      gateServo.write(0);  // Đóng cửa
      isDoorOpen = false;

      // Gửi dữ liệu lên server khi đóng cửa
      postJsonData(false);
    }
  }

  delay(500);  // Đợi một chút trước khi kiểm tra lại
}

String getFormattedDateTime() {
  time_t rawTime = timeClient.getEpochTime();
  struct tm * timeInfo = localtime(&rawTime);
  
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
    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, String(URL) + "/add")) {
      Serial.print("[HTTP] POST...\n");
      StaticJsonDocument<200> doc;
      doc["status"] = isDoorOpened ? "opened" : "closed";
      doc["atTime"] = getFormattedDateTime();
      
      char output[256];
      serializeJson(doc, output);
      http.addHeader("Content-Type", "application/json");
      int httpCode = http.POST(output);
      Serial.println(httpCode);
      http.end();
      Serial.println("closing connection");
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
          const char* action = doc["action"];
          if (strcmp(action, "open") == 0) {
            gateServo.write(180);  // Mở cửa
            isDoorOpen = true;
            doorCloseTime = millis();  // Ghi lại thời điểm mở cửa
          } else if (strcmp(action, "close") == 0) {
            gateServo.write(0);  // Đóng cửa
            isDoorOpen = false;
          }
        }
      }
      http.end();
    }
  }
}
