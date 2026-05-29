#include <Arduino.h>

#include "config_pins.h"
#include "IR_stage.h"

void IRStageBegin() {
  pinMode(IR_PIN, INPUT_PULLUP);
}

bool IRStageTriggered() {
  return digitalRead(IR_PIN) == HIGH;
}

void IRUnitTest() {
  Serial.println("Starting IR Stage Unit Test...");
  IRStageBegin();
  Serial.println("IR Stage Initialized. Please trigger the IR sensor.");

  while (true) {
    if (IRStageTriggered()) {
      Serial.println("IR sensor triggered!");
      delay(1000); // Debounce delay
    }
  }
}


