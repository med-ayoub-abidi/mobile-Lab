/*
 * Green Home - ESP32 -> API (valeurs reelles des capteurs)
 * DHT11 (temperature + humidite) sur GPIO 18
 * CY-30 / BH1750 (luminosite) en I2C : SDA 21, SCL 22
 * Envoie les mesures toutes les 30 secondes vers POST /api/measurements
 * Libraries : ArduinoJson (Benoit Blanchon), DHT sensor library (Adafruit), BH1750 (Christopher Laws)
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <DHT.h>
#include <BH1750.h>
#include <time.h>

// ====== BADDEL KEN EL WIFI ======


// ID des capteurs fi base (lazem ykounou mawjoudin fi Swagger)
const int SENSOR_TEMPERATURE = 2;
const int SENSOR_HUMIDITE    = 3;
const int SENSOR_LUMINOSITE  = 4;

const unsigned long PERIODE_MS = 30000;   // kol 30 secondes

// ====== DHT11 ======
#define DHTPIN  18
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ====== CY-30 / BH1750 ======
#define SDA_PIN 21
#define SCL_PIN 22
BH1750 lightMeter;
bool luminositeDisponible = false;

unsigned long dernierMs = 0;

// ---------------------------------------------------------
void connecterWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.print(" OK, IP ESP32 = ");
  Serial.println(WiFi.localIP());
}

// ---------------------------------------------------------
bool envoyerMesure(const char* nom, int sensorId, float valeur) {
  struct tm t;
  if (!getLocalTime(&t)) {
    Serial.println("Erreur : heure NTP indisponible");
    return false;
  }
  char now[32];
  strftime(now, sizeof(now), "%Y-%m-%dT%H:%M:%S+01:00", &t);

  JsonDocument doc;
  doc["sensorId"]   = sensorId;
  doc["value"]      = roundf(valeur * 10) / 10.0;   // 1 decimale
  doc["measuredAt"] = now;

  String body;
  serializeJson(doc, body);

  HTTPClient http;
  http.setTimeout(5000);
  http.begin(API_URL);
  http.addHeader("Content-Type", "application/ld+json");
  http.addHeader("Accept", "application/ld+json");
  int code = http.POST(body);

  Serial.printf("[%s] %s -> HTTP %d\n", nom, body.c_str(), code);
  if (code != 201) {
    if (code > 0) Serial.println(http.getString());
    else          Serial.println(http.errorToString(code));
  }
  http.end();
  return code == 201;
}

// ---------------------------------------------------------
void lireEtEnvoyer() {
  // --- DHT11 ---
  float temperature = dht.readTemperature();
  float humidite    = dht.readHumidity();

  if (isnan(temperature) || isnan(humidite)) {
    Serial.println("Erreur lecture DHT11 ! (thabet fel cablage / GPIO 18)");
  } else {
    Serial.printf("DHT11 : %.1f C, %.1f %%\n", temperature, humidite);
    envoyerMesure("temperature", SENSOR_TEMPERATURE, temperature);
    delay(500);
    envoyerMesure("humidite", SENSOR_HUMIDITE, humidite);
  }

  // --- CY-30 / BH1750 ---
  if (!luminositeDisponible) {
    Serial.println("CY-30 non disponible");
    return;
  }
  float lux = lightMeter.readLightLevel();
  if (lux < 0) {
    Serial.println("Erreur lecture CY-30 !");
  } else {
    Serial.printf("CY-30 : %.1f lux\n", lux);
    delay(500);
    envoyerMesure("luminosite", SENSOR_LUMINOSITE, lux);
  }
}

// ---------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n===== DEMARRAGE ESP32 =====");

  dht.begin();
  Serial.println("DHT11 initialise sur GPIO 18");

  Wire.begin(SDA_PIN, SCL_PIN);
  luminositeDisponible = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
  Serial.println(luminositeDisponible ? "CY-30 initialise" : "ERREUR : CY-30 non detecte");

  connecterWiFi();

  configTime(3600, 0, "pool.ntp.org");   // wa9t Tunisie (UTC+1)
  struct tm t;
  while (!getLocalTime(&t)) { Serial.println("NTP..."); delay(500); }
  Serial.println("Wa9t OK");

  delay(2000);   // DHT11 y7eb chwaya wa9t ba3d el demarrage
}

// ---------------------------------------------------------
void loop() {
  if (dernierMs == 0 || millis() - dernierMs >= PERIODE_MS) {
    dernierMs = millis();

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi ma9tou3 ! Reconnexion...");
      WiFi.reconnect();
      return;
    }
    lireEtEnvoyer();
  }
}
