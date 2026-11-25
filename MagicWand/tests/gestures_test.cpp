#include "Arduino.h"
#include "sketch_magic_wand.ino"

#include <assert.h>
#include <stdio.h>

uint32_t testNow = 0;
int pinValues[32] = {};
bool buttonPressed = false;
TestSerial Serial;

void tick(uint32_t now, bool pressed = false) {
  testNow = now;
  buttonPressed = pressed;
  loop();
}

void press(uint32_t now) {
  tick(now, true);
  tick(now + DEBOUNCE_MS, true);
}

void release(uint32_t now) {
  tick(now);
  tick(now + DEBOUNCE_MS);
}

void reset() {
  testNow = 0;
  buttonPressed = false;
  lastButtonPressed = false;
  buttonChangedAt = 0;
  serialWasConnected = false;
  Serial.connected = true;
  Serial.input = -1;
  Serial.output.clear();
  colorCycling = false;
  rgbEnabled = false;
  colorStep = 510;
  colorChangedAt = 0;
  selectedRed = 0;
  selectedGreen = 255;
  selectedBlue = 0;
  button = ButtonGestures(
      GESTURE_TIMING,
      HOLD_GESTURES, sizeof(HOLD_GESTURES) / sizeof(HOLD_GESTURES[0]),
      MORSE_GESTURES, sizeof(MORSE_GESTURES) / sizeof(MORSE_GESTURES[0]),
      allOff, holdReleased);
  setup();
}

void expectColor(int red, int green, int blue) {
  assert(pinValues[R_PIN] == red);
  assert(pinValues[G_PIN] == green);
  assert(pinValues[B_PIN] == blue);
}

uint32_t enterDots(uint32_t start, size_t count);

void testInitialColor() {
  setup();
  expectColor(0, 0, 0);
  const uint32_t end = enterDots(100, 3);
  tick(end + MORSE_SEQUENCE_GAP_MS);
  expectColor(0, 255, 0);
}

void testStartupSerial() {
  reset();
  serialWasConnected = false;
  Serial.connected = false;
  Serial.output.clear();
  press(100);
  tick(100 + COLOR_HOLD_MS, true);
  assert(colorCycling);
  assert(Serial.output.empty());

  Serial.connected = true;
  tick(100 + COLOR_HOLD_MS + 1, true);
  assert(Serial.output.find("Magic wand activated\n") != std::string::npos);
  assert(Serial.output.find("Startup: all LEDs initialized off\n") !=
         std::string::npos);
  Serial.output.clear();
  tick(100 + COLOR_HOLD_MS + 2, true);
  assert(Serial.output.empty());
  Serial.input = '?';
  tick(100 + COLOR_HOLD_MS + 3, true);
  assert(Serial.output.find("Magic wand activated\n") != std::string::npos);

  Serial.output.clear();
  Serial.connected = false;
  tick(100 + COLOR_HOLD_MS + 4, true);
  Serial.connected = true;
  tick(100 + COLOR_HOLD_MS + 5, true);
  assert(Serial.output.find("Magic wand activated\n") != std::string::npos);
  assert(colorCycling);
}

void testHoldThresholdsAndFreeze() {
  reset();
  expectColor(0, 0, 0);
  assert(pinValues[UV_PIN] == LOW);
  press(100);
  tick(2100, true);
  assert(pinValues[UV_PIN] == LOW);
  tick(5099, true);
  assert(!colorCycling);
  tick(5100, true);
  assert(colorCycling);
  assert(pinValues[UV_PIN] == LOW);
  expectColor(0, 255, 0);
  tick(5500, true);
  expectColor(0, 255, 20);
  release(5501);
  assert(!colorCycling);
  const int blue = selectedBlue;
  tick(9000);
  expectColor(0, 255, blue);
  assert(pinValues[UV_PIN] == LOW);

  press(9100);
  release(9200);
  expectColor(0, 0, 0);
  assert(pinValues[UV_PIN] == LOW);
  assert(pinValues[TEST_PIN] == LOW);
  assert(selectedBlue == blue);
}

void testReleaseDebounceDoesNotExtendHold() {
  reset();
  press(100);
  release(100 + COLOR_HOLD_MS - 1);
  assert(pinValues[UV_PIN] == LOW);
  assert(!colorCycling);
}

