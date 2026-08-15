/*
 * Smart Fish Tank - Arduino Uno (Final Optimized Version)
 * Pump via Relay + PWM LED + Sensors + Non-blocking Loop
 */

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Servo.h>

/* ========= إعدادات ========= */

// غيّرها حسب نوع الريليه عندك
#define RELAY_ACTIVE_LOW 0   
// 0 = Active HIGH
// 1 = Active LOW

/* =========================== */

// تعريف المنافذ
#define TEMP_PIN 2
#define TRIG_PIN 3
#define ECHO_PIN 4
#define PUMP_PIN 7
#define LED_PIN 5          // PWM حقيقي
#define SERVO_PIN 9
#define TURBIDITY_PIN A0

// الكائنات
OneWire oneWire(TEMP_PIN);
DallasTemperature tempSensor(&oneWire);
Servo feederServo;

// المتغيرات
float waterTemp = 0;
int waterLevel = 0;
int turbidity = 0;
int ledBrightness = 200;
bool pumpOn = false;

// إعدادات الحوض
const int TANK_HEIGHT = 20;
const int SENSOR_TO_TANK = 2;

// مؤقت غير حاجز (Non-blocking Timer)
unsigned long lastSensorRead = 0;
const long sensorInterval = 2000; // قراءة الحساسات كل ثانيتين

/* ===== دوال مساعدة للريليه ===== */
void pumpON() {
#if RELAY_ACTIVE_LOW
  digitalWrite(PUMP_PIN, LOW);
#else
  digitalWrite(PUMP_PIN, HIGH);
#endif
  pumpOn = true;
}

void pumpOFF() {
#if RELAY_ACTIVE_LOW
  digitalWrite(PUMP_PIN, HIGH);
#else
  digitalWrite(PUMP_PIN, LOW);
#endif
  pumpOn = false;
}
/* =============================== */

void setup() {
  Serial.begin(9600);

  tempSensor.begin();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  feederServo.attach(SERVO_PIN);
  feederServo.write(0);

  pumpOFF();                     // المضخة مطفية بالبداية
  analogWrite(LED_PIN, ledBrightness);

  Serial.println("Arduino Ready!");
}

void loop() {
  // تنفيذ قراءة الحساسات وإرسال البيانات كل ثانيتين دون إيقاف البرنامج
  unsigned long currentMillis = millis();
  if (currentMillis - lastSensorRead >= sensorInterval) {
    lastSensorRead = currentMillis;
    readSensors();
    sendDataToESP32();
  }

  // استقبال الأوامر والاستجابة لها بشكل فوري ولحظي
  receiveCommands();
}

/* ========= قراءة الحساسات ========= */
void readSensors() {

  tempSensor.requestTemperatures();
  waterTemp = tempSensor.getTempCByIndex(0);
  if (waterTemp < 0 || waterTemp == -127) waterTemp = 0;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  
  // حماية من انقطاع قراءة التراسونيك
  if (duration == 0) {
    waterLevel = 0; 
  } else {
    int distance = duration * 0.034 / 2;
    int waterHeight = TANK_HEIGHT - (distance - SENSOR_TO_TANK);
    waterHeight = constrain(waterHeight, 0, TANK_HEIGHT);
    waterLevel = map(waterHeight, 0, TANK_HEIGHT, 0, 100);
  }

  int turbRaw = analogRead(TURBIDITY_PIN);
  turbidity = map(turbRaw, 0, 1023, 100, 0);
}

/* ========= إرسال البيانات ========= */
void sendDataToESP32() {
  Serial.print(waterTemp, 1);
  Serial.print(",");
  Serial.print(waterLevel);
  Serial.print(",");
  Serial.print(turbidity);
  Serial.print(",");
  Serial.println(pumpOn ? "1" : "0");
}

/* ========= استقبال الأوامر ========= */
void receiveCommands() {
  if (Serial.available()) {

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "FEED") {
      feedFish();
    }

    else if (cmd == "PUMP_ON") {
      pumpON();
    }

    else if (cmd == "PUMP_OFF") {
      pumpOFF();
    }

    else if (cmd.startsWith("LED:")) {
      int value = cmd.substring(4).toInt();
      ledBrightness = constrain(value, 0, 255);
      analogWrite(LED_PIN, ledBrightness);
    }

    else if (cmd == "LED_OFF") {
      analogWrite(LED_PIN, 0);
    }
  }
}

/* ========= تغذية السمك ========= */
void feedFish() {
  feederServo.write(90);
  delay(1000);
  feederServo.write(0);
}