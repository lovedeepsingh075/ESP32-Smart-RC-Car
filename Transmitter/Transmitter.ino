#include <esp_now.h>
#include <WiFi.h>
#include <EEPROM.h>

/*
  ESP32 SMART RC CAR TRANSMITTER
  - Sends throttle + steering + headlight + hazard states
  - Supports trim calibration saved in EEPROM
  - Communication: ESP-NOW (Low Latency)
*/

// ----------------- EEPROM SETTINGS -----------------
#define EEPROM_SIZE 4
#define EEPROM_TRIM_THROTTLE_ADDR 0
#define EEPROM_TRIM_STEERING_ADDR 2

// ----------------- RECEIVER MAC ADDRESS -----------------
uint8_t receiverMacAddress[] = {0x1C, 0xC3, 0xAB, 0xB4, 0x41, 0xDC};

// ----------------- TRIM BUTTONS -----------------
#define TRIM_UP_THROTTLE    19
#define TRIM_DOWN_THROTTLE  18
#define TRIM_UP_STEERING    23
#define TRIM_DOWN_STEERING  22

// ----------------- FEATURE BUTTONS -----------------
#define HEADLIGHT_BUTTON 4
#define HAZARD_BUTTON    5

// ----------------- ANALOG INPUT PINS -----------------
#define THROTTLE_PIN 32
#define STEERING_PIN 33

// ----------------- PACKET STRUCTURE -----------------
typedef struct PacketData {
  uint8_t throttle;
  uint8_t steering;
  bool headlights;
  bool hazard;
} PacketData;

PacketData data;
esp_now_peer_info_t peerInfo;

// ----------------- VARIABLES -----------------
bool headlightState = false;
bool hazardState = false;

bool lastHeadlightButtonState = HIGH;
bool lastHazardButtonState = HIGH;

// Trim values (middle calibration points)
int trimThrottle = 2000;
int trimSteering = 2000;

// ----------------- RESET DEFAULT PACKET -----------------
void ResetData() {
  data.throttle = 127;
  data.steering = 127;
  data.headlights = false;
  data.hazard = false;
}

// ----------------- BORDER MAP FUNCTION -----------------
int Border_Map(int val, int lower, int middle, int upper, bool reverse) {
  val = constrain(val, lower, upper);

  if (val < middle) {
    val = map(val, lower, middle, 0, 128);
  } else {
    val = map(val, middle, upper, 128, 255);
  }

  return (reverse ? 255 - val : val);
}

// ----------------- SETUP -----------------
void setup() {
  Serial.begin(115200);

  EEPROM.begin(EEPROM_SIZE);
  WiFi.mode(WIFI_STA);

  // Buttons
  pinMode(TRIM_UP_THROTTLE, INPUT_PULLUP);
  pinMode(TRIM_DOWN_THROTTLE, INPUT_PULLUP);
  pinMode(TRIM_UP_STEERING, INPUT_PULLUP);
  pinMode(TRIM_DOWN_STEERING, INPUT_PULLUP);

  pinMode(HEADLIGHT_BUTTON, INPUT_PULLUP);
  pinMode(HAZARD_BUTTON, INPUT_PULLUP);

  // Load trim values from EEPROM
  trimThrottle = EEPROM.read(EEPROM_TRIM_THROTTLE_ADDR) * 16;
  trimSteering = EEPROM.read(EEPROM_TRIM_STEERING_ADDR) * 16;

  // Safety default if EEPROM is empty
  if (trimThrottle < 1120 || trimThrottle > 2520) trimThrottle = 2000;
  if (trimSteering < 1120 || trimSteering > 2520) trimSteering = 2000;

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Initialization Failed!");
    return;
  }

  // Add receiver as peer
  memcpy(peerInfo.peer_addr, receiverMacAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer!");
    return;
  }

  ResetData();
  Serial.println("Transmitter Ready.");
}

