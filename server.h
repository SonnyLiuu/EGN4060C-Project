#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Adafruit_OV7670.h"

// source for all wifi stuff
// https://gist.github.com/idreamsi/f3caa557fb55e4fd1c69045de8c24719 

IPAddress    apIP(42, 42, 42, 42);  // Defining a static IP address: local & gateway
                                    // Default IP in AP mode is 192.168.4.1

#define MAX_BUFFER_SIZE 1024


const char* ssid = "egn4060c";
const char* password = "pass";
ESP8266WebServer server(80);

OV7670_pins pins = {.enable = 2, .reset = 0, .xclk = 4};
#define CAM_I2C Wire
#define CAM_SIZE OV7670_SIZE_DIV2
#define CAM_MODE OV7670_COLOR_RGB

Adafruit_OV7670 cam(OV7670_ADDR, &pins, &CAM_I2C);


VL53L1X sensor;

void cameraFeed();

void sensorFeed();

