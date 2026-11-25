# Magic wand

Open `sketch_magic_wand/sketch_magic_wand.ino` in Arduino IDE. Select
**ESP32C3 Dev Module** and enable **USB CDC On Boot**.

## Wiring

The sketch uses active-HIGH outputs: UV GPIO 8, common-cathode RGB
GPIOs 2 (red), 3 (green), and 4 (blue). Connect the button between
GPIO 1 and GND; the internal pull-up is enabled. GPIO 0 (the previous
test indicator) stays LOW, so it does not blink after an off gesture.
Edit the pin constants to match your hardware.

Use current-limiting resistors for RGB channels and a suitable external
driver for the UV LED; do not power a high-current LED directly from a GPIO.
The onboard GPIO 8 indicator on many Super Mini boards is active-LOW;
this sketch instead follows the external UV driver's active-HIGH polarity.
Avoid looking directly at UV light or exposing skin.

## Gestures

| Button action | Result |
| --- | --- |
| Press for less than 150 ms | Immediately on debounced release, stop playback and turn all controlled LEDs off |
| Enter `-` (Morse T) | UV turns on after the input sequence gap |
| Hold for 5 seconds | RGB starts cycling; release to keep its current color |
| Enter `...` (Morse S) | Flash `STEPANKA` using the last selected RGB color |
| Enter `--` (Morse M) | Enter manual mode, confirmed by flashing `--` |

In manual mode the RGB LED lights in the selected color while the button
is held and turns off on release. A press shorter than 150 ms turns all LEDs
off and exits manual mode. Other gestures are disabled until manual mode
exits; UV retains its previous state until the off gesture. A button press
during the confirmation interrupts the flashes and starts manual control.

UV remains on during color selection and Morse playback until the off
gesture. Off remembers the selected color. Before any color selection,
Morse uses green. After playback, RGB returns to its previous steady on/off
state. Morse T cancels playback when recognized, and a hold cancels playback
when it reaches the 5-second threshold; an off gesture cancels it immediately
on release.

Timing constants are at the top of the sketch:

- Debounce: 20 ms.
- Dot: 300 ms; dash: 900 ms. Input tolerance: +/-320 ms.
- Pauses between input symbols must be shorter than 700 ms.
  After 700 ms without another press, the complete sequence is matched.
- Output pauses: one dot between symbols, five dots (1,500 ms) between
  letters, and three dots after the final letter before restoring the
  steady color. Adjust `MORSE_LETTER_GAP_MS` to change the letter gap.
- Color changes every 20 ms.

Very short presses are **not** Morse dots. Unrecognized durations or
sequences do nothing and clear the pending sequence; they never turn off
LEDs accidentally. Four dots do not match S.

## Extending gestures

`ButtonGestures.h` handles debouncing and recognition independently of
LED behavior. Call `update(pressed, millis())` frequently from `loop()`;
no interrupts or blocking delays are needed.

Add hold callbacks to `HOLD_GESTURES`, in increasing duration order.
Every crossed hold threshold fires once per press. Add Morse patterns
such as `".-"` and their callbacks to `MORSE_GESTURES`; patterns can
contain up to eight symbols. Overlong input is discarded as a whole.
`MORSE_NAME` accepts uppercase A-Z.

## Validation

Compile using Arduino CLI:

```powershell
arduino-cli compile --fqbn esp32:esp32:esp32c3:CDCOnBoot=cdc --warnings all .\MagicWand\sketch_magic_wand
```

The host tests include the actual sketch with simulated Arduino I/O and
time. From the repository root, using a C++11 or newer compiler:

```powershell
g++ -std=c++11 -Wall -Wextra -Werror -I .\MagicWand\tests -I .\MagicWand\sketch_magic_wand .\MagicWand\tests\gestures_test.cpp -o "$env:TEMP\magic-wand-tests.exe"
& "$env:TEMP\magic-wand-tests.exe"
```

These check gesture thresholds, bounce, frozen colors, off behavior,
exact sequence matching, extensibility, timer rollover, and every Morse
flash and pause in `STEPANKA`. They do not replace testing on the wired board.