// ----------------- LOOP -----------------
void loop() {

  // ----------------- TRIM CALIBRATION -----------------
  if (digitalRead(TRIM_UP_THROTTLE) == LOW && trimThrottle < 2520) {
    trimThrottle += 60;
    EEPROM.write(EEPROM_TRIM_THROTTLE_ADDR, trimThrottle / 16);
    EEPROM.commit();
    delay(130);
  }

  if (digitalRead(TRIM_DOWN_THROTTLE) == LOW && trimThrottle > 1120) {
    trimThrottle -= 60;
    EEPROM.write(EEPROM_TRIM_THROTTLE_ADDR, trimThrottle / 16);
    EEPROM.commit();
    delay(130);
  }

  if (digitalRead(TRIM_UP_STEERING) == LOW && trimSteering < 2520) {
    trimSteering += 60;
    EEPROM.write(EEPROM_TRIM_STEERING_ADDR, trimSteering / 16);
    EEPROM.commit();
    delay(130);
  }

  if (digitalRead(TRIM_DOWN_STEERING) == LOW && trimSteering > 1120) {
    trimSteering -= 60;
    EEPROM.write(EEPROM_TRIM_STEERING_ADDR, trimSteering / 16);
    EEPROM.commit();
    delay(130);
  }

  // ----------------- HEADLIGHT TOGGLE -----------------
  bool headlightButton = digitalRead(HEADLIGHT_BUTTON);

  if (headlightButton == LOW && lastHeadlightButtonState == HIGH) {
    headlightState = !headlightState;
    delay(200);
  }

  lastHeadlightButtonState = headlightButton;

  // ----------------- HAZARD TOGGLE -----------------
  bool hazardButton = digitalRead(HAZARD_BUTTON);

  if (hazardButton == LOW && lastHazardButtonState == HIGH) {
    hazardState = !hazardState;
    delay(200);
  }

  lastHazardButtonState = hazardButton;

  // ----------------- READ JOYSTICK / ANALOG -----------------
  int throttleRaw = analogRead(THROTTLE_PIN);
  int steeringRaw = analogRead(STEERING_PIN);

  data.throttle = Border_Map(throttleRaw, 100, trimThrottle, 3900, true);
  data.steering = Border_Map(steeringRaw, 100, trimSteering, 3900, true);

  data.headlights = headlightState;
  data.hazard = hazardState;

  // ----------------- SEND PACKET -----------------
  esp_now_send(receiverMacAddress, (uint8_t*)&data, sizeof(data));

  delay(50);
}#include <esp_now.h>
#include <WiFi.h>
#include <EEPROM.h>

/*
  ESP32 SMART RC CAR TRANSMITTER
  - Sends throttle + steering + headlight + hazard states
  - Supports trim calibration saved in EEPROM
  - Communication: ESP-NOW (Low Latency)
*/

// ----------------- EEPROM SETTINGS -----------------
#define EEPROM_SIZE 4
#define EEPROM_TRIM_THROTTLE_ADDR 0
#define EEPROM_TRIM_STEERING_ADDR 2

// ----------------- RECEIVER MAC ADDRESS -----------------
uint8_t receiverMacAddress[] = {0x1C, 0xC3, 0xAB, 0xB4, 0x41, 0xDC};

// ----------------- TRIM BUTTONS -----------------
#define TRIM_UP_THROTTLE    19
#define TRIM_DOWN_THROTTLE  18
#define TRIM_UP_STEERING    23
#define TRIM_DOWN_STEERING  22

// ----------------- FEATURE BUTTONS -----------------
#define HEADLIGHT_BUTTON 4
#define HAZARD_BUTTON    5

// ----------------- ANALOG INPUT PINS -----------------
#define THROTTLE_PIN 32
#define STEERING_PIN 33

// ----------------- PACKET STRUCTURE -----------------
typedef struct PacketData {
  uint8_t throttle;
  uint8_t steering;
  bool headlights;
  bool hazard;
} PacketData;

PacketData data;
esp_now_peer_info_t peerInfo;

// ----------------- VARIABLES -----------------
bool headlightState = false;
bool hazardState = false;

bool lastHeadlightButtonState = HIGH;
bool lastHazardButtonState = HIGH;

// Trim values (middle calibration points)
int trimThrottle = 2000;
int trimSteering = 2000;

// ----------------- RESET DEFAULT PACKET -----------------
void ResetData() {
  data.throttle = 127;
  data.steering = 127;
  data.headlights = false;
  data.hazard = false;
}

// ----------------- BORDER MAP FUNCTION -----------------
int Border_Map(int val, int lower, int middle, int upper, bool reverse) {
  val = constrain(val, lower, upper);

  if (val < middle) {
    val = map(val, lower, middle, 0, 128);
  } else {
    val = map(val, middle, upper, 128, 255);
  }

  return (reverse ? 255 - val : val);
}

// ----------------- SETUP -----------------
void setup() {
  Serial.begin(115200);

  EEPROM.begin(EEPROM_SIZE);
  WiFi.mode(WIFI_STA);

  // Buttons
  pinMode(TRIM_UP_THROTTLE, INPUT_PULLUP);
  pinMode(TRIM_DOWN_THROTTLE, INPUT_PULLUP);
  pinMode(TRIM_UP_STEERING, INPUT_PULLUP);
  pinMode(TRIM_DOWN_STEERING, INPUT_PULLUP);

  pinMode(HEADLIGHT_BUTTON, INPUT_PULLUP);
  pinMode(HAZARD_BUTTON, INPUT_PULLUP);

  // Load trim values from EEPROM
  trimThrottle = EEPROM.read(EEPROM_TRIM_THROTTLE_ADDR) * 16;
  trimSteering = EEPROM.read(EEPROM_TRIM_STEERING_ADDR) * 16;

  // Safety default if EEPROM is empty
  if (trimThrottle < 1120 || trimThrottle > 2520) trimThrottle = 2000;
  if (trimSteering < 1120 || trimSteering > 2520) trimSteering = 2000;

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Initialization Failed!");
    return;
  }

  // Add receiver as peer
  memcpy(peerInfo.peer_addr, receiverMacAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer!");
    return;
  }

  ResetData();
  Serial.println("Transmitter Ready.");
}

