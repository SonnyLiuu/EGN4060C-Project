// control for rear motors


// motor 1       motor 2
// out1-2        out3-4      (L298n #1)
// M ----------- M           (Front)
//        -
//        -
//        -
//        -
//        -
// M ----------- M           (Rear)
// motor 3      motor 4
// out1-2        out3-4      (L298n #2)


// Wiring Diagram (wire color)
// B     B
// R     R



// B     B
// R     R

/*******************************************************
 *  Minimal Example: ESP8266 Robot Car Controller
 *  
 *  - Creates a WiFi access point named "RobotCar"
 *    with password "12345678"
 *  - Serves a simple page at http://192.168.4.1/
 *  - Captures key presses (q, w, e, a, s, d, z, x, c)
 *    via JavaScript and calls REST endpoints
 *  - Each REST endpoint (e.g. /forward) runs
 *    one of the motor movement functions
 ********************************************************/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
// ---------------------------
// Motor pin definitions
// ---------------------------
#define motor1Pin1 5   // D1  for IN1 (Motor 1)
#define motor1Pin2 4   // D2  for IN2 (Motor 1)
#define motor2Pin1 0   // D3  for IN3 (Motor 2)
#define motor2Pin2 2   // D4  for IN4 (Motor 2)

#define motor3Pin1 14   // D5  for IN1 (Motor 3)
#define motor3Pin2 12   // D6  for IN2 (Motor 3)
#define motor4Pin1 13   // D7  for IN3 (Motor 4)
#define motor4Pin2 15   // D8  for IN4 (Motor 4)


// ---------------------------
// Forward declarations
// ---------------------------
void forwardLeft();
void forward();
void forwardRight();
void rotateLeft();
void rotateRight();
void left();
void stop();
void right();
void backLeft();
void backward();
void backRight();

// ---------------------------
// Create web server on port 80
// ---------------------------
ESP8266WebServer server(80);

// ---------------------------
// HTML page served by the ESP8266
// Includes JavaScript to detect key events and fetch() endpoints
// ---------------------------
const char MAIN_page[] PROGMEM = R"=====(

<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8"/>
    <title>Robot Car Controller</title>
  </head>
  <body>
    <h1>ESP8266 Robot Car Controller</h1>
    <p>Use keys <br/>
       <b>Q W E</b><br/> 
       <b>A S D</b><br/>
       <b>Z X C</b><br/>
       <b>1 2</b><br/>
       to move the robot. S stops it.
    </p>
       
    <script>
      // Keydown listener
      document.addEventListener('keydown', (event) => {
        const key = event.key.toLowerCase(); // unify everything to lowercase

        // Send fetch GET requests based on key
        // Q W E  A S D  Z X C
        if (key === '1') {
          fetch('/rotateLeft');
        } else if (key === '3') {
          fetch('/rotateRight');
        } else if (key === 'q') {
          fetch('/forwardLeft');
        } else if (key === 'w') {
          fetch('/forward');
        } else if (key === 'e') {
          fetch('/forwardRight');
        } else if (key === 'a') {
          fetch('/left');
        } else if (key === 's') {
          fetch('/stop');
        } else if (key === 'd') {
          fetch('/right');
        } else if (key === 'z') {
          fetch('/backLeft');
        } else if (key === 'x') {
          fetch('/backward');
        } else if (key === 'c') {
          fetch('/backRight');
        }
      });
    </script>
  </body>
</html>

)=====";


// ---------------------------
// Setup: Wi-Fi, GPIO, routes
// ---------------------------
void setup() {
  Serial.begin(115200);

  // Initialize motor pins
  // Front Left
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  // Front Right
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  // Rear Left
  pinMode(motor3Pin1, OUTPUT);
  pinMode(motor3Pin2, OUTPUT);
  // Rear Right
  pinMode(motor4Pin1, OUTPUT);
  pinMode(motor4Pin2, OUTPUT);

  // Stop motors at startup
  stop();

  // ---------------------------
  // Create a Wi-Fi access point
  // (SSID: RobotCar, PASS: 12345678)
  // (You can change these to your own)
  // ---------------------------
  WiFi.mode(WIFI_AP);
  WiFi.softAP("RobotCar", "12345678");

  Serial.println("");
  Serial.print("Access Point created. IP address: ");
  Serial.println(WiFi.softAPIP());


  // ---------------------------
  // Define REST endpoints
  // ---------------------------
  server.on("/", handleRoot);
  server.on("/forwardLeft", [](){
    forwardLeft();
    server.send(200, "text/plain", "forwardLeft");
  });
  server.on("/forward", [](){
    forward();
    server.send(200, "text/plain", "forward");
  });
  server.on("/forwardRight", [](){
    forwardRight();
    server.send(200, "text/plain", "forwardRight");
  });
  server.on("/rotateLeft", [](){
    rotateLeft();
    server.send(200, "text/plain", "rotateLeft");
  });
  server.on("/rotateRight", [](){
    rotateRight();
    server.send(200, "text/plain", "rotateRight");
  });
  server.on("/left", [](){
    left();
    server.send(200, "text/plain", "left");
  });
  server.on("/stop", [](){
    stop();
    server.send(200, "text/plain", "stop");
  });
  server.on("/right", [](){
    right();
    server.send(200, "text/plain", "right");
  });
  server.on("/backLeft", [](){
    backLeft();
    server.send(200, "text/plain", "backLeft");
  });
  server.on("/backward", [](){
    backward();
    server.send(200, "text/plain", "backward");
  });
  server.on("/backRight", [](){
    backRight();
    server.send(200, "text/plain", "backRight");
  });

  // Catch-all for any undefined URL
  server.onNotFound([](){
    server.send(404, "text/plain", "Not found");
  });

  // Start the server
  server.begin();
  Serial.println("HTTP server started");
}

