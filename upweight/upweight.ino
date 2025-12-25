// #include "HX711.h"
// #include <ESP8266WiFi.h>
// #include <ESP8266HTTPClient.h>
// #include <Wire.h>
// #include <LiquidCrystal_I2C.h>

// // ===== WiFi info =====
// const char* ssid = "<3";
// const char* password = "chibaosayhu";


// // ===== Flask server =====
// String serverName = "http://172.20.10.13:5000/weight";


// // ===== HX711 pins =====
// #define DOUT D5
// #define CLK D6
// HX711 scale;

// // ===== LCD (địa chỉ 0x27 hoặc 0x3F) =====
// LiquidCrystal_I2C lcd(0x27, 16, 2);

// // ======= Calibration =======
// float calibration_factor = 425.24;

// void setup() {
//   Serial.begin(115200);

//   // --- LCD ---
//   lcd.init();
//   lcd.backlight();
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   lcd.print("Connecting WiFi");

//   // --- Kết nối WiFi ---
//   WiFi.begin(ssid, password);
//   int retry = 0;
//   while (WiFi.status() != WL_CONNECTED && retry < 20) {
//     delay(500);
//     lcd.print(".");
//     Serial.print(".");
//     retry++;
//   }

//   lcd.clear();
//   if (WiFi.status() == WL_CONNECTED) {
//     Serial.println("\n✅ WiFi Connected!");
//     lcd.setCursor(0, 0);
//     lcd.print("WiFi Connected!");
//   } else {
//     Serial.println("\n❌ WiFi Failed!");
//     lcd.setCursor(0, 0);
//     lcd.print("WiFi Failed!");
//   }
//   delay(1000);
//   lcd.clear();

//   // --- Loadcell ---
//   scale.begin(DOUT, CLK);
//   scale.set_scale(calibration_factor);
//   scale.tare();
//   Serial.println("✅ Loadcell Ready");
//   lcd.setCursor(0, 0);
//   lcd.print("Loadcell Ready");
//   delay(1000);
//   lcd.clear();
// }

// void loop() {
//   if (WiFi.status() == WL_CONNECTED) {
//     WiFiClient client;
//     HTTPClient http;

//     if (scale.is_ready()) {
//       float weight = scale.get_units(3)/1000;
//       if (abs(weight) < 0.05) weight = 0;

//       // --- Hiển thị LCD (mượt, xóa dòng cũ trước khi in) ---
//       lcd.setCursor(0, 0);
//       lcd.print("Weight:        "); 
//       lcd.setCursor(0, 1);
//       lcd.print("                ");
//       lcd.setCursor(0, 1);
//       lcd.print(abs(weight), 2);
//       lcd.print(" kg");

//       // --- Serial log ---
//       Serial.print("⚖️  Weight: ");
//       Serial.println(abs(weight), 2);

//       // --- Gửi Flask ---
//       http.begin(client, serverName);
//       http.addHeader("Content-Type", "application/json");
//       String payload = "{\"weight\":" + String(abs(weight), 2) + "}";
//       int httpResponseCode = http.POST(payload);

//       if (httpResponseCode > 0) {
//         Serial.print("📤 HTTP code: ");
//         Serial.println(httpResponseCode);
//         lcd.setCursor(0, 0);
//         lcd.print("Sent to server ");
//       } else {
//         Serial.print("⚠️ Send error: ");
//         Serial.println(httpResponseCode);
//         lcd.setCursor(0, 0);
//         lcd.print("Send error!    ");
//       }

//       http.end();
//     } else {
//       lcd.clear();
//       lcd.setCursor(0, 0);
//       lcd.print("HX711 Error!");
//       Serial.println("❌ HX711 not ready!");
//     }
//   } else {
//     lcd.clear();
//     lcd.setCursor(0, 0);
//     lcd.print("WiFi Lost!");
//     Serial.println("❌ WiFi Lost!");
//     WiFi.reconnect();
//   }

//   delay(200);
// }

#include "HX711.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>  // 📦 BẮT BUỘC: Cài thư viện ArduinoJson
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ================= WIFI =================
const char* ssid     = "NGOC HOA";
const char* password = "home1234";

// ================= SERVER BACKEND =================
String backendAPI = "https://fruitstore.loca.lt/files/latest-fruit";  // API lấy kết quả
String weightAPI = "http://192.168.1.14:5000/weight";  // Gửi cân lên Flask

// ================= HX711 =================
#define DOUT D5
#define CLK  D6
HX711 scale;
float calibration_factor = 425.24;

// ================= LCD =================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================= BIẾN TOÀN CỤC =================
float currentWeight = 0.0;
String fruitName = "Chờ...";
unsigned long lastWeightSend = 0;
unsigned long lastFruitCheck = 0;

// ================= TIMER =================
unsigned long tReadWeight = 0;
unsigned long tSendWeight = 0;
unsigned long tPullFruit = 0;

