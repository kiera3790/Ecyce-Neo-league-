#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include "take_data_pi.h"
#include "wifi_config.h"
#include "config_pins.h"

static WiFiClient mqttNetClient;
static PubSubClient mqttClient(mqttNetClient);
static char pendingCommand[128];
static char returnedCommand[128];
static unsigned long lastMqttReconnectTryMs = 0;
static constexpr unsigned long MQTT_RECONNECT_INTERVAL_MS = 3000;

static bool parsePiPayloadToRunCommand(const char *payload, char *outCommand, size_t outSize) {
  int counts[MOTOR_COUNT] = {0, 0, 0, 0};
  const char *cursor = payload;

  while (*cursor) {
    while (*cursor && isspace((unsigned char)*cursor)) {
      cursor++;
    }
    if (*cursor == '\0') {
      break;
    }

    if (strncmp(cursor, "MOTOR", 5) != 0) {
      return false;
    }
    cursor += 5;

    while (*cursor && isspace((unsigned char)*cursor)) {
      cursor++;
    }
    if (!isdigit((unsigned char)*cursor)) {
      return false;
    }
    const int motorIndex = (int)strtol(cursor, (char **)&cursor, 10);
    if (motorIndex < 1 || motorIndex > MOTOR_COUNT) {
      return false;
    }

    while (*cursor && isspace((unsigned char)*cursor)) {
      cursor++;
    }
    if (*cursor != ':') {
      return false;
    }
    cursor++;

    while (*cursor && isspace((unsigned char)*cursor)) {
      cursor++;
    }
    if (!isdigit((unsigned char)*cursor)) {
      return false;
    }
    const int touchCount = (int)strtol(cursor, (char **)&cursor, 10);
    if (touchCount < 0) {
      return false;
    }

    counts[motorIndex - 1] = touchCount;

    while (*cursor && isspace((unsigned char)*cursor)) {
      cursor++;
    }
    if (*cursor == ';') {
      cursor++;
    } else if (*cursor != '\0') {
      return false;
    }
  }

  size_t used = 0;
  const int prefixLen = snprintf(outCommand, outSize, "RUN:");
  if (prefixLen < 0 || (size_t)prefixLen >= outSize) {
    return false;
  }
  used = (size_t)prefixLen;

  bool wroteAnyMotor = false;
  for (int motor = 1; motor <= MOTOR_COUNT; motor++) {
    for (int k = 0; k < counts[motor - 1]; k++) {
      const int written = snprintf(outCommand + used, outSize - used, "%s%d", wroteAnyMotor ? "," : "", motor);
      if (written < 0 || (size_t)written >= (outSize - used)) {
        return false;
      }
      used += (size_t)written;
      wroteAnyMotor = true;
    }
  }

  if (!wroteAnyMotor) {
    strncpy(outCommand, "STOP", outSize - 1);
    outCommand[outSize - 1] = '\0';
  }

  return true;
}

static void mqttCallback(char *topic, byte *payload, unsigned int length) {
  (void)topic;

  if (length == 0 || length >= 120) {
    Serial.println("MQTT payload invalid size");
    return;
  }

  char payloadText[120];
  memcpy(payloadText, payload, length);
  payloadText[length] = '\0';

  if (!parsePiPayloadToRunCommand(payloadText, pendingCommand, sizeof(pendingCommand))) {
    Serial.print("MQTT payload parse error: ");
    Serial.println(payloadText);
    return;
  }

  Serial.print("MQTT command queued: ");
  Serial.println(pendingCommand);
}

static bool ensureMqttConnected() {
  if (mqttClient.connected()) {
    return true;
  }

  const unsigned long now = millis();
  if (now - lastMqttReconnectTryMs < MQTT_RECONNECT_INTERVAL_MS) {
    return false;
  }
  lastMqttReconnectTryMs = now;

  bool connected = false;
  if (strlen(MQTT_USERNAME) > 0) {
    connected = mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);
  } else {
    connected = mqttClient.connect(MQTT_CLIENT_ID);
  }

  if (connected) {
    mqttClient.subscribe(MQTT_TOPIC_COMMAND);
    Serial.print("MQTT connected, subscribed to ");
    Serial.println(MQTT_TOPIC_COMMAND);
    return true;
  }

  Serial.print("MQTT connect failed, state=");
  Serial.println(mqttClient.state());
  return false;
}

void takeDataPiBegin() {
  memset(pendingCommand, 0, sizeof(pendingCommand));
  memset(returnedCommand, 0, sizeof(returnedCommand));

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected. ESP32 IP: ");
  Serial.println(WiFi.localIP());

  mqttClient.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORT);
  mqttClient.setCallback(mqttCallback);
  ensureMqttConnected();
}

bool takeDataPiHasCommand() {
  if (pendingCommand[0] != '\0') {
    return true;
  }

  if (ensureMqttConnected()) {
    mqttClient.loop();
  }

  return pendingCommand[0] != '\0';
}

const char* takeDataPiReadCommand() {
  if (pendingCommand[0] == '\0') {
    return "";
  }

  strncpy(returnedCommand, pendingCommand, sizeof(returnedCommand) - 1);
  returnedCommand[sizeof(returnedCommand) - 1] = '\0';
  memset(pendingCommand, 0, sizeof(pendingCommand));
  return returnedCommand;
}

bool takeDataPiConnected() {
  return WiFi.status() == WL_CONNECTED && mqttClient.connected();
}