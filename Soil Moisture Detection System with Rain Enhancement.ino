/*
  ESP32 publish telemetry data to VOne Cloud (Soil Moisture)
*/

#include "VOneMqttClient.h"

// Constants for soil moisture sensor data range
int MinMoistureValue = 4095; // Maximum raw value for soil moisture (dry)
int MaxMoistureValue = 1800; // Minimum raw value for soil moisture (wet)
int MinMoisture = 0;         // Minimum value for moisture percentage
int MaxMoisture = 100;       // Maximum value for moisture percentage
int Moisture = 0;            // Variable to store current moisture percentage

// WiFi Credentials
#define WIFI_SSID "HUAWEI-B310-730F"  // Replace with your Wi-Fi SSID
#define WIFI_PASSWORD "047BHMTHG2B"   // Replace with your Wi-Fi password

// Device IDs for sensors
const char* RainSensor = "4c96f694-71dd-491b-9369-119f05ed90b1";  // Rain sensor ID
const char* MoistureSensor = "3d13e9d0-67c7-421d-a8c7-b3c2ea5febcf";  // Soil moisture sensor ID

// Pins used for reading rain and moisture data
const int rainPin = 34;  // Pin connected to rain sensor
const int moisturePin = 35;  // Pin connected to soil moisture sensor

// Create an instance of VOneMqttClient for communication with VOne Cloud
VOneMqttClient voneClient;

// Variable to track the time of the last message sent
unsigned long lastMsgTime = 0;

void setup_wifi() {
  // Initialize serial communication and attempt to connect to WiFi
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);  // Print Wi-Fi SSID to serial monitor

  WiFi.mode(WIFI_STA);  // Set Wi-Fi mode to Station (client)
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  // Connect to Wi-Fi using credentials

  // Wait until connected to Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");  // Print dot to show connection progress
  }

  // Once connected, print the IP address
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  // Initialize serial communication at 115200 baud rate
  Serial.begin(115200);
  setup_wifi();  // Call the Wi-Fi setup function
  voneClient.setup();  // Initialize VOne MQTT client
}

void loop() {
  // Check if the MQTT client is connected to the server
  if (!voneClient.connected()) {
    // If not connected, try reconnecting
    voneClient.reconnect();
    // Publish initial device status (indicating devices are online)
    voneClient.publishDeviceStatusEvent(RainSensor, true);
    voneClient.publishDeviceStatusEvent(MoistureSensor, true);
  }

  // Process incoming messages and maintain the connection
  voneClient.loop();

  // Check if the interval has passed (to control the frequency of data publishing)
  unsigned long cur = millis();
  if (cur - lastMsgTime > INTERVAL) {
    lastMsgTime = cur;  // Update last message time

    // Publish telemetry data 1: Rain sensor status (whether it is raining or not)
    int raining = !digitalRead(rainPin);  // Read rain sensor (inverted logic)
    voneClient.publishTelemetryData(RainSensor, "Raining", raining);  // Publish data to VOne Cloud

    // Publish telemetry data 2: Soil moisture percentage
    int sensorValue = analogRead(moisturePin);  // Read analog value from soil moisture sensor
    Moisture = map(sensorValue, MinMoistureValue, MaxMoistureValue, MinMoisture, MaxMoisture);  // Map raw value to percentage
    voneClient.publishTelemetryData(MoistureSensor, "Soil moisture", Moisture);  // Publish moisture data to VOne Cloud
  }
}
