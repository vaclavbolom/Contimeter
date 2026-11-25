// ESP32-C3 Super Mini
// Board: "ESP32C3 Dev Module", USB CDC On Boot: "Enabled"

#include "ButtonGestures.h"

const int UV_PIN = 8;  // External UV driver, active HIGH
const int BUTTON_PIN = 4;  // Button between GPIO and GND
const int R_PIN = 2;
const int G_PIN = 3;
const int B_PIN = 1;
const int TEST_PIN = 0;

const uint32_t DEBOUNCE_MS = 20;
const uint32_t SHORT_PRESS_MAX_MS = 150;
const uint32_t MORSE_DOT_MS = 300;
const uint32_t MORSE_DASH_MS = 3 * MORSE_DOT_MS;
const uint32_t MORSE_LETTER_GAP_MS = 5 * MORSE_DOT_MS;
const uint32_t MORSE_TOLERANCE_MS = 320;
const uint32_t MORSE_SEQUENCE_GAP_MS = 700;
const uint32_t COLOR_HOLD_MS = 5000;
const uint32_t COLOR_STEP_MS = 20;
const char MORSE_NAME[] = "STEPANKA";

bool colorCycling = false;
bool rgbEnabled = false;
bool manualMode = false;
bool lastButtonPressed = false;
uint32_t buttonChangedAt = 0;
uint16_t colorStep = 510;
uint32_t colorChangedAt = 0;
uint8_t selectedRed = 0;
uint8_t selectedGreen = 255;
uint8_t selectedBlue = 0;

const char* const MORSE_ALPHABET[] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---",
  "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-",
  "..-", "...-", ".--", "-..-", "-.--", "--.."
};
bool morseActive = false;
bool morseLit = false;
const char* morseText = MORSE_NAME;
size_t morseLetter = 0;
size_t morseSymbol = 0;
uint32_t morseChangedAt = 0;
uint32_t morseDuration = 0;

void setRgb(uint8_t red, uint8_t green, uint8_t blue) {
  analogWrite(R_PIN, red);
  analogWrite(G_PIN, green);
  analogWrite(B_PIN, blue);
}

void showSelectedColor() {
  setRgb(selectedRed, selectedGreen, selectedBlue);
}

void restoreRgb() {
  if (rgbEnabled) {
    showSelectedColor();
  } else {
    setRgb(0, 0, 0);
  }
}

void stopMorse() {
  morseActive = false;
  morseLit = false;
  restoreRgb();
}

void allOff() {
  manualMode = false;
  colorCycling = false;
  rgbEnabled = false;
  stopMorse();
  digitalWrite(UV_PIN, LOW);
  digitalWrite(TEST_PIN, LOW);
  Serial.println("All LEDs off");
}

void uvOn() {
  stopMorse();
  digitalWrite(UV_PIN, HIGH);
  Serial.println("UV on");
}

void startColors() {
  stopMorse();
  rgbEnabled = true;
  colorCycling = true;
  colorChangedAt = millis();
  showSelectedColor();
  Serial.println("Selecting RGB color");
}

void holdReleased() {
  if (colorCycling) {
    colorCycling = false;
    Serial.println("RGB color selected");
  }
}

void updateColors(uint32_t now) {
  const uint32_t steps = (now - colorChangedAt) / COLOR_STEP_MS;
  if (!colorCycling || steps == 0) {
    return;
  }
  colorChangedAt += steps * COLOR_STEP_MS;
  colorStep = (colorStep + steps % 1530) % 1530;
  const uint8_t rising = colorStep % 255;
  const uint8_t falling = 255 - rising;
  switch (colorStep / 255) {
    case 0: selectedRed = 255; selectedGreen = rising; selectedBlue = 0; break;
    case 1: selectedRed = falling; selectedGreen = 255; selectedBlue = 0; break;
    case 2: selectedRed = 0; selectedGreen = 255; selectedBlue = rising; break;
    case 3: selectedRed = 0; selectedGreen = falling; selectedBlue = 255; break;
    case 4: selectedRed = rising; selectedGreen = 0; selectedBlue = 255; break;
    case 5: selectedRed = 255; selectedGreen = 0; selectedBlue = falling; break;
  }
  showSelectedColor();
}

char currentMorseSymbol() {
  return MORSE_ALPHABET[morseText[morseLetter] - 'A'][morseSymbol];
}

