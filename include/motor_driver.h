#pragma once

#include <Arduino.h>

void motorDriverBegin();
void motorDriverUpdate();
void motorDriverQueueFromCommand(const char *command);
void motorDriverUnitTest();