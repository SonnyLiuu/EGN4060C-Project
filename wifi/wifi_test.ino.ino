#include <ESP8266WiFi.h>

const char* ssid = "Test_AP";        // SSID for your AP
const char* password = "12345678";   // Password for your AP

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Start the ESP8266 in Access Point mode
  WiFi.softAP(ssid, password);

  // Print the IP address assigned to the AP
  IPAddress ip = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(ip);
}

void loop() {
  // Nothing to do here
}
