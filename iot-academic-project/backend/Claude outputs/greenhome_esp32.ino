/*
 * Green Home - ESP32 -> Symfony API Platform
 * Yeb3ath mesure kol SEND_INTERVAL_MS b POST /api/measurements
 *
 * Bibliothèques (Arduino IDE > Library Manager) :
 *   - ArduinoJson (Benoit Blanchon)
 *   - DHT sensor library (Adafruit)  [ken testa3mel DHT22]
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "DHT.h"

// ====== A BADDEL ======
const char* WIFI_SSID     = "ISM_EL_WIFI";
const char* WIFI_PASSWORD = "PASSWORD_WIFI";
const char* API_URL       = "http://192.168.1.15:8000/api/measurements"; // IP mta3 el PC

const int SENSOR_ID_TEMP  = 1;   // id mta3 sensor "temperature" fel base
const int SENSOR_ID_HUM   = 2;   // id mta3 sensor "air_humidity" fel base

const unsigned long SEND_INTERVAL_MS = 30000; // kol 30 secondes

#define DHTPIN  4
#define DHTTYPE DHT22
// ======================

DHT dht(DHTPIN, DHTTYPE);
unsigned long lastSend = 0;

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print(" OK, IP = ");
  Serial.println(WiFi.localIP());
}

// Wa9t men Internet (NTP) bech nab3thou measuredAt
void syncTime() {
  configTime(3600, 0, "pool.ntp.org", "time.google.com"); // Tunisie UTC+1
  struct tm t;
  while (!getLocalTime(&t)) {
    Serial.println("Nestanna NTP...");
    delay(500);
  }
  Serial.println("Wa9t OK");
}

// ex : 2026-11-25T19:50:00+01:00
String isoNow() {
  struct tm t;
  getLocalTime(&t);
  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S+01:00", &t);
  return String(buf);
}

bool sendMeasurement(int sensorId, float value) {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();

  JsonDocument doc;
  doc["sensorId"]   = sensorId;   // nafs esm el champ elli khdem fi Swagger
  doc["value"]      = value;
  doc["measuredAt"] = isoNow();

  String body;
  serializeJson(doc, body);

  HTTPClient http;
  http.begin(API_URL);
  http.addHeader("Content-Type", "application/ld+json");
  int code = http.POST(body);

  Serial.printf("POST sensor %d = %.2f -> HTTP %d\n", sensorId, value, code);
  if (code != 201) Serial.println(http.getString());

  http.end();
  return code == 201;
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  connectWiFi();
  syncTime();
}

void loop() {
  if (millis() - lastSend >= SEND_INTERVAL_MS) {
    lastSend = millis();

    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();

    if (isnan(temp) || isnan(hum)) {
      Serial.println("Erreur lecture DHT22");
      return;
    }

    sendMeasurement(SENSOR_ID_TEMP, temp);
    delay(1100); // measuredAt mokhtalef (UNIQUE sensor_id + measured_at)
    sendMeasurement(SENSOR_ID_HUM, hum);
  }
}
