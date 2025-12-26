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

// #include "HX711.h"
// #include <ESP8266WiFi.h>
// #include <ESP8266HTTPClient.h>
// #include <WiFiClientSecure.h>
// #include <ArduinoJson.h>
// #include <Wire.h>
// #include <LiquidCrystal_I2C.h>

// // ================= WIFI =================
// const char* ssid     = "NGOC HOA";
// const char* password = "home1234";

// // ================= API =================
// String weightAPI = "http://192.168.1.14:5000/weight";   // HTTP OK
// String fruitAPI  = "https://fruitstore.loca.lt/files/latest-fruit"; // HTTPS

// // ================= HX711 =================
// #define DOUT D5
// #define CLK  D6
// HX711 scale;
// float calibration_factor = 425.24;

// // ================= LCD =================
// LiquidCrystal_I2C lcd(0x27, 16, 2);

// // ================= GLOBAL =================
// float currentWeight = 0.0;
// String fruitName = "Waiting";

// // ================= TIMER =================
// unsigned long tReadWeight = 0;
// unsigned long tSendWeight = 0;
// unsigned long tPullFruit  = 0;

// // ================= FRUIT =================
// unsigned long fruitWaitStart = 0;
// bool fruitReceived = false;
// bool allowPullFruit = false;

// // ================= SETUP =================
// void setup() {
//   Serial.begin(115200);

//   lcd.init();
//   lcd.backlight();
//   lcd.print("Connecting WiFi");

//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     lcd.print(".");
//   }

//   lcd.clear();
//   lcd.print("WiFi Connected");
//   delay(1000);
//   lcd.clear();

//   scale.begin(DOUT, CLK);
//   scale.set_scale(calibration_factor);
//   scale.tare();

//   lcd.print("Loadcell Ready");
//   delay(1000);
//   lcd.clear();

//   Serial.println("✅ SYSTEM READY");
// }

// // ================= READ WEIGHT =================
// void taskReadWeight() {
//   if (millis() - tReadWeight < 200) return;
//   tReadWeight = millis();

//   if (scale.is_ready()) {
//     float w = scale.get_units(3) / 1000.0;
//     if (abs(w) < 0.05) w = 0;
//     currentWeight = abs(w);
//   }
// }

// // ================= SEND WEIGHT =================
// void taskSendWeight() {
//   if (millis() - tSendWeight < 500) return;
//   tSendWeight = millis();

//   WiFiClient client;
//   HTTPClient http;

//   http.begin(client, weightAPI);
//   http.addHeader("Content-Type", "application/json");

//   String payload = "{\"weight\":" + String(currentWeight, 2) + "}";
//   int code = http.POST(payload);
//   http.end();

//   if (code == 200) {
//     Serial.println("📤 Weight sent");

//     fruitWaitStart = millis();
//     fruitReceived = false;
//     allowPullFruit = true;
//     fruitName = "Detecting";
//   }
// }

// // ================= PULL FRUIT (HTTPS) =================
// void taskPullFruit() {
//   if (!allowPullFruit) return;

//   // timeout 1s
//   if (!fruitReceived && millis() - fruitWaitStart > 1000) {
//     fruitName = "None";
//     allowPullFruit = false;
//     return;
//   }

//   if (millis() - tPullFruit < 800) return;
//   tPullFruit = millis();

//   WiFiClientSecure client;
//   client.setInsecure();   // 🔥 BẮT BUỘC CHO HTTPS

//   HTTPClient http;
//   http.begin(client, fruitAPI);

//   int code = http.GET();
//   Serial.println("🌐 HTTPS CODE: " + String(code));

//   if (code == 200) {
//     String res = http.getString();
//     Serial.println("📥 " + res);

//     StaticJsonDocument<256> doc;
//     if (deserializeJson(doc, res) == DeserializationError::Ok) {
//       if (String(doc["status"]) == "success") {
//         fruitName = doc["fruit_name"].as<String>();
//         fruitReceived = true;
//         allowPullFruit = false;
//         Serial.println("🍎 Fruit:" + fruitName);
//       }
//     }
//   }

//   http.end();
// }

// // ================= LCD =================
// void updateLCD() {
//   lcd.setCursor(0, 0);
//   lcd.print("Fruit:");
//   lcd.print(fruitName);
//   lcd.print("        ");

//   lcd.setCursor(0, 1);
//   lcd.print("Weight: ");
//   lcd.print(currentWeight, 2);
//   lcd.print("kg   ");
// }

