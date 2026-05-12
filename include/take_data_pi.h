#pragma once

#include <Arduino.h>

void takeDataPiBegin();
bool takeDataPiHasCommand();
const char* takeDataPiReadCommand();
bool takeDataPiConnected();
