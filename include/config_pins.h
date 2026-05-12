#pragma once

#include <Arduino.h>

struct MotorPinMap {
    uint8_t in1;
    uint8_t in2;
    uint8_t en;
};

#define MOTOR_COUNT 4

// Update these pin numbers to match your exact wiring.
// motor pins
const struct MotorPinMap MOTOR_PINS[MOTOR_COUNT] = {
    {26, 27, 14}, // Motor 1
    {25, 33, 12}, // Motor 2
    {32, 15, 13}, // Motor 3
    {4, 16, 17}   // Motor 4
};

// Piezo inputs. LOW means touched/triggered.
#define PIEZO_PIN 34
#define PIEZO_THRESHOLD 1500 // Adjust based on testing

// ESP32 LEDC PWM channels, one per motor.
const uint8_t MOTOR_PWM_CHANNELS[MOTOR_COUNT] = {0, 1, 2, 3};
#define MOTOR_PWM_FREQ_HZ 2000
#define MOTOR_PWM_RES_BITS 8
#define MOTOR_PWM_DUTY 210 // 0..255
