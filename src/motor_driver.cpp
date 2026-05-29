#include <Arduino.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "config_pins.h"
#include "motor_driver.h"
#include "IR_stage.h"

#define QUEUE_CAPACITY 24

static uint8_t motorQueue[QUEUE_CAPACITY];
static uint8_t queueHead = 0;
static uint8_t queueTail = 0;

static bool isRunning = false;
static uint8_t activeMotorIdx = 0;
static bool waitForPiezoRelease = false;

static bool queueIsEmpty() { return queueHead == queueTail; }
static bool queueIsFull() {
  return (uint8_t)((queueTail + 1) % QUEUE_CAPACITY) == queueHead;
}

static void clearQueue() { queueHead = queueTail = 0; }

static bool enqueue(uint8_t motorNum) {
  if (motorNum < 1 || motorNum > MOTOR_COUNT || queueIsFull()) {
    return false;
  }
  motorQueue[queueTail] = motorNum;
  queueTail = (uint8_t)((queueTail + 1) % QUEUE_CAPACITY);
  return true;
}

static bool dequeue(uint8_t *motorNum) {
  if (queueIsEmpty()) {
    return false;
  }
  *motorNum = motorQueue[queueHead];
  queueHead = (uint8_t)((queueHead + 1) % QUEUE_CAPACITY);
  return true;
}

static void stopMotorByIdx(uint8_t idx) {
  digitalWrite(MOTOR_PINS[idx].in1, LOW);
  digitalWrite(MOTOR_PINS[idx].in2, LOW);
  ledcWrite(MOTOR_PWM_CHANNELS[idx], 0);
}

static void startMotorByIdx(uint8_t idx) {
  digitalWrite(MOTOR_PINS[idx].in1, HIGH);
  digitalWrite(MOTOR_PINS[idx].in2, LOW);
  ledcWrite(MOTOR_PWM_CHANNELS[idx], MOTOR_PWM_DUTY);
}

static void stopAll() {
  for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
    stopMotorByIdx(i);
  }
}

static void beginNextMotorIfNeeded() {
  if (isRunning) {
    return;
  }
  uint8_t motorNum = 0;
  if (!dequeue(&motorNum)) {
    return;
  }
  activeMotorIdx = (uint8_t)(motorNum - 1);
  startMotorByIdx(activeMotorIdx);
  isRunning = true;
  waitForPiezoRelease = true;
  Serial.printf("START M%d\n", motorNum);
}

void motorDriverBegin() { //set up motors in1, in2, en for all motors
  for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
    pinMode(MOTOR_PINS[i].in1, OUTPUT);
    pinMode(MOTOR_PINS[i].in2, OUTPUT);
    pinMode(MOTOR_PINS[i].en, OUTPUT);

    ledcSetup(MOTOR_PWM_CHANNELS[i], MOTOR_PWM_FREQ_HZ, MOTOR_PWM_RES_BITS);
    ledcAttachPin(MOTOR_PINS[i].en, MOTOR_PWM_CHANNELS[i]);
  }

  stopAll();
  clearQueue();
  isRunning = false;
  waitForPiezoRelease = false;
}

void motorDriverQueueFromCommand(const char *command) {
  char cmd[256];
  strcpy(cmd, command);

  // trim leading/trailing spaces
  char *start = cmd;
  while (*start && isspace(*start)) start++;
  char *end = start + strlen(start) - 1;
  while (end > start && isspace(*end)) *end-- = '\0';

  // to upper case
  for (char *p = start; *p; p++) *p = toupper(*p);

  if (strcmp(start, "STOP") == 0 || strcmp(start, "RESET") == 0) {
    stopAll();
    clearQueue();
    isRunning = false;
    waitForPiezoRelease = false;
    Serial.println("OK STOPPED");
    return;
  }

  if (strncmp(start, "RUN", 3) == 0) {
    char *p = start + 3;
    while (*p && isspace(*p)) p++;
    if (*p == ':') {
      p++;
      while (*p && isspace(*p)) p++;
    }
    strcpy(cmd, p);
    start = cmd;
  }

  // replace , and ; with space
  for (char *p = start; *p; p++) {
    if (*p == ',' || *p == ';') *p = ' ';
  }

  bool addedAny = false;
  char *token = strtok(start, " ");
  while (token) {
    int motorNum = atoi(token);
    if (motorNum >= 1 && motorNum <= MOTOR_COUNT) {
      if (!enqueue((uint8_t)motorNum)) {
        Serial.println("ERR QUEUE_FULL");
        return;
      }
      addedAny = true;
    }
    token = strtok(NULL, " ");
  }

  if (!addedAny) {
    Serial.println("ERR BAD_CMD");
    return;
  }

  Serial.println("OK QUEUED");
}

void motorDriverUpdate() {
  beginNextMotorIfNeeded();

  if (!isRunning) {
    return;
  }

  // Shared single sensor for all motors:
  // ignore an already-held LOW at motor start, then stop on the next touch.
  if (waitForPiezoRelease) {
    if (!IRStageTriggered()) {
      waitForPiezoRelease = false;
    }
    return;
  }

  if (IRStageTriggered()) {
    stopMotorByIdx(activeMotorIdx);
    Serial.printf("DONE M%d\n", activeMotorIdx + 1);
    isRunning = false;
    waitForPiezoRelease = false;
  }
}

void motorDriverUnitTest() {
  // =========================
  // TEST 1: Forward
  // =========================
  Serial.println("TEST 1: Forward");

  for (int i = 0; i < MOTOR_COUNT; i++) {
    digitalWrite(MOTOR_PINS[i].in1, HIGH);
    digitalWrite(MOTOR_PINS[i].in2, LOW);
    ledcWrite(MOTOR_PWM_CHANNELS[i], 180);
  }

  // =========================
  // TEST 2: Stop
  // =========================
  Serial.println("TEST 2: Stop");
  for (int i = 0; i < MOTOR_COUNT; i++) {
    ledcWrite(MOTOR_PWM_CHANNELS[i], 0);
  }

  delay(2000);

  // =========================
  // TEST 3: Reverse
  // =========================
  Serial.println("TEST 3: Reverse");

  for (int i = 0; i < MOTOR_COUNT; i++) {
    digitalWrite(MOTOR_PINS[i].in1, LOW);
    digitalWrite(MOTOR_PINS[i].in2, HIGH);
  }

  for (int i = 0; i < MOTOR_COUNT; i++) {
    ledcWrite(MOTOR_PWM_CHANNELS[i], 180);
  }

  delay(3000);

  // =========================
  // TEST 4: Speed Ramp
  // =========================
  Serial.println("TEST 4: Speed Ramp");

  for (int i = 0; i < MOTOR_COUNT; i++) {
    digitalWrite(MOTOR_PINS[i].in1, HIGH);
    digitalWrite(MOTOR_PINS[i].in2, LOW);
  }

  for (int speed = 0; speed <= 255; speed += 5) {
    for (int i = 0; i < MOTOR_COUNT; i++) {
      ledcWrite(MOTOR_PWM_CHANNELS[i], speed);
    }
    Serial.print("PWM: ");
    Serial.println(speed);
    delay(100);
  }

  // Stop
  for (int i = 0; i < MOTOR_COUNT; i++) {
    ledcWrite(MOTOR_PWM_CHANNELS[i], 0);
  }

  delay(3000);
}
