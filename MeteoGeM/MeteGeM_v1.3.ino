// Po zapnutí sa ESP pripojí na WiFi.
// Následne inicializuje BME280 senzor (teplota, vlhkosť, tlak).
// Ak senzor nie je nájdený, ESP sa reštartuje.
// Ak je senzor pripravený, ESP sa pripojí na MQTT broker (nastavený na NAS).
// Po úspešnom pripojení ESP zmeria aktuálne hodnoty zo senzora a odosiela ich na MQTT topic-y.
// Pridané: Odosiela aj čas vyčítania údajov s podporou letného/zimného času.
// Následne vyšle signál DONE (napr. pre spínanie napájania watchdogom) a ukončí činnosť.
// Ak niečo nefunguje (WiFi, BME280, MQTT), ESP sa reštartuje pre opätovnú snahu.


// ========== Knižnice ==========
#include <Wire.h>                 // Knižnica pre I2C komunikáciu
#include <Adafruit_Sensor.h>     // Knižnica základných senzorov Adafruit
#include <Adafruit_BME280.h>     // Knižnica pre BME280 senzor (teplota, vlhkosť, tlak)
#include <ESP8266WiFi.h>         // Knižnica pre WiFi na ESP8266
#include <PubSubClient.h>        // Knižnica pre MQTT protokol
#include <NTPClient.h>           // Knižnica pre získavanie aktuálneho času z internetu (NTP)
#include <WiFiUdp.h>             // UDP spojenie pre NTPClient
#include <Timezone.h>            // Knižnica pre automatické prepínanie na letný/zimný čas

// ========== Konštanta pre tlak na hladine mora ==========
#define SEALEVELPRESSURE_HPA (1013.25)  // Používa sa na korekciu meraného tlaku na hladinu mora

// ========== WiFi konfigurácia ==========
const char* ssid = "";              // Názov WiFi siete (SSID)
const char* password = "";    // Heslo k WiFi sieti

//const char* ssid = ";              // Názov WiFi siete (SSID)
//const char* password = ";    // Heslo k WiFi sieti

// ========== MQTT konfigurácia ==========
const char* mqtt_server = "";  // IP adresa MQTT brokera (napr. NAS server)
const int mqtt_port = 1883;                   // Port, na ktorom MQTT broker beží (štandard je 1883)
// MQTT topic-y, kam budeme posielať dáta
const char* mqtt_topic_temp = "meteostanica/teplota";
const char* mqtt_topic_humi = "meteostanica/vlhkost";
const char* mqtt_topic_pres = "meteostanica/tlak";
const char* mqtt_topic_vbat = "meteostanica/napatie";
const char* mqtt_topic_time = "meteostanica/cas";  // Nový topic pre odosielanie času merania

// ========== Globálne objekty ==========
WiFiClient espClient;             // WiFi klient (pre sieťovú komunikáciu)
PubSubClient client(espClient);   // MQTT klient, ktorý používa WiFi klienta
Adafruit_BME280 bme;              // Objekt pre BME280 senzor
WiFiUDP ntpUDP;                   // UDP klient pre NTP komunikáciu
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);  // NTP klient (čas v UTC, aktualizuje každú minútu)

// ========== Letný/zimný čas ==========
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};  // CEST = Central European Summer Time (UTC+2)
TimeChangeRule CET  = {"CET", Last, Sun, Oct, 3, 60};    // CET = Central European Time (UTC+1)
Timezone CE(CEST, CET);  // Časová zóna pre Slovensko/ČR

// ========== Definícia pinov ==========
#define DONE_PIN D6             // Pin D6 (GPIO12) použitý pre signalizáciu DONE (napr. pre watchdog TPL5110)

// ========== Watchdog Timeout ==========
// Nastavenie watchdog časovača na 8000 ms (8 sekúnd)
// Ak program "zamrzne" dlhšie ako toto, ESP sa reštartuje
#define WATCHDOG_TIMEOUT 8000

// ========== Setup funkcia ==========
void setup() {
  pinMode(DONE_PIN, OUTPUT);         // Nastav DONE_PIN ako výstupný pin
  digitalWrite(DONE_PIN, LOW);       // Na začiatku je DONE pin LOW (vypnutý signál)

  Serial.begin(115200);              // Inicializuj sériovú komunikáciu pre debug cez USB na 115200 baud
  delay(100);                        // krátka pauza

  ESP.wdtEnable(WATCHDOG_TIMEOUT);   // Aktivuj watchdog s timeoutom 8 sekúnd

  Wire.begin(D2, D1);                // Inicializuj I2C zbernicu s pinmi SDA = D2 a SCL = D1

  connectWiFi();                     // Pripoj sa na WiFi sieť

  // Inicializuj BME280 senzor na adrese 0x76
  // Ak senzor nenájdeš, vypíš chybu a reštartuj ESP
  if (!bme.begin(0x76)) {
    Serial.println("Nenajdeny BME280 sensor!");
    delay(1000);
    ESP.restart();
  }

  timeClient.begin();               // Spusť NTP klienta
  timeClient.update();              // Získaj aktuálny UTC čas

  connectMQTT();                    // Pripoj sa na MQTT broker

  sendData();                       // Odosli merané dáta na MQTT

  // Posli signál na DONE pin (ak máš watchdog TPL5110, ukončí napájanie)
  Serial.println("Odosielam DONE signál...");
  digitalWrite(DONE_PIN, HIGH);
  delay(100);
  pinMode(D5, INPUT);
  //digitalWrite(DONE_PIN, LOW);

  Serial.println("Vypnutie...");
}

