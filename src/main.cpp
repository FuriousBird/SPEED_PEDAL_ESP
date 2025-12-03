#include <Arduino.h>
#include <Wire.h>
#include <AS5600.h>

AS5600 encoder;

#define VOLTAGE1_PIN 34
#define VOLTAGE2_PIN 35
#define USE_CALIB 1

float calib_v1_off = 0;
float calib_v2_off = 0;
float calib_v1_scl = 1.0;
float calib_v2_scl = 1.0;


// ---------------------------------------------------------
// SEND PACKETS
// ---------------------------------------------------------

// Send DATA packet: prefix 0xA5 + 12-bit value + 4-bit checksum
void sendAnglePacket(uint16_t angle12, uint16_t V1, uint16_t V2) {
  // Send a formatted string with values
  Serial.print("#");
  Serial.print(angle12);
  Serial.print(",");
  Serial.print(V1);
  Serial.print(",");
  Serial.println(V2);
}

// Send ERROR packet: prefix 0x5A + 16-bit code + 8-bit checksum
// Send human-readable ERROR packet: prefix 0x5A + 3-char error code + 8-bit checksum
void sendErrorPacket(uint16_t err) {
  String errorCode;

  // Map error codes to human-readable strings
  switch (err) {
    case 0x01:
      errorCode = "NON";  // not detected
      break;
    case 0x02:
      errorCode = "STK";  // magnet too strong
      break;
    case 0x03:
      errorCode = "WEK";  // magnet too weak
      break;
    case 0x04:
      return; // ignore magnet is gud
    default:
      errorCode = "UNK";  // Default Unknown
      break;
  }

  // Send the prefix 0x5A
  Serial.write("!");

  // Send the 3-character error code (ASCII)
  for (int i = 0; i < 3; i++) {
    Serial.write(errorCode[i]);
  }
  Serial.write('\n');
}


// ---------------------------------------------------------
// SETUP
// ---------------------------------------------------------

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // pinMode(VOLTAGE1_PIN, INPUT);
  // pinMode(VOLTAGE2_PIN, INPUT);

  if (!encoder.begin()) {
    // error code 0x0001 = AS5600 not detected
    sendErrorPacket(0x0001);
    while (1);
  }

  // analogSetPinAttenuation(VOLTAGE1_PIN, ADC_11db);
  // analogSetPinAttenuation(VOLTAGE2_PIN, ADC_11db);
  analogReadResolution(12);  // This ensures full 12-bit resolution (0-4095)
}

// ---------------------------------------------------------
// LOOP
// ---------------------------------------------------------
int initial_mean = -50;

void loop() {
  // -------- AS5600 STATUS --------
  int status = encoder.readStatus();

  if (status & 0b00001000) {    // magnet too strong
    sendErrorPacket(0x0002);
  }
  if (status & 0b00010000) {    // magnet too weak
    sendErrorPacket(0x0003);
  }
  if (status & 0b00100000) {    // magnet detected
    sendErrorPacket(0x0004);
  }

  // -------- ANGLE (0–4095 raw) --------
  uint16_t rawAngle = encoder.readAngle();   // native 12-bit angle
  rawAngle &= 0x0FFF;                        // ensure 12-bit clean

  uint16_t V1 = analogRead(VOLTAGE1_PIN);
  uint16_t V2 = analogRead(VOLTAGE2_PIN);
  float mean = (V1 + V2) / 2.0;
  if (initial_mean == -50) {
    initial_mean = mean;
  }
  float diff = mean - initial_mean;
  if (abs(diff) > 4095*0.05) {
    // calibrate
    Serial.println("\%ABSOLUTE DIFFERENCE TOO HIGH");
  }
  sendAnglePacket(rawAngle, V1, V2);
  if (abs(V1 - V2) > 5) {
    V1 = 0;
    V2 = 0;
    
  }

  // Slow the loop a bit so UART isn’t spammed too fast
  delay(50); //WARNING: KEEP THIS BELOW 100 to shutdown all power in case of issue within 100ms
}