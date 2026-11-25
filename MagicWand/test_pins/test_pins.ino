const int LEDS[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 21};
const int LED_COUNT = sizeof(LEDS) / sizeof(LEDS[0]);

unsigned long previous = 0;
const unsigned long BLINK_INTERVAL = 500;
bool led_on = false;

void blink_led(const int* pins, int count) {
  unsigned long current = millis();

  if (current - previous >= BLINK_INTERVAL) {
    previous = current;
    led_on = !led_on;

    for (int i = 0; i < count; i++) {
      digitalWrite(pins[i], led_on ? HIGH : LOW);
    }
  }
}

void setup() {
  for (int i = 0; i < LED_COUNT; i++) {
    pinMode(LEDS[i], OUTPUT);
  }
}

void loop() {
  blink_led(LEDS, LED_COUNT);
}
