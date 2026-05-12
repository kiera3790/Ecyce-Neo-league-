#include <Arduino.h>

#include "config_pins.h"
#include "piezo_stage.h"

void piezoStageBegin() {
  pinMode(PIEZO_PIN, INPUT_PULLUP);
}

bool piezoStageTriggered() {
  return analogRead(PIEZO_PIN) > PIEZO_THRESHOLD; // Adjust threshold as needed based on testing
}

void piezoUnitTest() {
  Serial.println("Starting Piezo Stage Unit Test...");
  piezoStageBegin();
  Serial.println("Piezo Stage Initialized. Please trigger the piezo sensor.");
  
  while (true) {
    if (piezoStageTriggered()) {
      Serial.println("Piezo sensor triggered!");
      delay(1000); // Debounce delay
    }
  }
}