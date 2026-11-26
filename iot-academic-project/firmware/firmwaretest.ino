#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <DHT.h>
#include <BH1750.h>
#include <time.h>

// =====================================
// CONFIGURATION WIFI / API
// =====================================

const char* WIFI_SSID     = "Redmi Note 14";
const char* WIFI_PASSWORD = "mot_de_passe_wifi";

// IP du PC sur le WiFi + port Symfony (PAS de "/" avant ":8000")
const char* API_URL = "http://10.233.34.105:8000/api/measurements";

// Token de l'appareil si l'API l'exige, sinon laisser ""
const char* DEVICE_TOKEN = "";

// =====================================
// IDENTIFIANTS DES CAPTEURS (a adapter aux id en base)
// =====================================

const int SENSOR_TEMPERATURE = 2;
const int SENSOR_HUMIDITE    = 3;
const int SENSOR_LUMINOSITE  = 4;

const unsigned long PERIODE_MS = 30000;

// =====================================
// CONFIGURATION DHT11
// =====================================

#define DHTPIN 18
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// =====================================
// CONFIGURATION CY-30 / BH1750
// =====================================

#define SDA_PIN 21
#define SCL_PIN 22

BH1750 lightMeter;
bool luminositeDisponible = false;

// =====================================
// INITIALISATION
// =====================================

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n===== DEMARRAGE ESP32 =====");

  // DHT11
  dht.begin();
  Serial.println("DHT11 initialise sur GPIO 18");

  // I2C + CY-30
  Wire.begin(SDA_PIN, SCL_PIN);
  luminositeDisponible = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
  Serial.println(luminositeDisponible ? "CY-30 initialise avec succes"
                                      : "ERREUR : CY-30 non detecte");

  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connexion WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connecte !");
  Serial.print("IP ESP32 = ");
  Serial.println(WiFi.localIP());

  // NTP (Tunisie : UTC+1, pas d'heure d'ete)
  configTime(3600, 0, "pool.ntp.org");
  struct tm t;
  Serial.print("Synchronisation NTP");
  while (!getLocalTime(&t)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nHeure synchronisee !");
}

// =====================================
// ENVOI D'UNE MESURE VERS L'API
// =====================================

bool envoyerMesure(const char* nomCapteur, int sensorId, float valeur) {
  struct tm t;
  if (!getLocalTime(&t)) {
    Serial.println("Erreur : heure NTP indisponible");
    return false;
  }

  char now[32];
  strftime(now, sizeof(now), "%Y-%m-%dT%H:%M:%S+01:00", &t);

  JsonDocument doc;
  doc["sensorId"]   = sensorId;
  doc["value"]      = roundf(valeur * 10) / 10;  // 1 decimale
  doc["measuredAt"] = now;

  String body;
  serializeJson(doc, body);

  Serial.println("\n===== ENVOI MESURE =====");
  Serial.printf("Capteur : %s (id %d)\n", nomCapteur, sensorId);
  Serial.print("JSON : ");
  Serial.println(body);

  HTTPClient http;
  http.setTimeout(5000);
  http.begin(API_URL);
  http.addHeader("Content-Type", "application/ld+json");
  http.addHeader("Accept", "application/ld+json");
  if (strlen(DEVICE_TOKEN) > 0) {
    http.addHeader("X-Device-Token", DEVICE_TOKEN);
  }

  int code = http.POST(body);
  Serial.printf("HTTP Response : %d\n", code);
  if (code > 0) {
    Serial.println(http.getString());
  } else {
    Serial.println(http.errorToString(code));
  }
  http.end();

  return code == 201;
}

// =====================================
// LECTURE DES CAPTEURS
// =====================================

void lireEtEnvoyer() {
  // DHT11
  float temperature = dht.readTemperature();
  float humidite    = dht.readHumidity();

  if (!isnan(temperature) && !isnan(humidite)) {
    Serial.println("\n--- DHT11 ---");
    Serial.printf("Temperature : %.1f C\n", temperature);
    Serial.printf("Humidite : %.1f %%\n", humidite);

    envoyerMesure("temperature", SENSOR_TEMPERATURE, temperature);
    delay(500);
    envoyerMesure("humidity", SENSOR_HUMIDITE, humidite);
  } else {
    Serial.println("Erreur lecture DHT11 !");
  }

  // CY-30
  if (luminositeDisponible) {
    float luminosite = lightMeter.readLightLevel();
    if (luminosite >= 0) {
      Serial.println("\n--- CY-30 ---");
      Serial.printf("Luminosite : %.1f lux\n", luminosite);
      delay(500);
      envoyerMesure("light", SENSOR_LUMINOSITE, luminosite);
    } else {
      Serial.println("Erreur lecture CY-30 !");
    }
  } else {
    Serial.println("CY-30 non disponible");
  }
}

// =====================================
// LOOP
// =====================================

unsigned long dernierMs = 0;

void loop() {
  if (dernierMs == 0 || millis() - dernierMs >= PERIODE_MS) {
    dernierMs = millis();

    if (WiFi.status() == WL_CONNECTED) {
      lireEtEnvoyer();
    } else {
      Serial.println("WiFi deconnecte ! Reconnexion...");
      WiFi.reconnect();
    }
  }
}