// ----------------- LOOP -----------------
void loop() {

  // ----------------- TRIM CALIBRATION -----------------
  if (digitalRead(TRIM_UP_THROTTLE) == LOW && trimThrottle < 2520) {
    trimThrottle += 60;
    EEPROM.write(EEPROM_TRIM_THROTTLE_ADDR, trimThrottle / 16);
    EEPROM.commit();
    delay(130);
  }

  if (digitalRead(TRIM_DOWN_THROTTLE) == LOW && trimThrottle > 1120) {
    trimThrottle -= 60;
    EEPROM.write(EEPROM_TRIM_THROTTLE_ADDR, trimThrottle / 16);
    EEPROM.commit();
    delay(130);
  }

  if (digitalRead(TRIM_UP_STEERING) == LOW && trimSteering < 2520) {
    trimSteering += 60;
    EEPROM.write(EEPROM_TRIM_STEERING_ADDR, trimSteering / 16);
    EEPROM.commit();
    delay(130);
  }

  if (digitalRead(TRIM_DOWN_STEERING) == LOW && trimSteering > 1120) {
    trimSteering -= 60;
    EEPROM.write(EEPROM_TRIM_STEERING_ADDR, trimSteering / 16);
    EEPROM.commit();
    delay(130);
  }

  // ----------------- HEADLIGHT TOGGLE -----------------
  bool headlightButton = digitalRead(HEADLIGHT_BUTTON);

  if (headlightButton == LOW && lastHeadlightButtonState == HIGH) {
    headlightState = !headlightState;
    delay(200);
  }

  lastHeadlightButtonState = headlightButton;

  // ----------------- HAZARD TOGGLE -----------------
  bool hazardButton = digitalRead(HAZARD_BUTTON);

  if (hazardButton == LOW && lastHazardButtonState == HIGH) {
    hazardState = !hazardState;
    delay(200);
  }

  lastHazardButtonState = hazardButton;

  // ----------------- READ JOYSTICK / ANALOG -----------------
  int throttleRaw = analogRead(THROTTLE_PIN);
  int steeringRaw = analogRead(STEERING_PIN);

  data.throttle = Border_Map(throttleRaw, 100, trimThrottle, 3900, true);
  data.steering = Border_Map(steeringRaw, 100, trimSteering, 3900, true);

  data.headlights = headlightState;
  data.hazard = hazardState;

  // ----------------- SEND PACKET -----------------
  esp_now_send(receiverMacAddress, (uint8_t*)&data, sizeof(data));

  delay(50);
}#include <esp_now.h>
#include <WiFi.h>
#include <EEPROM.h>

/*
  ESP32 SMART RC CAR TRANSMITTER
  - Sends throttle + steering + headlight + hazard states
  - Supports trim calibration saved in EEPROM
  - Communication: ESP-NOW (Low Latency)
*/

// ----------------- EEPROM SETTINGS -----------------
#define EEPROM_SIZE 4
#define EEPROM_TRIM_THROTTLE_ADDR 0
#define EEPROM_TRIM_STEERING_ADDR 2

// ----------------- RECEIVER MAC ADDRESS -----------------
uint8_t receiverMacAddress[] = {0x1C, 0xC3, 0xAB, 0xB4, 0x41, 0xDC};

// ----------------- TRIM BUTTONS -----------------
#define TRIM_UP_THROTTLE    19
#define TRIM_DOWN_THROTTLE  18
#define TRIM_UP_STEERING    23
#define TRIM_DOWN_STEERING  22

// ----------------- FEATURE BUTTONS -----------------
#define HEADLIGHT_BUTTON 4
#define HAZARD_BUTTON    5

// ----------------- ANALOG INPUT PINS -----------------
#define THROTTLE_PIN 32
#define STEERING_PIN 33

// ----------------- PACKET STRUCTURE -----------------
typedef struct PacketData {
  uint8_t throttle;
  uint8_t steering;
  bool headlights;
  bool hazard;
} PacketData;

PacketData data;
esp_now_peer_info_t peerInfo;

