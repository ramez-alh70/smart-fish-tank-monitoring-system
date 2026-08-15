/*
 * Smart Fish Tank - ESP32 (Final Stable Version)
 * Web API + UART to Arduino via Serial2
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>


const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

WebServer server(80);

// UART with Arduino (Serial2)
HardwareSerial UnoSerial(2);
const int UNO_RX_PIN = 16;  // ESP32 RX  <- Arduino TX (مع مقسم جهد)
const int UNO_TX_PIN = 17;  // ESP32 TX  -> Arduino RX

// المتغيرات
float currentTemp = 0;
int waterLevel = 0;
int turbidity = 0;
bool pumpOn = false;
bool systemLED = true;
int ledBrightness = 80;
unsigned long lastFeedTime = 0;

void setup() {
  // USB Debug
  Serial.begin(115200);

  // UART to Arduino
  UnoSerial.begin(9600, SERIAL_8N1, UNO_RX_PIN, UNO_TX_PIN);

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/data", HTTP_GET, handleGetData);
  server.on("/api/feed", HTTP_POST, handleFeed);
  server.on("/api/pump", HTTP_POST, handleTogglePump);
  server.on("/api/led", HTTP_POST, handleToggleLED);
  server.on("/api/brightness", HTTP_POST, handleBrightness);

  server.enableCORS(true);
  server.begin();

  Serial.println("Web Server Started!");
}

void loop() {
  server.handleClient();
  readFromArduino();
}

// ===== الصفحة الرئيسية =====
void handleRoot() {
  String html = "<h1>Smart Fish Tank</h1>";
  html += "<p>System Online</p>";
  html += "<p>IP: " + WiFi.localIP().toString() + "</p>";
  server.send(200, "text/html", html);
}

// ===== إرسال البيانات (JSON) =====
void handleGetData() {
  StaticJsonDocument<512> doc;

  doc["temperature"] = currentTemp;
  doc["waterLevel"]   = waterLevel;
  doc["turbidity"]    = turbidity;
  doc["pumpOn"]       = pumpOn;
  doc["systemLED"]    = systemLED;
  doc["ledBrightness"] = ledBrightness;
  doc["lastFeed"]     = getTimeSinceLastFeed();
  doc["status"]       = "online";

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

// ===== تغذية السمك =====
void handleFeed() {
  UnoSerial.println("FEED");   // <-- للـ Arduino
  lastFeedTime = millis();

  server.send(200, "application/json",
              "{\"success\":true,\"message\":\"Fish fed successfully!\"}");
}

// ===== تشغيل/إيقاف المضخة =====
void handleTogglePump() {
  pumpOn = !pumpOn;

  if (pumpOn) UnoSerial.println("PUMP_ON");
  else        UnoSerial.println("PUMP_OFF");

  String res = "{\"success\":true,\"pumpOn\":";
  res += pumpOn ? "true" : "false";
  res += ",\"message\":\"Pump ";
  res += pumpOn ? "started" : "stopped";
  res += "\"}";

  server.send(200, "application/json", res);
}

// ===== تشغيل/إيقاف LED =====
void handleToggleLED() {
  systemLED = !systemLED;

  if (systemLED) {
    int pwmValue = map(ledBrightness, 0, 100, 0, 255);
    UnoSerial.println("LED:" + String(pwmValue));
  } else {
    UnoSerial.println("LED_OFF");
  }

  String res = "{\"success\":true,\"systemLED\":";
  res += systemLED ? "true" : "false";
  res += "}";
  server.send(200, "application/json", res);
}

// ===== تغيير سطوع LED =====
void handleBrightness() {
  if (server.hasArg("value")) {
    ledBrightness = server.arg("value").toInt();
    ledBrightness = constrain(ledBrightness, 0, 100);

    if (systemLED) {
      int pwmValue = map(ledBrightness, 0, 100, 0, 255);
      UnoSerial.println("LED:" + String(pwmValue));
    }
  }

  server.send(200, "application/json",
              "{\"success\":true,\"brightness\":" + String(ledBrightness) + "}");
}

// ===== قراءة البيانات من Arduino =====
void readFromArduino() {
  while (UnoSerial.available()) {
    String data = UnoSerial.readStringUntil('\n');
    data.trim();

    // صيغة البيانات: temp,water,turbidity,pumpState
    int c1 = data.indexOf(',');
    int c2 = data.indexOf(',', c1 + 1);
    int c3 = data.indexOf(',', c2 + 1);

    if (c1 > 0 && c2 > 0 && c3 > 0) {
      currentTemp = data.substring(0, c1).toFloat();
      waterLevel   = data.substring(c1 + 1, c2).toInt();
      turbidity    = data.substring(c2 + 1, c3).toInt();
      pumpOn       = data.substring(c3 + 1).toInt() == 1;
    }
  }
}

// ===== حساب وقت آخر تغذية =====
String getTimeSinceLastFeed() {
  if (lastFeedTime == 0) return "Never";

  unsigned long diff = (millis() - lastFeedTime) / 1000;
  if (diff < 60) return "Just now";
  else if (diff < 3600) return String(diff / 60) + "m ago";
  else return String(diff / 3600) + "h ago";
}