// ========== Funkcia na pripojenie WiFi ==========
void connectWiFi() {
  Serial.print("Pripajanie na WiFi: ");
  Serial.println(ssid);

  // Spusti pripojenie na WiFi so zadanými parametrami
  WiFi.begin(ssid, password);

  int attempts = 0;
  // Čakaj na pripojenie max 20x po 500 ms
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  // Ak sme pripojení, vypíš IP adresu zariadenia
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi pripojene!");
    Serial.print("IP adresa zariadenia: ");
    Serial.println(WiFi.localIP());
  } else {
    // Ak sa nepripojilo, vypíš chybu a reštartuj
    Serial.println("\nWiFi sa nepripojilo! Restartujem...");
    delay(1000);
    ESP.restart();
  }
}

// ========== Funkcia na pripojenie k MQTT brokeru ==========
void connectMQTT() {
  Serial.print("Pripajanie na MQTT broker: ");
  Serial.println(mqtt_server);

  // Nastav MQTT server a port
  client.setServer(mqtt_server, mqtt_port);

  int attempts = 0;
  // Skús sa pripojiť max 10-krát
  while (!client.connected() && attempts < 10) {
    Serial.print("Pokus o pripojenie MQTT (#");
    Serial.print(attempts + 1);
    Serial.println(") ...");

    // Pokús sa pripojiť s identifikátorom "ESP8266Client"
    if (client.connect("ESP8266Client")) {
      Serial.println("MQTT pripojenie uspesne!");
      client.loop();  // Inicializuj MQTT loop
      break;
    } else {
      // Ak pripojenie zlyhalo, vypíš chybu s kódom stavu
      Serial.print("MQTT zlyhalo, rc=");
      Serial.println(client.state());
      delay(1000);
      attempts++;
    }
  }

  // Ak sa nedarí pripojiť, reštartuj ESP
  if (!client.connected()) {
    Serial.println("MQTT pripojenie neuspesne! Restartujem...");
    ESP.restart();
  }
}

// ========== Funkcia na odoslanie dát na MQTT ==========
void sendData() {
  float temperature = bme.readTemperature();        // Načítaj aktuálnu teplotu zo senzora BME280
  float humidity = bme.readHumidity();              // Načítaj aktuálnu vlhkosť
  float pressure = bme.readPressure() / 100.0F;     // Načítaj tlak (v Pa) a prepočítaj na hPa
  float vbat = readBatteryVoltage();                // Zmeraj napätie na batérii alebo napájacom zdroji

  // Získaj a konvertuj aktuálny čas na lokálny (CET/CEST)
  timeClient.update();                              // Aktualizuj NTP čas (UTC)
  time_t utc = timeClient.getEpochTime();           // Získaj epochový čas (UTC)
  time_t localTime = CE.toLocal(utc);               // Preveď na lokálny čas (podľa zimného/letného režimu)
  struct tm *timeInfo = localtime(&localTime);      // Rozparsuj čas
  char timeString[25];
  sprintf(timeString, "%04d-%02d-%02d %02d:%02d:%02d",
          timeInfo->tm_year + 1900,
          timeInfo->tm_mon + 1,
          timeInfo->tm_mday,
          timeInfo->tm_hour,
          timeInfo->tm_min,
          timeInfo->tm_sec);

  // Vypíš hodnoty do seriového monitoru pre debug
  Serial.print("Teplota: "); Serial.print(temperature); Serial.println(" °C");
  Serial.print("Vlhkost: "); Serial.print(humidity); Serial.println(" %");
  Serial.print("Tlak: "); Serial.print(pressure); Serial.println(" hPa");
  Serial.print("Napatie na A0: "); Serial.print(vbat); Serial.println(" V");
  Serial.print("Cas merania: "); Serial.println(timeString);

  // Ak sme pripojení na MQTT broker, pošli dáta na príslušné topic-y
  if (client.connected()) {
    if (client.publish(mqtt_topic_temp, String(temperature).c_str(), true)) {
      Serial.println("Teplota odoslana na MQTT");
    } else {
      Serial.println("Chyba pri odosielani teploty!");
    }
    if (client.publish(mqtt_topic_humi, String(humidity).c_str(), true)) {
      Serial.println("Vlhkost odoslana na MQTT");
    } else {
      Serial.println("Chyba pri odosielani vlhkosti!");
    }
    if (client.publish(mqtt_topic_pres, String(pressure).c_str(), true)) {
      Serial.println("Tlak odoslany na MQTT");
    } else {
      Serial.println("Chyba pri odosielani tlaku!");
    }
    if (client.publish(mqtt_topic_vbat, String(vbat).c_str(), true)) {
      Serial.println("Napatie odoslane na MQTT");
    } else {
      Serial.println("Chyba pri odosielani napatia!");
    }
    if (client.publish(mqtt_topic_time, timeString, true)) {
      Serial.println("Cas odoslany na MQTT");
    } else {
      Serial.println("Chyba pri odosielani casu!");
    } 
  } else {
    // Ak MQTT nie je pripojené, reštartuj ESP (pre istotu)
    Serial.println("MQTT nie je pripojene! Restartujem...");
    ESP.restart();
  }
}

// ========== Funkcia na zmeranie napätia na analógovom vstupe A0 ==========
float readBatteryVoltage() {
  int analogValue = analogRead(A0);                 // Prečítaj hodnotu z ADC (0-1023)
  float voltage = analogValue * (3.3 / 1023.0);     // Prepočítaj na napätie (max 3.3V, 10-bit ADC)
  return voltage;
}

// ========== Loop funkcia ==========
// Hlavný cyklus ktorý beží stále
void loop() {
  // Volaj MQTT loop, aby sa spracovali správy, udržiaval spojenie atď.
  client.loop();

  // Tento kód však ESP po odoslaní dát vypne cez DONE pin (setup),
  // takže v loop vlastne nič ďalšie nerobím
}
