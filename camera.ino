#include <WebServer.h>
#include <Wire.h>
#include <esp32cam.h>
#include <WiFi.h>


WebServer server(80);

void handleCapture() {
  auto img = esp32cam::capture();
  if (img == nullptr) {
    server.send(500, "", "");
    return;
  }
  server.setContentLength(img->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  img->writeTo(client);
}

const char* ssid = "Test_AP";        
const char* password = "12345678"; 

void setup() {
  //Serial.begin(115200);
  //delay(1000);
  auto res = esp32cam::Resolution::find(1024, 768);
  esp32cam::Config cfg;
  cfg.setPins(esp32cam::pins::AiThinker);
  cfg.setJpeg(80);
  esp32cam::Camera.begin(cfg);

  WiFi.softAP(ssid, password);

  // Print the IP address assigned to the AP
  IPAddress ip = WiFi.softAPIP();
  server.on("/capture.jpg", handleCapture);
  server.begin();
}

void loop() {
  server.handleClient();
}