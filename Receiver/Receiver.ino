#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>

/*
  ESP32 SMART RC CAR RECEIVER
  - Receives throttle + steering + headlight + hazard data using ESP-NOW
  - Controls Servo Steering + ESC Throttle
  - Automotive realistic features:
      * Headlights
      * Brake light
      * Turn indicators with memory
      * Hazard mode
      * Auto center calibration
      * FAILSAFE safety stop if signal lost
*/

// ----------------- SERVO OBJECTS -----------------
Servo steeringServo;
Servo esc;

// ----------------- OUTPUT PINS -----------------
#define HEADLIGHT_PIN   4
#define FRONT_LEFT_PIN  13
#define FRONT_RIGHT_PIN 15
#define REAR_LEFT_PIN   12
#define REAR_RIGHT_PIN  14
#define BRAKE_LIGHT_PIN 27

// ----------------- SERVO PINS -----------------
#define STEERING_SERVO_PIN 25
#define ESC_PIN            26

// ----------------- TIMERS -----------------
unsigned long blinkTimer = 0;
unsigned long lastSteerTime = 0;
unsigned long lastPacketTime = 0;

// ----------------- STATES -----------------
bool blinkState = false;

// ----------------- PACKET STRUCTURE -----------------
typedef struct PacketData {
  uint8_t throttle;
  uint8_t steering;
  bool headlights;
  bool hazard;
} PacketData;

PacketData receiverData;

// ----------------- LOGIC VARIABLES -----------------
int prevThrottle = 127;
int steerCenter = 127;

int lastDirection = 0;   // -1 = left, +1 = right

// ----------------- ESP-NOW CALLBACK -----------------
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {

  memcpy(&receiverData, incomingData, sizeof(receiverData));
  lastPacketTime = millis();

  // Servo outputs
  steeringServo.write(map(receiverData.steering, 0, 254, 0, 180));
  esc.write(map(receiverData.throttle, 0, 254, 0, 180));

  // Learn steering center (trim auto calibration)
  if (receiverData.steering > 110 && receiverData.steering < 145) {
    steerCenter = (steerCenter * 9 + receiverData.steering) / 10;
  }

  // Brake light logic
  if (prevThrottle > 140 && receiverData.throttle < 135) {
    digitalWrite(BRAKE_LIGHT_PIN, HIGH);
  }
  else if (receiverData.throttle < 120) {
    digitalWrite(BRAKE_LIGHT_PIN, HIGH);
  }
  else {
    digitalWrite(BRAKE_LIGHT_PIN, LOW);
  }

  prevThrottle = receiverData.throttle;

  // Detect steering direction for indicator memory
  if (receiverData.steering < steerCenter - 20) {
    lastDirection = -1;
    lastSteerTime = millis();
  }

  if (receiverData.steering > steerCenter + 20) {
    lastDirection = 1;
    lastSteerTime = millis();
  }
}

// ----------------- SETUP -----------------
void setup() {
  Serial.begin(115200);

  // Attach servos
  steeringServo.attach(STEERING_SERVO_PIN, 1000, 2000);
  esc.attach(ESC_PIN, 1000, 2000);

  // Output pins
  pinMode(HEADLIGHT_PIN, OUTPUT);
  pinMode(FRONT_LEFT_PIN, OUTPUT);
  pinMode(FRONT_RIGHT_PIN, OUTPUT);
  pinMode(REAR_LEFT_PIN, OUTPUT);
  pinMode(REAR_RIGHT_PIN, OUTPUT);
  pinMode(BRAKE_LIGHT_PIN, OUTPUT);

  // Defaults
  digitalWrite(HEADLIGHT_PIN, LOW);
  digitalWrite(FRONT_LEFT_PIN, LOW);
  digitalWrite(FRONT_RIGHT_PIN, LOW);
  digitalWrite(REAR_LEFT_PIN, LOW);
  digitalWrite(REAR_RIGHT_PIN, LOW);
  digitalWrite(BRAKE_LIGHT_PIN, LOW);

  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Initialization Failed!");
    return;
  }

  // Register receive callback
  esp_now_register_recv_cb(OnDataRecv);

  lastPacketTime = millis();
  Serial.println("Receiver Ready.");
}

// ----------------- LOOP -----------------
void loop() {
  unsigned long now = millis();

  // Blink timer (indicator/hazard)
  if (now - blinkTimer > 400) {
    blinkTimer = now;
    blinkState = !blinkState;
  }

  // ----------------- FAILSAFE (Signal Lost) -----------------
  if (now - lastPacketTime > 700) {

    // Stop motor + center steering
    esc.write(90);
    steeringServo.write(90);

    // Turn on hazard blinking
    digitalWrite(FRONT_LEFT_PIN, blinkState);
    digitalWrite(FRONT_RIGHT_PIN, blinkState);
    digitalWrite(REAR_LEFT_PIN, blinkState);
    digitalWrite(REAR_RIGHT_PIN, blinkState);

    digitalWrite(BRAKE_LIGHT_PIN, HIGH);
    digitalWrite(HEADLIGHT_PIN, LOW);

    return;
  }

  // ----------------- HEADLIGHTS -----------------
  digitalWrite(HEADLIGHT_PIN, receiverData.headlights);

  // ----------------- HAZARD MODE -----------------
  if (receiverData.hazard) {

    digitalWrite(FRONT_LEFT_PIN, blinkState);
    digitalWrite(FRONT_RIGHT_PIN, blinkState);
    digitalWrite(REAR_LEFT_PIN, blinkState);
    digitalWrite(REAR_RIGHT_PIN, blinkState);

    return;
  }

  // ----------------- STEERING DIRECTION DETECTION -----------------
  bool left = false;
  bool right = false;

  if (receiverData.steering < steerCenter - 20) left = true;
  if (receiverData.steering > steerCenter + 20) right = true;

  // Indicator memory: keep blinking after turning
  if (!left && !right) {
    if (now - lastSteerTime < 1500) {
      if (lastDirection == -1) left = true;
      if (lastDirection == 1) right = true;
    }
  }

  // ----------------- LEFT INDICATOR -----------------
  if (left) {

    digitalWrite(FRONT_LEFT_PIN, blinkState);
    digitalWrite(REAR_LEFT_PIN, blinkState);

    digitalWrite(FRONT_RIGHT_PIN, LOW);
    digitalWrite(REAR_RIGHT_PIN, receiverData.headlights);
  }

  // ----------------- RIGHT INDICATOR -----------------
  else if (right) {

    digitalWrite(FRONT_RIGHT_PIN, blinkState);
    digitalWrite(REAR_RIGHT_PIN, blinkState);

    digitalWrite(FRONT_LEFT_PIN, LOW);
    digitalWrite(REAR_LEFT_PIN, receiverData.headlights);
  }

  // ----------------- STRAIGHT (NO INDICATORS) -----------------
  else {

    digitalWrite(FRONT_LEFT_PIN, LOW);
    digitalWrite(FRONT_RIGHT_PIN, LOW);

    // Rear lights act as tail lamps when headlights ON
    digitalWrite(REAR_LEFT_PIN, receiverData.headlights);
    digitalWrite(REAR_RIGHT_PIN, receiverData.headlights);
  }
}
