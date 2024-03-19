#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define WIFI_SSID "Siskom4"
#define WIFI_PASSWORD "rahasia123"
#define URL "http::/dht"
#define DHTPIN 4
#define LED 17
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// StaticJsonDocument<8> jsonDoc;
DynamicJsonDocument jsonDoc(128);
String data;
const char* token = "Token_Authorization";
unsigned long startTime, sendDataTime;
unsigned long sendDataDelay = -1;
int sensorReadCount = 0;
int dataSendCount = 0;
int nodeId = 1;

void setup() {
  Serial.begin(9600);

  dht.begin();
  pinMode(LED, OUTPUT);
  connectToWiFi();

  delay(2000);
}

void loop() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    delay(5000);
    return;
  }

  int temperature = round(t);
  int humidity = round(h);

  sensorReadCount++;
  dataSendCount++;
  sendData(temperature, humidity, sensorReadCount, dataSendCount);

  delay(10000);
}

void connectToWiFi() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED, LOW);  //LED PIN SET LOW
    Serial.print(".");
    delay(1000);
  }

  digitalWrite(LED, HIGH);  //LED PIN SET HIGH

  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
}


void sendData(int temperature, int humidity, int sensorReadCount, int dataSendCount) {
  jsonDoc["node"] = nodeId;
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;
  jsonDoc["sensor_read_count"] = sensorReadCount;
  jsonDoc["data_send_count"] = dataSendCount;
  if(sendDataDelay != -1){
    jsonDoc["delay"] = sendDataDelay;
  }

  // Get the length of the serialized JSON string
  size_t payloadSize = data.length();

  // Assign payload size to the JSON document
  jsonDoc["payload_size"] = payloadSize;
  // Pengukuran payoad masih belum tahu
  // Serial.println(measureJson(jsonDoc));
  // Serial.println(sizeof(jsonDoc));
  // Serial.println(sizeof(data));

  serializeJson(jsonDoc, data);
  

  // Print the serialized JSON data
  Serial.println(data);

  HTTPClient http;
  http.begin(String(URL));
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");
  String authHeader = "Bearer ";
  authHeader += token;
  http.addHeader("Authorization", authHeader);
  //waktu mulai post
  startTime = millis();

  int httpCode = http.POST(data);

  if (httpCode == 201) {
    Serial.printf("POST code: %d\n", httpCode);
    sendDataTime = millis();

    //delay pengiriman
    sendDataDelay = sendDataTime - startTime;

    Serial.print("Delay time: ");
    Serial.print(sendDataDelay);
    Serial.println(" ms");
    // sendDelayData(sendDataDelay);
  } else if (httpCode > 0) {
    //waktu delay pengiriman data
    Serial.printf("POST code: %d\n", httpCode);
    Serial.println(httpCode);
  } else {
    Serial.println("Failed to send data!");
    Serial.println(httpCode);
  }

  http.end();
  jsonDoc.clear();
}

// void sendDelayData(unsigned long delay) {
//   DynamicJsonDocument delayDataDoc(128);
//   delayDataDoc["delay"] = delay;

//   String delayData;
//   serializeJson(delayDataDoc, delayData);

//   // Kirim data delay ke cloud server
//   HTTPClient http;
//   http.begin(String(URL));  // Ganti URL dengan endpoint yang sesuai
//   http.addHeader("Content-Type", "application/json");
//   int httpCode = http.POST(delayData);

//   if (httpCode == 201) {
//     Serial.print("Delay data sent successfully. HTTP response code: ");
//     Serial.println(httpCode);
//   } else if (httpCode > 0) {
//     Serial.print("Delay data sent. HTTP response code: ");
//     Serial.println(httpCode);
//   } else {
//     Serial.println("Failed to send delay data!");
//   }

//   http.end();
// }