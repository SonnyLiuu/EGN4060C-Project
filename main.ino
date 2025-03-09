#include <Wire.h>
#include <VL53L1X.h>
#include "server.h"

void sensorSetup() {
  Wire.begin(21, 22); // Use default ESP32 I2C pins or specify custom ones
  Wire.setClock(400000); 

  sensor.setTimeout(500);
  if (!sensor.init()) {
    Serial.println("Failed to detect and initialize VL53L1X sensor!");
    while (1); // Freeze execution
  }
  sensor.setDistanceMode(VL53L1X::Long);
  sensor.setMeasurementTimingBudget(50000);
  sensor.startContinuous(50);

  Serial.println("Sensor initialized and running.");

}

void serverSetup() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));   // subnet FF FF FF 00  
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);

  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  server.on("/camera", cameraFeed());
  server.on("/sensor", sensorFeed());

  server.begin();
	Serial.println("HTTP server started");

}


void setup() {
  Serial.begin(9600);
  delay(100); 
  sensorSetup();
  
  serverSetup();
  
}

void loop() {
  server.handleClient();
  
}