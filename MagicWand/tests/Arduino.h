#pragma once

#include <stdint.h>
#include <string>

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
  bool connected = true;
  int input = -1;
  std::string output;

  void begin(int) {}
  operator bool() const { return connected; }
  int available() const { return input >= 0 ? 1 : 0; }
  int read() {
    const int value = input;
    input = -1;
    return value;
  }
  void print(const char* text) {
    if (connected) output += text;
  }
  void println(const char* text) {
    if (connected) {
      output += text;
      output += '\n';
    }
  }
  void println(uint32_t value) {
    if (connected) {
      output += std::to_string(value);
      output += '\n';
    }
  }
};

extern TestSerial Serial;
