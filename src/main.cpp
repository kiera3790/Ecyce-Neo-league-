#include <Arduino.h>
#include <string.h>

#include "motor_driver.h"
#include "piezo_stage.h"
#include "take_data_pi.h"

char serialBuffer[96];
int bufferIndex = 0;

void processSerialInput() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\r') {
      continue;
    }

    if (c == '\n') {
      motorDriverQueueFromCommand(serialBuffer);
      memset(serialBuffer, 0, sizeof(serialBuffer));
      bufferIndex = 0;
      continue;
    }

    if (bufferIndex < sizeof(serialBuffer) - 1) {
      serialBuffer[bufferIndex++] = c;
      serialBuffer[bufferIndex] = '\0';
    } else {
      memset(serialBuffer, 0, sizeof(serialBuffer));
      bufferIndex = 0;
      Serial.println("ERR CMD_TOO_LONG");
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  memset(serialBuffer, 0, sizeof(serialBuffer));
  bufferIndex = 0;
  piezoStageBegin();
  motorDriverBegin();
  takeDataPiBegin();
  Serial.println("READY. Use WiFi or Serial: RUN:1,3,4 (or STOP).");
}

void loop() {
  processSerialInput();

  if (takeDataPiHasCommand()) {
    motorDriverQueueFromCommand(takeDataPiReadCommand());
  }

  motorDriverUpdate();
}

/*
DC Motor Unit Test
ESP32 + L298N
*/

// #include <Arduino.h>

// #include "motor_driver.h"
// #include  "config_pins.h"
// #include "piezo_stage.h"

// void setup() {
//     Serial.begin(115200);
//     motorDriverBegin();
//     piezoStageBegin();
//     Serial.println("Motor _ piezo Unit Test Starting...");
// }

// int i = 0;

// void loop() {
//     if (IRStageTriggered()) {
//         i = 1;
//         Serial.println("IR triggered! Running motor");
//         //stop
//         digitalWrite(MOTOR_PINS[i-1].in1, HIGH);
//         digitalWrite(MOTOR_PINS[i-1].in2, LOW);
//         ledcWrite(MOTOR_PWM_CHANNELS[i-1], 0);
//     } else {
//         i = 0;
//     }
//     Serial.println("Running motor " + String(i));
//     digitalWrite(MOTOR_PINS[i].in1, HIGH);
//     digitalWrite(MOTOR_PINS[i].in2, LOW);
//     ledcWrite(MOTOR_PWM_CHANNELS[i], 180);
//     // IRUnitTest();
// }