// // ================= LOOP =================
// void loop() {
//   if (WiFi.status() != WL_CONNECTED) {
//     lcd.clear();
//     lcd.print("WiFi Lost");
//     WiFi.reconnect();
//     delay(1000);
//     return;
//   }

//   taskReadWeight();
//   taskSendWeight();

//   taskPullFruit();
   
//    updateLCD();

//   delay(30);
// }



#include "HX711.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ================= WIFI =================
const char* ssid     = "<3";
const char* password = "chibaosayhu";

// ================= API =================
String weightAPI = "http://172.20.10.13:5000/weight";
String fruitAPI  = "https://optics-habitat-wireless-longitude.trycloudflare.com/files/latest-fruit";

// ================= HX711 =================
#define DOUT D5
#define CLK  D6
HX711 scale;
float calibration_factor = 425.24;

// ================= LCD =================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================= GLOBAL =================
float currentWeight = 0.0;
String fruitName = "None";

// ================= TIMER =================
unsigned long tReadWeight = 0;
unsigned long tSendWeight = 0;
unsigned long tPullFruit  = 0;

// ================= CONTROL =================
int weightSendCount = 0;
bool allowPullFruit = false;

#define WEIGHT_THRESHOLD 0.05

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.print("Connecting WiFi");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    lcd.print(".");
  }

  lcd.clear();
  lcd.print("WiFi Connected");
  delay(800);
  lcd.clear();

  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor);
  scale.tare();

  lcd.print("Loadcell Ready");
  delay(800);
  lcd.clear();

  Serial.println("✅ SYSTEM READY");
}

// ================= READ WEIGHT =================
void taskReadWeight() {
  if (millis() - tReadWeight < 150) return;
  tReadWeight = millis();

  if (!scale.is_ready()) return;

  float w = scale.get_units(3) / 1000.0;
  if (abs(w) < WEIGHT_THRESHOLD) w = 0;
  currentWeight = abs(w);

  // ===== KHÔNG CÓ VẬT =====
  if (currentWeight == 0) {
    fruitName = "None";
    weightSendCount = 0;
    allowPullFruit = false;
  }
}

// ================= SEND WEIGHT =================
void taskSendWeight() {
  if (currentWeight <= WEIGHT_THRESHOLD) return;
  if (millis() - tSendWeight < 400) return;
  tSendWeight = millis();

  WiFiClient client;
  HTTPClient http;

  http.begin(client, weightAPI);
  http.addHeader("Content-Type", "application/json");

  String payload = "{\"weight\":" + String(currentWeight, 2) + "}";
  int code = http.POST(payload);
  http.end();

  if (code == 200) {
    weightSendCount++;
    Serial.println("📤 Weight sent (" + String(weightSendCount) + ")");

    // ✅ ĐỦ 3 LẦN → CHỜ FRUIT MỚI
    if (weightSendCount >= 2) {
      allowPullFruit = true;
      weightSendCount = 0;

      // 🔥 QUAN TRỌNG: HIỂN THỊ WAITING (KHÔNG GIỮ FRUIT CŨ)
      fruitName = "Waiting";
    }
  }
}

// ================= PULL FRUIT =================
void taskPullFruit() {
  if (!allowPullFruit) return;
  if (millis() - tPullFruit < 800) return;
  tPullFruit = millis();

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, fruitAPI);

  int code = http.GET();
  Serial.println("🌐 HTTPS CODE: " + String(code));

  if (code == 200) {
    String res = http.getString();
    StaticJsonDocument<256> doc;

    if (deserializeJson(doc, res) == DeserializationError::Ok) {
      if (String(doc["status"]) == "success") {
        fruitName = doc["fruit_name"].as<String>();
        Serial.println("🍎 Fruit: " + fruitName);
      }
    }
  }

  http.end();
  allowPullFruit = false;   // pull xong thì tắt
}

// ================= LCD =================
void updateLCD() {
  lcd.setCursor(0, 0);
  lcd.print("Fruit:");
  lcd.print(fruitName);
  lcd.print("        ");

  lcd.setCursor(0, 1);
  lcd.print("Weight:");
  lcd.print(currentWeight, 2);
  lcd.print("kg   ");
}

// ================= LOOP =================
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    lcd.clear();
    lcd.print("WiFi Lost");
    WiFi.reconnect();
    delay(500);
    return;
  }

  taskReadWeight();
  taskSendWeight();
  taskPullFruit();
  updateLCD();

  yield();   // 🔥 BẮT BUỘC cho ESP8266
}
