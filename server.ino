#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Adafruit_OV7670.h"
#include "server.h"



void cameraFeed() {
    cam.suspend();
    cam.Y2RGB565();
    //server.sendHeader("Content-Type", "image/jpeg");
    server.send(200, "image/jpeg", (const char*)cam.getBuffer(), cam.width() * cam.height() * 2);
    cam.resume();
}


void sensorFeed() {
    delay(100);
    int distance = sensor.read();
    char response[MAX_BUFFER_SIZE];
    snprintf(response, sizeof(response), "{\"distance\": %d}", distance);
    server.send(200, "application/json", response);
}
