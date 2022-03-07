#include <Arduino_JSON.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

#include <DHT.h>


#define LED_BUILT_IN 2

#define WIFI_SSID "P1206"           //ten wifi
#define WIFI_PASSWORD "binhan2809"  //mat khau wifi

//const String TEST_SERVER = "192.168.100.7";
#define TEST_SERVER  "https://google.com"

#define DHTPIN 5     // D3
#define DHTTYPE DHT11

#define POWER_SOIL_HUMI_PIN 4 // D4
#define LIHGT_PIN 12 // D8
#define PUMP_PIN 13  // D9
#define LIGHT_SENSOR 14 // D5

#define DEVICE_ID 58

DHT dht(DHTPIN, DHTTYPE);

float temperature = 0;
float airHumi = 0;
int soilHumi = 0;

int lightSensor = 1; // 1 == dark

bool autoLight = false;
bool lightState = false;

bool autoPump = false;
int lowerBoundSoilHumi = 100;
int upperBoundSoilHumi = 100;

void setup() {
  connectWifi();
  Serial.begin(115200);
  pinMode(LED_BUILT_IN, OUTPUT);
  pinMode(POWER_SOIL_HUMI_PIN, OUTPUT);
  pinMode(LIHGT_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);

  digitalWrite(POWER_SOIL_HUMI_PIN, LOW);
  digitalWrite(LED_BUILT_IN, HIGH);
  digitalWrite(LIHGT_PIN, HIGH);
  digitalWrite(PUMP_PIN, HIGH);
  
  pinMode(LIGHT_SENSOR, INPUT);
}

void loop() {
  readSensor();
  getData();
//  bool success = true;
  updateState();

//  if(success) {
//    
//  } else {
//    
//  }
  delay(5000);
}

void connectWifi() {
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  Serial.begin(115200);
  Serial.print("connecting");
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("connected\n");
}

bool getData() {
    Serial.print("wifi status: ");
    Serial.println(WiFi.status());
    HTTPClient http;
  //  WiFiClient wifiClient;
    WiFiClientSecure wifiClient;
    wifiClient.setInsecure();
    http.setReuse(false);

    StaticJsonDocument<200> bodyJson;
    bodyJson["temp"] = (int) temperature;
    bodyJson["timeTemp"] = "auto";
    bodyJson["humi"] = (int) airHumi;
    bodyJson["timeHumi"] = "auto";
    String body = "";
    serializeJson(bodyJson, body);
    Serial.print("request: ");
    Serial.println(body);
  //  http.begin(wifiClient, "https://www.example.com");
//    http.begin(wifiClient, "https://192.168.100.7:5001/api/esp/test");
  //  http.begin(wifiClient, "192.168.0.7", 5001, "/api/esp/test");
    http.begin(wifiClient, "https://iot-app-nnc.herokuapp.com/api/device/" + String(DEVICE_ID) + "/state");
    http.addHeader("Authorization", "Beaerer token-go-here");
    http.addHeader("Accept", "application/json");
    http.addHeader("Content-Type", "application/json");
//    int httpResponseCode = http.GET();
    int httpResponseCode = http.POST(body);
    String payload = http.getString();
    Serial.print("Status code: ");
    Serial.println(httpResponseCode);
    Serial.print("data: ");
    Serial.println(payload);
    http.end();


//    // good serializer library 
    StaticJsonDocument<300> myJson;
    DeserializationError error = deserializeJson(myJson, payload);
    if(error) {
      Serial.print("deserializeJson() failed: ");
        Serial.println(error.f_str());
    } else {
        autoLight = myJson["autoLight"];
        lightState = myJson["lightState"];
        autoPump = myJson["intVar"];
        lowerBoundSoilHumi = myJson["lowerBoundSoilHumi"];
        upperBoundSoilHumi = myJson["upperBoundSoilHumi"];
        Serial.println(autoLight);
        Serial.println(lightState);
        Serial.println(autoPump);
        Serial.println(lowerBoundSoilHumi);
        Serial.println(upperBoundSoilHumi);
    }

    if(error || httpResponseCode != 200)
      return false;
    return true;
}

void readSensor() {
    dht.begin();
    temperature = dht.readTemperature();
    airHumi = dht.readHumidity();
    Serial.print("nhiet do: ");
    Serial.println(temperature);
    Serial.print("do am ko khi: ");
    Serial.println(airHumi);

    readSoilHumi();

    lightSensor = digitalRead(LIGHT_SENSOR);
    Serial.print("lightSensor: ");
    Serial.println(lightSensor);
}

void readSoilHumi() {
     digitalWrite(POWER_SOIL_HUMI_PIN, HIGH);
    int analogValue = analogRead(A0);
    Serial.print("analog: ");
    Serial.println(analogValue);
    soilHumi = analogValue * 100 /650;
    Serial.print("soil humi: ");
    Serial.println(soilHumi);
    digitalWrite(POWER_SOIL_HUMI_PIN, LOW);
}

void turnOn(int pin) {
  digitalWrite(pin, LOW);
}

void turnOff(int pin) {
  digitalWrite(pin, HIGH);
}

void updateState() {
  if(autoLight) {
    if(lightSensor == 1) {
      turnOn(LIHGT_PIN);
    } else {
      turnOff(LIHGT_PIN);
    }
  } else if(lightState) {
    turnOn(LIHGT_PIN);
  } else {
    turnOff(LIHGT_PIN);
  }
//  if(autoPump && soilHumi < lowerBoundSoilHumi) {
//    turnOn(PUMP_PIN);
//    while(soilHumi < upperBoundSoilHumi) {
//      delay(10000);
//    }
//    turnOff(PUMP_PIN);
//  }
}