void lightMorseSymbol() {
  morseLit = true;
  morseDuration = currentMorseSymbol() == '.' ? MORSE_DOT_MS : MORSE_DASH_MS;
  showSelectedColor();
}

void startMorse(const char* text) {
  for (size_t i = 0; text[i]; ++i) {
    if (text[i] < 'A' || text[i] > 'Z') {
      Serial.println("Cannot transmit Morse: use uppercase A-Z only");
      return;
    }
  }
  if (!text[0]) {
    Serial.println("Cannot transmit empty Morse text");
    return;
  }
  colorCycling = false;
  morseActive = true;
  morseText = text;
  morseLetter = 0;
  morseSymbol = 0;
  morseChangedAt = millis();
  lightMorseSymbol();
  Serial.print("Transmitting ");
  Serial.println(text);
}

void transmitName() {
  startMorse(MORSE_NAME);
}

void startManualMode() {
  manualMode = true;
  rgbEnabled = false;
  startMorse("M");
  Serial.println("Manual mode on");
}

void updateMorse(uint32_t now) {
  while (morseActive && now - morseChangedAt >= morseDuration) {
    morseChangedAt += morseDuration;
    if (!morseLit) {
      if (!morseText[morseLetter]) {
        stopMorse();
        Serial.println("Morse transmission complete");
        continue;
      }
      lightMorseSymbol();
      continue;
    }
    morseLit = false;
    setRgb(0, 0, 0);
    ++morseSymbol;
    if (currentMorseSymbol()) {
      morseDuration = MORSE_DOT_MS;
    } else {
      ++morseLetter;
      morseSymbol = 0;
      morseDuration = morseText[morseLetter] ? MORSE_LETTER_GAP_MS
                                            : 3 * MORSE_DOT_MS;
    }
  }
}

const ButtonGestures::Timing GESTURE_TIMING = {
  DEBOUNCE_MS, SHORT_PRESS_MAX_MS, MORSE_DOT_MS, MORSE_DASH_MS,
  MORSE_TOLERANCE_MS, MORSE_SEQUENCE_GAP_MS
};
const ButtonGestures::Hold HOLD_GESTURES[] = {
  {COLOR_HOLD_MS, startColors}
};
const ButtonGestures::Sequence MORSE_GESTURES[] = {
  {"...", transmitName},
  {"-", uvOn},
  {"--", startManualMode}
};
ButtonGestures button(
  GESTURE_TIMING,
  HOLD_GESTURES, sizeof(HOLD_GESTURES) / sizeof(HOLD_GESTURES[0]),
  MORSE_GESTURES, sizeof(MORSE_GESTURES) / sizeof(MORSE_GESTURES[0]),
  allOff, holdReleased
);
ButtonGestures manualButton(GESTURE_TIMING, nullptr, 0, nullptr, 0,
                            allOff, nullptr);

void setup() {
  Serial.begin(115200);
  Serial.println("Activating magic wand");
  pinMode(UV_PIN, OUTPUT);
  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  pinMode(TEST_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  allOff();
  Serial.println("Magic wand activated");
}

void loop() {
  const bool pressed = digitalRead(BUTTON_PIN) == LOW;
  const uint32_t now = millis();
  if (pressed != lastButtonPressed) {
    lastButtonPressed = pressed;
    buttonChangedAt = now;
  }
  const bool wasManual = manualMode;
  if (!manualMode) {
    button.update(pressed, now);
  }
  if (manualMode) {
    if (!wasManual) {
      manualButton = ButtonGestures(GESTURE_TIMING, nullptr, 0, nullptr, 0,
                                    allOff, nullptr);
      if (pressed) {
        manualButton.update(true, buttonChangedAt);
      }
    }
    manualButton.update(pressed, now);
    if (!manualMode) {
      button = ButtonGestures(
          GESTURE_TIMING,
          HOLD_GESTURES, sizeof(HOLD_GESTURES) / sizeof(HOLD_GESTURES[0]),
          MORSE_GESTURES, sizeof(MORSE_GESTURES) / sizeof(MORSE_GESTURES[0]),
          allOff, holdReleased);
    } else if (manualButton.isPressed()) {
      if (morseActive) {
        stopMorse();
      }
      showSelectedColor();
    } else if (morseActive) {
      updateMorse(now);
    } else {
      setRgb(0, 0, 0);
    }
  } else {
    updateColors(now);
    updateMorse(now);
  }
}