// Serve the main page
void handleRoot() {
  server.send_P(200, "text/html", MAIN_page);
}






// ---------------------------
// Main loop
// ---------------------------
void loop() {
  // Handle incoming client requests
  server.handleClient();
}


// ---------------------------
// Motor movement functions
// ---------------------------
void rotateLeft() {
  // Front Left Backward
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  // Front Right Forward
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);
  // Rear Left Backward
  digitalWrite(motor3Pin1, HIGH);
  digitalWrite(motor3Pin2, LOW);
  // Rear Right Forward
  digitalWrite(motor4Pin1, LOW);
  digitalWrite(motor4Pin2, HIGH);
}
void rotateRight() {
  // Front Left Forward
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  // Front Right Backward
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
  // Rear Left Forward
  digitalWrite(motor3Pin1, LOW);
  digitalWrite(motor3Pin2, HIGH);
  // Rear Right Backward
  digitalWrite(motor4Pin1, HIGH);
  digitalWrite(motor4Pin2, LOW);
}


void forwardLeft() {
  // Front Left Stop
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  // Front Right Forward
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);
  // Rear Left Forward
  digitalWrite(motor3Pin1, LOW);
  digitalWrite(motor3Pin2, HIGH);
  // Rear Right Stop
  digitalWrite(motor4Pin1, LOW);
  digitalWrite(motor4Pin2, LOW);
}
void forward() {
  // Front Left Forward
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  // Front Right Forward
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);
  // Rear Left Forward
  digitalWrite(motor3Pin1, LOW);
  digitalWrite(motor3Pin2, HIGH);
  // Rear Right Forward
  digitalWrite(motor4Pin1, LOW);
  digitalWrite(motor4Pin2, HIGH);
}
void forwardRight() {
  // Front Left Forward
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  // Front Right Stop
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
  // Rear Left Stop
  digitalWrite(motor3Pin1, LOW);
  digitalWrite(motor3Pin2, LOW);
  // Rear Right Forward
  digitalWrite(motor4Pin1, LOW);
  digitalWrite(motor4Pin2, HIGH);
}


void left() {
  // Front Left Backward
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  // Front Right Forward
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);
  // Rear Left Forward
  digitalWrite(motor3Pin1, LOW);
  digitalWrite(motor3Pin2, HIGH);
  // Rear Right Backward
  digitalWrite(motor4Pin1, HIGH);
  digitalWrite(motor4Pin2, LOW);
}
void stop() {
  // Front Left Stop
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  // Front Right Stop
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
  // Rear Left Stop
  digitalWrite(motor3Pin1, LOW);
  digitalWrite(motor3Pin2, LOW);
  // Rear Right Stop
  digitalWrite(motor4Pin1, LOW);
  digitalWrite(motor4Pin2, LOW);
}
void right() {
  // Front Left Forward
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  // Front Right Backward
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
  // Rear Left Backward
  digitalWrite(motor3Pin1, HIGH);
  digitalWrite(motor3Pin2, LOW);
  // Rear Right Forward
  digitalWrite(motor4Pin1, LOW);
  digitalWrite(motor4Pin2, HIGH);
}


void backLeft() {
  // Front Left Backward
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  // Front Right Stop
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
  // Rear Left Stop
  digitalWrite(motor3Pin1, LOW);
  digitalWrite(motor3Pin2, LOW);
  // Rear Right Backward
  digitalWrite(motor4Pin1, HIGH);
  digitalWrite(motor4Pin2, LOW);
}
void backward() {
  // Front Left Backward
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  // Front Right Backward
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
  // Rear Left Backward
  digitalWrite(motor3Pin1, HIGH);
  digitalWrite(motor3Pin2, LOW);
  // Rear Right Backward
  digitalWrite(motor4Pin1, HIGH);
  digitalWrite(motor4Pin2, LOW);
}
void backRight() {
  // Front Left Stop
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  // Front Right Backward
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
  // Rear Left Backward
  digitalWrite(motor3Pin1, HIGH);
  digitalWrite(motor3Pin2, LOW);
  // Rear Right Stop
  digitalWrite(motor4Pin1, LOW);
  digitalWrite(motor4Pin2, LOW);
}

