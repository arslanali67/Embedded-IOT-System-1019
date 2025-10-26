
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// tell the compiler that OLED does not have a physical reset pin
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------------- LED PINS ----------------
// define LED pins connected to the ESP32 board
#define LED_YELLOW 18
#define LED_GREEN 17
#define LED_RED 16

// ---------------- BUTTON PINS ----------------
// define pins for buttons used to cycle and reset LED modes
#define BTN_CYCLE 25
#define BTN_RESET 26

// ---------------- BUTTON DEBOUNCE VARIABLES ----------------
// stores the **current stable state** of the cycle button — after it has stopped bouncing
int stableButtonState = HIGH;
// used to compare and detect button changes — stores the last read value
int lastReading = HIGH;
// stores the **previous stable state** of the button
int lastStableState = HIGH;
// store the time when the button last changed state
unsigned long lastDebounceTime = 0;
// debounce delay — how long to wait before confirming a button press
const unsigned long DEBOUNCE_MS = 30;

// similar variables for the reset button
int stableResetState = HIGH;
int lastResetReading = HIGH;
int lastStableResetState = HIGH;
unsigned long lastResetDebounceTime = 0;

// ---------------- LED MODE VARIABLES ----------------
// LED state: 0 = ALL OFF, 1 = ALL ON, 2 = BLINK, 3 = PWM FADE
int ledMode = 0;

// ---------------- BLINK VARIABLES ----------------
// store the last time the LED blinked
unsigned long lastBlinkTime = 0;
// interval between LED blinks (in milliseconds)
const unsigned long BLINK_INTERVAL = 500;
// keeps track of current ON/OFF blink state
bool blinkState = false;

// ---------------- PWM FADE VARIABLES ----------------
// stores current LED brightness level (0 = OFF, 255 = full brightness)
int fadeValue = 0;
// direction of fade: 1 = fading up, -1 = fading down
int fadeDirection = 1;
// stores the last time the brightness was updated
unsigned long lastFadeTime = 0;
// time between each brightness update (smaller = smoother fade)
const unsigned long FADE_INTERVAL = 10;

void setup()
{
  // set button pins as input with internal pull-up resistors
  pinMode(BTN_CYCLE, INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP);

  // set LED pins as outputs
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // ------------- INITIALIZE OLED DISPLAY -------------
  // initialize display and check if it connected properly
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    for (;;)
      ; // stop if display is not found
  }

  // clearDisplay() → clears the OLED screen to black
  display.clearDisplay();
  // setTextSize(2) → doubles the font size
  display.setTextSize(2);
  // setTextColor(SSD1306_WHITE) → sets text color to white
  display.setTextColor(SSD1306_WHITE);

  // initialize button states at startup
  lastReading = digitalRead(BTN_CYCLE);
  stableButtonState = lastReading;
  lastStableState = stableButtonState;

  lastResetReading = digitalRead(BTN_RESET);
  stableResetState = lastResetReading;
  lastStableResetState = stableResetState;

  // make sure all LEDs start OFF
  analogWrite(LED_YELLOW, 0);
  analogWrite(LED_GREEN, 0);
  analogWrite(LED_RED, 0);
}

void loop()
{
  unsigned long currentMillis = millis(); // current time (used for timing events)

  // -------- HANDLE BTN_CYCLE (MODE SWITCH) --------
  int reading = digitalRead(BTN_CYCLE);
  // check if button state has changed
  if (reading != lastReading)
  {
    lastDebounceTime = currentMillis; // reset debounce timer
    lastReading = reading;
  }
  // if stable for enough time, confirm the change
  if (currentMillis - lastDebounceTime >= DEBOUNCE_MS)
  {
    if (stableButtonState != reading)
    {
      stableButtonState = reading;
      // detect falling edge (HIGH → LOW) = button press
      if (lastStableState == HIGH && stableButtonState == LOW)
      {
        ledMode++; // move to next LED mode
        if (ledMode > 3)
          ledMode = 0; // wrap back to first mode
        // reset variables for blink/fade
        fadeValue = 0;
        fadeDirection = 1;
        blinkState = false;
        lastBlinkTime = currentMillis;
        lastFadeTime = currentMillis;
      }
      lastStableState = stableButtonState;
    }
  }

  // -------- HANDLE BTN_RESET (RESET TO OFF) --------
  int resetReading = digitalRead(BTN_RESET);
  if (resetReading != lastResetReading)
  {
    lastResetDebounceTime = currentMillis;
    lastResetReading = resetReading;
  }
  if (currentMillis - lastResetDebounceTime >= DEBOUNCE_MS)
  {
    if (stableResetState != resetReading)
    {
      stableResetState = resetReading;
      if (lastStableResetState == HIGH && stableResetState == LOW)
      {
        // reset everything to OFF mode
        ledMode = 0;
        analogWrite(LED_YELLOW, 0);
        analogWrite(LED_GREEN, 0);
        analogWrite(LED_RED, 0);
      }
      lastStableResetState = stableResetState;
    }
  }

  // -------- HANDLE LED MODES --------
  switch (ledMode)
  {
  case 0: // ALL OFF
    analogWrite(LED_YELLOW, 0);
    analogWrite(LED_GREEN, 0);
    analogWrite(LED_RED, 0);
    break;

  case 1: // ALL ON
    analogWrite(LED_YELLOW, 255);
    analogWrite(LED_GREEN, 255);
    analogWrite(LED_RED, 255);
    break;

  case 2: // ALTERNATE BLINK
    // toggle blink state every BLINK_INTERVAL ms
    if (currentMillis - lastBlinkTime >= BLINK_INTERVAL)
    {
      blinkState = !blinkState;
      lastBlinkTime = currentMillis;
    }
    // alternate ON/OFF pattern
    analogWrite(LED_YELLOW, blinkState ? 255 : 0);
    analogWrite(LED_GREEN, blinkState ? 0 : 255);
    analogWrite(LED_RED, blinkState ? 255 : 0);
    break;

  case 3: // PWM FADE
    // update brightness every FADE_INTERVAL ms
    if (currentMillis - lastFadeTime >= FADE_INTERVAL)
    {
      fadeValue += fadeDirection * 5; // increase or decrease brightness
      // reverse direction at limits
      if (fadeValue >= 255)
      {
        fadeValue = 255;
        fadeDirection = -1;
      }
      if (fadeValue <= 0)
      {
        fadeValue = 0;
        fadeDirection = 1;
      }
      // apply brightness to all LEDs
      analogWrite(LED_YELLOW, fadeValue);
      analogWrite(LED_GREEN, fadeValue);
      analogWrite(LED_RED, fadeValue);
      lastFadeTime = currentMillis;
    }
    break;
  }

  // -------- UPDATE OLED DISPLAY --------
  display.clearDisplay();   // clear old text
  display.setCursor(0, 20); // set text position
  // print current LED mode to OLED
  switch (ledMode)
  {
  case 0:
    display.println("ALL OFF");
    break;
  case 1:
    display.println("ALL ON");
    break;
  case 2:
    display.println("BLINKING");
    break;
  case 3:
    display.println("PWM FADE");
    break;
  }
  display.display(); // send buffer to the screen
}
