#include <esp32cam.h>
#include <WiFi.h>
#include <WebServer.h>

// const char* WIFI_SSID = "<3";
// const char* WIFI_PASS = "chibaosayhu";
const char* WIFI_SSID = "NGOC HOA";
const char* WIFI_PASS = "home1234";

WebServer server(80);

// Không còn .value()
static const auto loRes  = esp32cam::Resolution::find(320, 240);
static const auto midRes = esp32cam::Resolution::find(350, 530);
static const auto hiRes = esp32cam::Resolution::find(1024, 768);


void serveJpg() {
  auto frame = esp32cam::capture();
  if (!frame) {
    Serial.println("❌ CAPTURE FAIL");
    server.send(503, "text/plain", "Capture failed");
    return;
  }

  Serial.printf("✅ CAPTURE OK %dx%d %d bytes\n",
                frame->getWidth(),
                frame->getHeight(),
                static_cast<int>(frame->size()));

  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);
}

void handleJpgLo() {
  if (!esp32cam::Camera.changeResolution(loRes))
    Serial.println("⚠️ SET-LO-RES FAIL");
  serveJpg();
}

void handleJpgMid() {
  if (!esp32cam::Camera.changeResolution(midRes))
    Serial.println("⚠️ SET-MID-RES FAIL");
  serveJpg();
}

void handleJpgHi() {
  if (!esp32cam::Camera.changeResolution(hiRes))
    Serial.println("⚠️ SET-HI-RES FAIL");
  serveJpg();
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  using namespace esp32cam;
  Config cfg;
  cfg.setPins(pins::AiThinker);
  cfg.setResolution(hiRes);
  cfg.setBufferCount(3);
  cfg.setJpeg(90);

  if (!Camera.begin(cfg)) {
    Serial.println("❌ CAMERA INIT FAIL");
    for (;;);
  }
  Serial.println("✅ CAMERA OK");

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("✅ WiFi connected, IP: ");
  Serial.println(WiFi.localIP());

  server.on("/cam-lo.jpg",  handleJpgLo);
  server.on("/cam-mid.jpg", handleJpgMid);
  server.on("/cam-hi.jpg",  handleJpgHi);
  server.begin();

  Serial.println("Endpoints:");
  Serial.println("  /cam-lo.jpg");
  Serial.println("  /cam-mid.jpg");
  Serial.println("  /cam-hi.jpg");
}

void loop() {
  server.handleClient();
}

// #include <esp32cam.h>
// #include <WiFi.h>
// #include <HTTPClient.h>

// const char* WIFI_SSID = "<3";
// const char* WIFI_PASS = "chibaosayhu";

// // Flask server
// const char* FLASK_URL = "http://172.20.10.3:5000/upload";

// static const auto hiRes = esp32cam::Resolution::find(1024, 768);

// unsigned long lastShot = 0;
// const unsigned long interval = 2000; // 5s

// void setup() {
//   Serial.begin(115200);

//   using namespace esp32cam;
//   Config cfg;
//   cfg.setPins(pins::AiThinker);
//   cfg.setResolution(hiRes);
//   cfg.setJpeg(85);
//   cfg.setBufferCount(2);

//   if (!Camera.begin(cfg)) {
//     Serial.println("❌ Camera fail");
//     while (true);
//   }

//   WiFi.begin(WIFI_SSID, WIFI_PASS);
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("\n✅ WiFi connected");
// }

// void loop() {
//   if (millis() - lastShot > interval) {
//     lastShot = millis();
//     sendSnapshot();
//   }
// }

// void sendSnapshot() {
//   auto frame = esp32cam::capture();
//   if (!frame) {
//     Serial.println("❌ Capture fail");
//     return;
//   }

//   HTTPClient http;
//   WiFiClient client;

//   http.begin(client, FLASK_URL);
//   http.addHeader("Content-Type", "image/jpeg");

//   // ⚠️ CÁCH ĐÚNG: POST buffer
//   int httpCode = http.POST((uint8_t*)frame->data(), frame->size());

//   Serial.printf("📤 Sent %d bytes → HTTP %d\n",
//                 frame->size(), httpCode);

//   http.end();
// }
