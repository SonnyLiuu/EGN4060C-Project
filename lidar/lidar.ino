#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_now.h>
#include <Wire.h>
#include <Adafruit_VL53L1X.h>

// ——— CONFIG —————————————————————————————
static constexpr size_t   kNumSensors     = 4;
static constexpr uint8_t  kXshutPins[]    = { 13, 14, 27, 25 };
static constexpr uint8_t  kI2CAddresses[] = { 0x30, 0x31, 0x32, 0x33 };
static constexpr uint32_t kTimingBudgetUs = 50000;  // 50 ms

// Wi-Fi credentials
const char* ssid     = "ATT";
const char* password = "marbletheory398";

// UDP listen port
static constexpr unsigned int kCtrlPort = 5005;
WiFiUDP udp;

// ESP-NOW peer (your ESP8266’s MAC)
uint8_t motorMAC[6] = { 0xE0, 0x98, 0x06, 0x8E, 0x0F, 0x4D };

// VL53L1X sensors
Adafruit_VL53L1X sensors[kNumSensors] = {
  Adafruit_VL53L1X(kXshutPins[0]),
  Adafruit_VL53L1X(kXshutPins[1]),
  Adafruit_VL53L1X(kXshutPins[2]),
  Adafruit_VL53L1X(kXshutPins[3]),
};

// latest distance readings (mm)
static int16_t distances[kNumSensors] = { -1, -1, -1, -1 };
static int16_t avoidDistance = 120;

// state for obstacle avoidance
char avoidDirection = 'O';
static bool avoidanceActive = false;

// ——— ESP-NOW callbacks & helpers —————————————————
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // optional debug
}

void sendESPNOW(char cmd) {
  esp_now_send(motorMAC, (uint8_t*)&cmd, 1);
}

// ——— initialize a single VL53L1X —————————————————
void initSensor(size_t i) {
  pinMode(kXshutPins[i], OUTPUT);
  digitalWrite(kXshutPins[i], LOW);
  delay(10);
  digitalWrite(kXshutPins[i], HIGH);
  delay(50);  // give it a bit more time

  if (!sensors[i].begin(kI2CAddresses[i])) {
    Serial.printf("❌ Sensor %u @0x%02X failed\n", i, kI2CAddresses[i]);
    while (1) delay(10);
  }
  sensors[i].setTimingBudget(kTimingBudgetUs);
  sensors[i].startRanging();
  Serial.printf("✅ Sensor %u @0x%02X ready\n", i, kI2CAddresses[i]);
}

// ——— optional: scan I²C bus for debugging —————————
void scanI2C() {
  Serial.println("I2C Scan:");
  for (uint8_t addr = 1; addr < 127; ++addr) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf(" • Found 0x%02X\n", addr);
    }
  }
}

// ——— main decision logic —————————————————————
void handleCommand(char cmd, const int16_t dist[]) {
  bool clearAhead = dist[1] > avoidDistance && dist[2] > avoidDistance;

  if (!clearAhead) {
    avoidanceActive = true;
    if (cmd == 'S') {
      avoidDirection = 'O';
      sendESPNOW('S');
    }
    else if (avoidDirection != 'O') {
      sendESPNOW(avoidDirection);
    }
    else {
      avoidDirection = (cmd == 'W') ? 'D' : cmd;
      sendESPNOW(avoidDirection);
    }
  }
  else {
    if (avoidanceActive) {
      // just cleared
      sendESPNOW('W');
      delay(1000);
      avoidanceActive = false;
    }
    avoidDirection = 'O';
    sendESPNOW(cmd);
  }
}

static char currentCmd = 'S';

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  // ——— bring up I2C at 400 kHz ————————————————
  Wire.begin(21, 22);
  Wire.setClock(400000);
  scanI2C();

  // ——— Wi-Fi + UDP ———————————————————————
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Wi-Fi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.printf("\nIP: %s\n", WiFi.localIP().toString().c_str());
  udp.begin(kCtrlPort);
  Serial.printf("UDP @%u\n", kCtrlPort);

  // ——— ESP-NOW peer setup ———————————————————
  if (esp_now_init() != ESP_OK) {
    Serial.println("!! ESP-NOW init failed");
    while (1) delay(10);
  }
  esp_now_register_send_cb(onDataSent);
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, motorMAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("!! failed to add peer");
    while (1) delay(10);
  }

  // ——— VL53L1X sensors —————————————————————
  for (size_t i = 0; i < kNumSensors; i++) {
    initSensor(i);
  }
}

void loop() {
  // 1) read sensors
  for (size_t i = 0; i < kNumSensors; ++i) {
    if (sensors[i].dataReady()) {
      uint16_t mm = sensors[i].distance();
      if (mm > 0) distances[i] = mm;
      sensors[i].clearInterrupt();
    }
  }

  // 2) read UDP byte
  if (udp.parsePacket() > 0) {
    char cmd;
    udp.read(&cmd, 1);
    currentCmd = cmd;
  }

  // 3) avoidance logic
  handleCommand(currentCmd, distances);

  // 4) debug print
  static unsigned long last = 0;
  if (millis() - last > 1000) {
    last = millis();
    Serial.printf("S0=%d S1=%d S2=%d S3=%d → %c\n",
                  distances[0], distances[1],
                  distances[2], distances[3],
                  (avoidDirection!='O' ? avoidDirection : currentCmd));
  }

  delay(100);
}