// ----------------- VARIABLES -----------------
bool headlightState = false;
bool hazardState = false;

bool lastHeadlightButtonState = HIGH;
bool lastHazardButtonState = HIGH;

// Trim values (middle calibration points)
int trimThrottle = 2000;
int trimSteering = 2000;

// ----------------- RESET DEFAULT PACKET -----------------
void ResetData() {
  data.throttle = 127;
  data.steering = 127;
  data.headlights = false;
  data.hazard = false;
}

// ----------------- BORDER MAP FUNCTION -----------------
int Border_Map(int val, int lower, int middle, int upper, bool reverse) {
  val = constrain(val, lower, upper);

  if (val < middle) {
    val = map(val, lower, middle, 0, 128);
  } else {
    val = map(val, middle, upper, 128, 255);
  }

  return (reverse ? 255 - val : val);
}

// ----------------- SETUP -----------------
void setup() {
  Serial.begin(115200);

  EEPROM.begin(EEPROM_SIZE);
  WiFi.mode(WIFI_STA);

  // Buttons
  pinMode(TRIM_UP_THROTTLE, INPUT_PULLUP);
  pinMode(TRIM_DOWN_THROTTLE, INPUT_PULLUP);
  pinMode(TRIM_UP_STEERING, INPUT_PULLUP);
  pinMode(TRIM_DOWN_STEERING, INPUT_PULLUP);

  pinMode(HEADLIGHT_BUTTON, INPUT_PULLUP);
  pinMode(HAZARD_BUTTON, INPUT_PULLUP);

  // Load trim values from EEPROM
  trimThrottle = EEPROM.read(EEPROM_TRIM_THROTTLE_ADDR) * 16;
  trimSteering = EEPROM.read(EEPROM_TRIM_STEERING_ADDR) * 16;

  // Safety default if EEPROM is empty
  if (trimThrottle < 1120 || trimThrottle > 2520) trimThrottle = 2000;
  if (trimSteering < 1120 || trimSteering > 2520) trimSteering = 2000;

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Initialization Failed!");
    return;
  }

  // Add receiver as peer
  memcpy(peerInfo.peer_addr, receiverMacAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer!");
    return;
  }

  ResetData();
  Serial.println("Transmitter Ready.");
}

// ----------------- LOOP -----------------
void loop() {

  // ----------------- TRIM CALIBRATION -----------------
  if (digitalRead(TRIM_UP_THROTTLE) == LOW && trimThrottle < 2520) {
    trimThrottle += 60;
    EEPROM.write(EEPROM_TRIM_THROTTLE_ADDR, trimThrottle / 16);
    EEPROM.commit();
    delay(130);
  }

  if (digitalRead(TRIM_DOWN_THROTTLE) == LOW && trimThrottle > 1120) {
    trimThrottle -= 60;
    EEPROM.write(EEPROM_TRIM_THROTTLE_ADDR, trimThrottle / 16);
    EEPROM.commit();
    delay(130);
  }

  if (digitalRead(TRIM_UP_STEERING) == LOW && trimSteering < 2520) {
    trimSteering += 60;
    EEPROM.write(EEPROM_TRIM_STEERING_ADDR, trimSteering / 16);
    EEPROM.commit();
    delay(130);
  }

  if (digitalRead(TRIM_DOWN_STEERING) == LOW && trimSteering > 1120) {
    trimSteering -= 60;
    EEPROM.write(EEPROM_TRIM_STEERING_ADDR, trimSteering / 16);
    EEPROM.commit();
    delay(130);
  }

  // ----------------- HEADLIGHT TOGGLE -----------------
  bool headlightButton = digitalRead(HEADLIGHT_BUTTON);

  if (headlightButton == LOW && lastHeadlightButtonState == HIGH) {
    headlightState = !headlightState;
    delay(200);
  }

  lastHeadlightButtonState = headlightButton;

  // ----------------- HAZARD TOGGLE -----------------
  bool hazardButton = digitalRead(HAZARD_BUTTON);

  if (hazardButton == LOW && lastHazardButtonState == HIGH) {
    hazardState = !hazardState;
    delay(200);
  }

  lastHazardButtonState = hazardButton;

  // ----------------- READ JOYSTICK / ANALOG -----------------
  int throttleRaw = analogRead(THROTTLE_PIN);
  int steeringRaw = analogRead(STEERING_PIN);

  data.throttle = Border_Map(throttleRaw, 100, trimThrottle, 3900, true);
  data.steering = Border_Map(steeringRaw, 100, trimSteering, 3900, true);

  data.headlights = headlightState;
  data.hazard = hazardState;

  // ----------------- SEND PACKET -----------------
  esp_now_send(receiverMacAddress, (uint8_t*)&data, sizeof(data));

  delay(50);
}
