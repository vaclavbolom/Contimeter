#pragma once

#include <stdint.h>

const int LOW = 0;
const int HIGH = 1;
const int OUTPUT = 1;
const int INPUT_PULLUP = 2;

extern uint32_t testNow;
extern int pinValues[32];
extern bool buttonPressed;

inline uint32_t millis() { return testNow; }
inline void pinMode(int, int) {}
inline void digitalWrite(int pin, int value) { pinValues[pin] = value; }
inline int digitalRead(int) { return buttonPressed ? LOW : HIGH; }
inline void analogWrite(int pin, int value) { pinValues[pin] = value; }

struct TestSerial {
  void begin(int) {}
  void print(const char*) {}
  void println(const char*) {}
  void println(uint32_t) {}
};

extern TestSerial Serial;
