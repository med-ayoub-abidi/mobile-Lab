/*
 * Green Home - Test ESP32 -> API (valeur fixe, bla capteur)
 * Yeb3ath mesure kol 30 secondes l POST /api/measurements
 * Library : ArduinoJson (Benoit Blanchon)
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

// ====== BADDEL KEN EL WIFI ======
const char* WIFI_SSID     = "ISM_EL_WIFI";
const char* WIFI_PASSWORD = "PASSWORD_WIFI";
// ================================

const char* API_URL   = "http://10.233.34.105:8000/api/measurements";
const int   SENSOR_ID = 2;   // el capteur elli sna3tou fi Swagger

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.print(" OK, IP ESP32 = ");
  Serial.println(WiFi.localIP());

  configTime(3600, 0, "pool.ntp.org");   // wa9t Tunisie (UTC+1)
  struct tm t;
  while (!getLocalTime(&t)) { Serial.println("NTP..."); delay(500); }
  Serial.println("Wa9t OK");
}

void loop() {
  struct tm t;
  getLocalTime(&t);
  char now[32];
  strftime(now, sizeof(now), "%Y-%m-%dT%H:%M:%S+01:00", &t);

  JsonDocument doc;
  doc["sensorId"]   = SENSOR_ID;
  doc["value"]      = 20 + random(0, 100) / 10.0;   // valeur de test
  doc["measuredAt"] = now;

  String body;
  serializeJson(doc, body);

  HTTPClient http;
  http.begin(API_URL);
  http.addHeader("Content-Type", "application/ld+json");
  int code = http.POST(body);
  Serial.printf("%s -> HTTP %d\n", body.c_str(), code);
  if (code != 201) Serial.println(http.getString());
  http.end();

  delay(30000);   // kol 30 secondes
}