void testMorseTActivatesUv() {
  reset();
  press(100);
  release(3100);
  tick(3100 + MORSE_SEQUENCE_GAP_MS);
  assert(pinValues[UV_PIN] == LOW);

  reset();
  press(100);
  const uint32_t end = 100 + MORSE_DASH_MS;
  release(end);
  assert(pinValues[UV_PIN] == LOW);
  tick(end + MORSE_SEQUENCE_GAP_MS - 1);
  assert(pinValues[UV_PIN] == LOW);
  tick(end + MORSE_SEQUENCE_GAP_MS);
  assert(pinValues[UV_PIN] == HIGH);
  press(end + MORSE_SEQUENCE_GAP_MS + 100);
  release(end + MORSE_SEQUENCE_GAP_MS + 200);
  assert(pinValues[UV_PIN] == LOW);

  reset();
  press(100);
  release(100 + MORSE_DASH_MS);
  press(100 + MORSE_DASH_MS + 200);
  release(100 + 2 * MORSE_DASH_MS + 200);
  tick(100 + 2 * MORSE_DASH_MS + 200 + MORSE_SEQUENCE_GAP_MS);
  assert(pinValues[UV_PIN] == LOW);

  reset();
  transmitName();
  press(100);
  release(100 + MORSE_DASH_MS);
  tick(100 + MORSE_DASH_MS + MORSE_SEQUENCE_GAP_MS);
  assert(pinValues[UV_PIN] == HIGH);
  assert(!morseActive);
}

void testBounceAndOffBoundary() {
  reset();
  digitalWrite(UV_PIN, HIGH);
  tick(100, true);
  tick(105);
  tick(130);
  assert(pinValues[UV_PIN] == HIGH);
  press(200);
  release(200 + SHORT_PRESS_MAX_MS);
  assert(pinValues[UV_PIN] == HIGH);
  press(500);
  release(500 + SHORT_PRESS_MAX_MS - 1);
  assert(pinValues[UV_PIN] == LOW);
}

uint32_t enterDots(uint32_t start, size_t count) {
  uint32_t lastRelease = start;
  for (size_t i = 0; i < count; ++i) {
    press(start);
    lastRelease = start + MORSE_DOT_MS;
    release(lastRelease);
    start = lastRelease + 200;
  }
  return lastRelease;
}

void testExactMorseSequenceAndTimingTolerance() {
  reset();
  uint32_t end = enterDots(100, 4);
  tick(end + MORSE_SEQUENCE_GAP_MS);
  assert(!morseActive);

  reset();
  end = enterDots(100, 11);
  tick(end + MORSE_SEQUENCE_GAP_MS);
  assert(!morseActive);

  reset();
  const uint32_t durations[] = {180, 420, 300};
  uint32_t start = 100;
  for (uint32_t duration : durations) {
    press(start);
    end = start + duration;
    release(end);
    start = end + 200;
  }
  tick(end + MORSE_SEQUENCE_GAP_MS - 1);
  assert(!morseActive);
  tick(end + MORSE_SEQUENCE_GAP_MS);
  assert(morseActive);
}

void testNamePlaybackAndCancellation() {
  reset();
  // Select a non-default color before entering the gesture.
  press(100);
  tick(5100, true);
  tick(5700, true);
  release(5701);
  const int red = selectedRed;
  const int green = selectedGreen;
  const int blue = selectedBlue;
  const uint32_t end = enterDots(6000, 3);
  uint32_t now = end + MORSE_SEQUENCE_GAP_MS;
  tick(now);
  assert(morseActive);

  const char* expected[] = {"...", "-", ".", ".--.", ".-", "-.", "-.-", ".-"};
  for (size_t letter = 0; letter < 8; ++letter) {
    for (size_t symbol = 0; expected[letter][symbol]; ++symbol) {
      expectColor(red, green, blue);
      const uint32_t duration =
          expected[letter][symbol] == '.' ? MORSE_DOT_MS : MORSE_DASH_MS;
      tick(now + duration - 1);
      expectColor(red, green, blue);
      now += duration;
      tick(now);
      expectColor(0, 0, 0);
      const uint32_t gap = expected[letter][symbol + 1]
                              ? MORSE_DOT_MS
                              : (letter < 7 ? 5 * MORSE_DOT_MS
                                            : 3 * MORSE_DOT_MS);
      tick(now + gap - 1);
      expectColor(0, 0, 0);
      now += gap;
      tick(now);
    }
  }
  assert(!morseActive);
  expectColor(red, green, blue);

  transmitName();
  press(now + 100);
  release(now + 200);
  assert(!morseActive);
  tick(now + 10000);
  expectColor(0, 0, 0);
  assert(pinValues[UV_PIN] == LOW);
}

uint32_t enterManualMode(uint32_t start) {
  press(start);
  release(start + MORSE_DASH_MS);
  const uint32_t second = start + MORSE_DASH_MS + 200;
  press(second);
  const uint32_t end = second + MORSE_DASH_MS;
  release(end);
  assert(!manualMode);
  tick(end + MORSE_SEQUENCE_GAP_MS);
  assert(manualMode);
  return end + MORSE_SEQUENCE_GAP_MS;
}

