#pragma once

#include <Arduino.h>

// Replace with your Raspberry Pi / router Wi-Fi credentials.
static constexpr char WIFI_SSID[] = "Ecyce";
static constexpr char WIFI_PASSWORD[] = "88888888";

// MQTT broker settings (Mosquitto on Raspberry Pi).
static constexpr char MQTT_BROKER_HOST[] = "192.168.1.10";
static constexpr uint16_t MQTT_BROKER_PORT = 1883;
static constexpr char MQTT_USERNAME[] = "";
static constexpr char MQTT_PASSWORD[] = "";
static constexpr char MQTT_CLIENT_ID[] = "esp32-pill-motor";

// Topic Raspberry Pi publishes command payloads to.
static constexpr char MQTT_TOPIC_COMMAND[] = "pill/motor/command";