// ================= FRUIT TIMEOUT =================
unsigned long fruitWaitStart = 0;
bool fruitReceived = false;

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.print("Connecting WiFi");

  // WiFi
  WiFi.begin(ssid, password);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    lcd.print(".");
    retry++;
  }

  lcd.clear();
  if (WiFi.status() == WL_CONNECTED) {
    lcd.print("WiFi Connected");
    Serial.println("Connected to " + String(ssid));
  } else {
    lcd.print("WiFi Failed");
    while(1) delay(1000);
  }
  delay(1000);
  lcd.clear();

  // HX711
  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor);
  scale.tare();

  lcd.print("Loadcell Ready");
  delay(1000);
  lcd.clear();
  
  Serial.println("✅ System Ready!");
}

// ================= TASK 1: READ WEIGHT =================
void taskReadWeight() {
  if (millis() - tReadWeight < 200) return;
  tReadWeight = millis();

  if (scale.is_ready()) {
    float w = scale.get_units(3) / 1000.0;
    if (abs(w) < 0.05) w = 0;
    currentWeight = abs(w);
  }
}

// ================= TASK 2: SEND WEIGHT =================
void taskSendWeight() {
  if (millis() - tSendWeight < 500) return;
  tSendWeight = millis();

  WiFiClient client;
  HTTPClient http;

  http.begin(client, weightAPI);
  http.addHeader("Content-Type", "application/json");
  
  String payload = "{\"weight\":" + String(currentWeight, 2) + "}";
  int httpCode = http.POST(payload);
  
  if (httpCode == 200) {
    Serial.println("✅ Weight sent: " + String(currentWeight, 2) + " kg");
    
    // 🔹 reset fruit wait timer khi có cân mới
    fruitWaitStart = millis();
    fruitReceived = false;
    fruitName = "Detecting...";
  } else {
    Serial.println("❌ Weight send failed: " + String(httpCode));
  }
  
  http.end();
}

// ================= TASK 3: PULL FRUIT NAME FROM BACKEND =================
void taskPullFruit() {
  // ⏱ timeout 1s → show "None"
  if (!fruitReceived && millis() - fruitWaitStart > 1000) {
    fruitName = "None";
  }

  if (millis() - tPullFruit < 1500) return;  // Kiểm tra mỗi 1.5 giây
  tPullFruit = millis();

  WiFiClient client;
  HTTPClient http;

  http.begin(client, backendAPI);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String response = http.getString();
    Serial.println("📥 Backend response: " + response);

    // Phân tích JSON với ArduinoJson
    StaticJsonDocument<256> doc;  // Kích thước đủ cho JSON của bạn
    DeserializationError error = deserializeJson(doc, response);
    
    if (!error) {
      // 🔹 LẤY TÊN QUẢ TỪ JSON - CHÍNH XÁC THEO MẪU CỦA BẠN
      String status = doc["status"] | "error";
      String newFruit = doc["fruit_name"] | "unknown";
      
      if (status == "success" && newFruit != "unknown") {
        fruitName = newFruit;
        fruitReceived = true;
        Serial.println("🍎 Fruit detected: " + fruitName);
      } else {
        fruitName = "None";
        Serial.println("⚠️ No fruit detected or API error");
      }
    } else {
      Serial.println("❌ JSON parse error: " + String(error.c_str()));
      fruitName = "Parse Error";
    }
  } else {
    Serial.println("❌ Backend API error: " + String(httpCode));
    fruitName = "API Error";
  }
  
  http.end();
}

// ================= LCD UPDATE =================
void updateLCD() {
  lcd.setCursor(0, 0);
  lcd.print("Fruit: ");
  
  // Hiển thị tên quả (tối đa 9 ký tự để vừa LCD)
  String displayFruit = fruitName;
  if (displayFruit.length() > 9) {
    displayFruit = displayFruit.substring(0, 9);
  }
  lcd.print(displayFruit);
  
  // Xóa phần còn lại của dòng
  for (int i = 7 + displayFruit.length(); i < 16; i++) {
    lcd.print(" ");
  }
  
  lcd.setCursor(0, 1);
  lcd.print("Weight: ");
  lcd.print(currentWeight, 2);
  lcd.print("kg");
  
  // Xóa phần còn lại của dòng
  int charsPrinted = 8 + String(currentWeight, 2).length() + 2;
  for (int i = charsPrinted; i < 16; i++) {
    lcd.print(" ");
  }
}

// ================= LOOP =================
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    lcd.clear();
    lcd.print("WiFi Lost!");
    WiFi.reconnect();
    delay(1000);
    return;
  }

  taskReadWeight();   // ⚡ Đọc cân liên tục
  taskSendWeight();   // ⏱ Gửi cân định kỳ
  taskPullFruit();    // 🔍 Lấy tên quả từ backend

  updateLCD();        // 📺 Cập nhật màn hình
  delay(50);          // Giảm tải CPU
}

// ================= HÀM BỔ TRỢ (Debug) =================
void debugInfo() {
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 5000) {  // In debug mỗi 5 giây
    lastDebug = millis();
    Serial.println("=== DEBUG INFO ===");
    Serial.println("Fruit: " + fruitName);
    Serial.println("Weight: " + String(currentWeight, 2) + " kg");
    Serial.println("WiFi: " + String(WiFi.RSSI()) + " dBm");
    Serial.println("==================");
  }
}