void testManualMode() {
  reset();
  digitalWrite(UV_PIN, HIGH);
  const uint32_t start = enterManualMode(100);
  expectColor(0, 255, 0);
  tick(start + MORSE_DASH_MS - 1);
  expectColor(0, 255, 0);
  tick(start + MORSE_DASH_MS);
  expectColor(0, 0, 0);
  tick(start + MORSE_DASH_MS + MORSE_DOT_MS);
  expectColor(0, 255, 0);
  const uint32_t secondEnd = start + 2 * MORSE_DASH_MS + MORSE_DOT_MS;
  tick(secondEnd);
  expectColor(0, 0, 0);
  tick(secondEnd + 3 * MORSE_DOT_MS);
  assert(manualMode);
  assert(!morseActive);
  assert(pinValues[UV_PIN] == HIGH);

  const uint32_t holdStart = secondEnd + 3 * MORSE_DOT_MS + 100;
  press(holdStart);
  expectColor(0, 255, 0);
  tick(holdStart + COLOR_HOLD_MS + 100, true);
  assert(!colorCycling);
  assert(pinValues[UV_PIN] == HIGH);
  expectColor(0, 255, 0);
  release(holdStart + COLOR_HOLD_MS + 200);
  assert(manualMode);
  expectColor(0, 0, 0);

  press(holdStart + COLOR_HOLD_MS + 400);
  release(holdStart + COLOR_HOLD_MS + 500);
  assert(!manualMode);
  assert(pinValues[UV_PIN] == LOW);
  expectColor(0, 0, 0);

  const uint32_t next = holdStart + COLOR_HOLD_MS + 800;
  press(next);
  release(next + MORSE_DASH_MS);
  tick(next + MORSE_DASH_MS + MORSE_SEQUENCE_GAP_MS);
  assert(pinValues[UV_PIN] == HIGH);
}

void testManualInterruptAndTransition() {
  reset();
  const uint32_t start = enterManualMode(100);
  press(start + 100);
  assert(!morseActive);
  expectColor(0, 255, 0);
  release(start + 200);
  assert(!manualMode);
  assert(!morseActive);
  expectColor(0, 0, 0);

  reset();
  press(100);
  release(100 + MORSE_DASH_MS);
  press(100 + MORSE_DASH_MS + 200);
  const uint32_t end = 100 + 2 * MORSE_DASH_MS + 200;
  release(end);
  const uint32_t next = end + MORSE_SEQUENCE_GAP_MS + 1;
  press(next);
  assert(manualMode);
  expectColor(0, 255, 0);
  release(next + SHORT_PRESS_MAX_MS);
  assert(manualMode);
  expectColor(0, 0, 0);
  press(next + 300);
  release(next + 300 + SHORT_PRESS_MAX_MS - 1);
  assert(!manualMode);
  expectColor(0, 0, 0);
}

void testMillisRollover() {
  reset();
  const uint32_t start = UINT32_MAX - 1000;
  press(start);
  tick(start + COLOR_HOLD_MS - 1, true);
  assert(!colorCycling);
  tick(start + COLOR_HOLD_MS, true);
  assert(colorCycling);
  assert(pinValues[UV_PIN] == LOW);
  release(start + COLOR_HOLD_MS + 100);

  reset();
  const uint32_t end = enterDots(UINT32_MAX - 800, 3);
  tick(end + MORSE_SEQUENCE_GAP_MS);
  assert(morseActive);
  tick(end + MORSE_SEQUENCE_GAP_MS + MORSE_DOT_MS);
  expectColor(0, 0, 0);
}

int firstHoldCount = 0;
int secondHoldCount = 0;
int sequenceCount = 0;
void firstHold() { ++firstHoldCount; }
void secondHold() { ++secondHoldCount; }
void sequenceAction() { ++sequenceCount; }

void testExtensibleBindings() {
  const ButtonGestures::Hold holds[] = {{2000, firstHold}, {5000, secondHold}};
  const ButtonGestures::Sequence sequences[] = {{".-", sequenceAction}};
  ButtonGestures gestures(GESTURE_TIMING, holds, 2, sequences, 1, nullptr, nullptr);
  const auto edge = [&gestures](bool pressed, uint32_t now) {
    gestures.update(pressed, now);
    gestures.update(pressed, now + DEBOUNCE_MS);
  };
  edge(true, 100);
  gestures.update(true, 5100);
  gestures.update(true, 6000);
  edge(false, 6100);
  assert(firstHoldCount == 1 && secondHoldCount == 1);
  edge(true, 7000);
  edge(false, 7300);
  edge(true, 7500);
  edge(false, 8400);
  gestures.update(false, 9100);
  assert(sequenceCount == 1);
}

int main() {
  testInitialColor();
  testStartupSerial();
  testHoldThresholdsAndFreeze();
  testReleaseDebounceDoesNotExtendHold();
  testMorseTActivatesUv();
  testBounceAndOffBoundary();
  testExactMorseSequenceAndTimingTolerance();
  testNamePlaybackAndCancellation();
  testManualMode();
  testManualInterruptAndTransition();
  testMillisRollover();
  testExtensibleBindings();
  puts("12 gesture, LED, and serial behavior tests passed");
}
