/********************************************************************
 *  ESP8266 Robot Car – ESP‑NOW receiver
 ********************************************************************/
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <user_interface.h>

// ---------- Motor pins ----------
#define motor1Pin1 5   // D1
#define motor1Pin2 4   // D2
#define motor2Pin1 0   // D3
#define motor2Pin2 2   // D4
#define motor3Pin1 14  // D5
#define motor3Pin2 12  // D6
#define motor4Pin1 13  // D7
#define motor4Pin2 15  // D8

// ---------- Prototype declarations ----------
void handleCmd(char c);
void forward(), forwardLeft(), forwardRight();

void rotateLeft(), rotateRight(), left(), right();
void backward(), backLeft(), backRight(), stop();

// ---------- Global flag set from ISR ----------
volatile char pendingCmd = 's';            // default = stop
volatile bool newCmd = false;

// ---------- ESP‑NOW receive callback ----------
void onDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  if (len == 1) {                          // we expect exactly one byte
    pendingCmd = incomingData[0];
    newCmd = true;
  }
  Serial.printf("Got packet: %c\n", pendingCmd); 
}

void setup() {
  Serial.begin(115200);

  // ----- GPIO init -----
  pinMode(motor1Pin1, OUTPUT); pinMode(motor1Pin2, OUTPUT);
  pinMode(motor2Pin1, OUTPUT); pinMode(motor2Pin2, OUTPUT);
  pinMode(motor3Pin1, OUTPUT); pinMode(motor3Pin2, OUTPUT);
  pinMode(motor4Pin1, OUTPUT); pinMode(motor4Pin2, OUTPUT);
  stop();

  // ----- Radio init -----
  WiFi.mode(WIFI_STA);             // station only
  WiFi.disconnect();               // no AP
  wifi_set_channel(1);             // MUST match transmitter

  if (esp_now_init() != 0) {
    Serial.println("ESP‑NOW init failed");
    while (true) delay(100);
  }
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(onDataRecv);

  Serial.print("Ready, MAC = "); Serial.println(WiFi.macAddress());
}

void loop() {
  if (newCmd) {                    // process outside ISR context
    noInterrupts();
    char c = pendingCmd;
    newCmd = false;
    interrupts();
    handleCmd(c);
  }
}

/*************** Map byte -> motion ****************/
void handleCmd(char c) {
  Serial.printf("Running cmd: %c\n", c);  
  switch (c) {
    case 'W': forward();            break;
    case 'A': left();               break;
    case 'D': right();              break;
    case 'S': stop();               break;
    case 'Q': forwardLeft();        break;
    case 'E': forwardRight();       break;
    case 'Z': backLeft();           break;
    case 'X': backward();           break;
    case 'C': backRight();          break;
    case '1': rotateLeft();         break;
    case '3': rotateRight();        break;
    default : stop();               break;
  }
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

