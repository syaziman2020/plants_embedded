#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "Siskom4"
#define WIFI_PASSWORD "rahasia123"
#define URL "http::/dht"
#define LED 17
#define SOIL1 34
#define SOIL2 35
#define SOIL3 32
#define SOIL4 33

// StaticJsonDocument<8> jsonDoc;
DynamicJsonDocument jsonDoc(128);
String data;
const char* token = "Token_Authorization";
unsigned long startTime, sendDataTime;
unsigned long sendDataDelay = -1;
int sensorReadCount = 0;
int dataSendCount = 0;
int nodeId = 2;  //identity node

void setup() {
  Serial.begin(9600);

  pinMode(LED, OUTPUT);
  pinMode(SOIL1, INPUT);
  pinMode(SOIL2, INPUT);
  pinMode(SOIL3, INPUT);
  pinMode(SOIL4, INPUT);
  connectToWiFi();

  delay(2000);
}

void loop() {
  float s1 = soil(SOIL1);
  float s2 = soil(SOIL2);
  float s3 = soil(SOIL3);
  float s4 = soil(SOIL4);


  if (isnan(s1) || isnan(s2) || isnan(s3) || isnan(s4)) {
    Serial.println(F("Failed to read from soil moisture sensor!"));
    delay(5000);
    return;
  }

  int soilMoisture1 = round(s1);
  int soilMoisture2 = round(s2);
  int soilMoisture3 = round(s3);
  int soilMoisture4 = round(s4);


  sensorReadCount++;
  dataSendCount++;
  sendData(soilMoisture1, soilMoisture2, soilMoisture3, soilMoisture4, sensorReadCount, dataSendCount);

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


void sendData(int soilMoisture1, int soilMoisture2, int soilMoisture3, int soilMoisture4, int sensorReadCount, int dataSendCount) {
  const int numSensors = 4;
  int soilMoistures[numSensors] = {soilMoisture1, soilMoisture2, soilMoisture3, soilMoisture4};

  jsonDoc["node"] = nodeId;
  jsonDoc["sensor_read_count"] = sensorReadCount;
  jsonDoc["data_send_count"] = dataSendCount;

   // Membuat Array JSON untuk menyimpan data kelembapan tanah dengan id unik
  JsonArray soilMoistureArray = jsonDoc.createNestedArray("soil_moistures");

  // Iterasi melalui setiap sensor dan menambahkannya ke dalam array JSON
  for (int i = 0; i < numSensors; i++) {
    // Membuat objek JSON untuk setiap kelembapan tanah
    JsonObject moistureObj = soilMoistureArray.createNestedObject();
    moistureObj["id"] = i + 1; // Id unik
    moistureObj["moisture"] = soilMoistures[i]; // Kelembapan tanah
  }

  if (sendDataDelay != -1) {
    jsonDoc["delay"] = sendDataDelay;
  }

  size_t payloadSize = data.length();

  // Assign payload size to the JSON document
  jsonDoc["payload_size"] = payloadSize;

  // Print the serialized JSON data
  Serial.println(data);

  serializeJson(jsonDoc, data);


  HTTPClient http;
  http.begin(String(URL));
  http.addHeader("Content-Type", "application/json");
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


float soil(int sensor) {
  float s = 100 - (float(analogRead(sensor)) / 4095 * 100);
  return s;
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