// #pragma once

// #include <Arduino.h>

// // Network Credentials (Mobile Hotspot)
// #define WIFI_SSID "KHIM"
// #define WIFI_PASSWORD "12345789"

// // MQTT Broker Settings (HiveMQ Cloud)
// static constexpr char MQTT_BROKER_HOST[] = "broker.hivemq.com";
// static constexpr uint16_t MQTT_BROKER_PORT = 1883;
// static constexpr char MQTT_USERNAME[] = "";
// static constexpr char MQTT_PASSWORD[] = "";
// static constexpr char MQTT_CLIENT_ID[] = "esp32-pill-motor-001";

// // Topic for device communication
// static constexpr char MQTT_TOPIC_COMMAND[] = "ecyce/medilink/device1";

#pragma once

#include <Arduino.h>

// Network Credentials (Mobile Hotspot)
#define WIFI_SSID "Ven Gia Nghia"
#define WIFI_PASSWORD "tramissocute"

// MQTT Broker Settings (Mosquitto Public)
static constexpr char MQTT_BROKER_HOST[] = "172.20.10.4";
static constexpr uint16_t MQTT_BROKER_PORT = 1883;
static constexpr char MQTT_USERNAME[] = ""; 
static constexpr char MQTT_PASSWORD[] = "";
static constexpr char MQTT_CLIENT_ID[] = "esp32-pill-motor-001";

// Topic for device communication
static constexpr char MQTT_TOPIC_COMMAND[] = "ecyce/medilink/device1";