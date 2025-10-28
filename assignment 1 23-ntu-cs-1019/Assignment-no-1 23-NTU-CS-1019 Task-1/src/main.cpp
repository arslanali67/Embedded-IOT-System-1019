#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// LED pins
#define LED_YELLOW 18
#define LED_GREEN  17
#define LED_RED    16

// LED PWM channels (for ESP32)
#define CH_YELLOW 0
#define CH_GREEN  1
#define CH_RED    2
#define PWM_FREQ  5000
#define PWM_RES   8   // 8-bit resolution (0-255)

// Button pins
#define BTN_CYCLE 25
#define BTN_RESET 26



// Button debounce
int stableButtonState = HIGH;
int lastReading = HIGH;
int lastStableState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_MS = 30;

int stableResetState = HIGH;
int lastResetReading = HIGH;
int lastStableResetState = HIGH;
unsigned long lastResetDebounceTime = 0;

// LED state: 0=OFF, 1=ALL ON, 2=BLINK, 3=PWM FADE
int ledMode = 0;

// Blink timing
unsigned long lastBlinkTime = 0;
const unsigned long BLINK_INTERVAL = 500;
bool blinkState = false;

// PWM fade
int fadeValue = 0;
int fadeDirection = 1;
unsigned long lastFadeTime = 0;
const unsigned long FADE_INTERVAL = 10;

void setup() {
  pinMode(BTN_CYCLE, INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  // Initialize button states
  lastReading = digitalRead(BTN_CYCLE);
  stableButtonState = lastReading;
  lastStableState = stableButtonState;

  lastResetReading = digitalRead(BTN_RESET);
  stableResetState = lastResetReading;
  lastStableResetState = stableResetState;

  // Attach PWM channels
  ledcSetup(CH_YELLOW, PWM_FREQ, PWM_RES);
  ledcAttachPin(LED_YELLOW, CH_YELLOW);

  ledcSetup(CH_GREEN, PWM_FREQ, PWM_RES);
  ledcAttachPin(LED_GREEN, CH_GREEN);

  ledcSetup(CH_RED, PWM_FREQ, PWM_RES);
  ledcAttachPin(LED_RED, CH_RED);

  // Initial LEDs off
  ledcWrite(CH_YELLOW, 0);
  ledcWrite(CH_GREEN, 0);
  ledcWrite(CH_RED, 0);
}

void loop() {
  unsigned long currentMillis = millis();

  // -------- Handle BTN_CYCLE --------
  int reading = digitalRead(BTN_CYCLE);
  if (reading != lastReading) {
    lastDebounceTime = currentMillis;
    lastReading = reading;
  }
  if (currentMillis - lastDebounceTime >= DEBOUNCE_MS) {
    if (stableButtonState != reading) {
      stableButtonState = reading;
      if (lastStableState == HIGH && stableButtonState == LOW) {
        ledMode++;
        if (ledMode > 3) ledMode = 0;

        fadeValue = 0;
        fadeDirection = 1;
        blinkState = false;
        lastBlinkTime = currentMillis;
        lastFadeTime = currentMillis;
      }
      lastStableState = stableButtonState;
    }
  }

  // -------- Handle BTN_RESET --------
  int resetReading = digitalRead(BTN_RESET);
  if (resetReading != lastResetReading) {
    lastResetDebounceTime = currentMillis;
    lastResetReading = resetReading;
  }
  if (currentMillis - lastResetDebounceTime >= DEBOUNCE_MS) {
    if (stableResetState != resetReading) {
      stableResetState = resetReading;
      if (lastStableResetState == HIGH && stableResetState == LOW) {
        ledMode = 0;

        // Turn off all LEDs
        ledcWrite(CH_YELLOW, 0);
        ledcWrite(CH_GREEN, 0);
        ledcWrite(CH_RED, 0);

        fadeValue = 0;
        fadeDirection = 1;
        blinkState = false;
        lastBlinkTime = currentMillis;
        lastFadeTime = currentMillis;
      }
      lastStableResetState = stableResetState;
    }
  }

  // -------- Handle LED modes --------
  switch (ledMode) {
    case 0: // ALL OFF
      ledcWrite(CH_YELLOW, 0);
      ledcWrite(CH_GREEN, 0);
      ledcWrite(CH_RED, 0);
      break;
    case 1: // ALL ON
      ledcWrite(CH_YELLOW, 255);
      ledcWrite(CH_GREEN, 255);
      ledcWrite(CH_RED, 255);
      break;
    case 2: // BLINK
      if (currentMillis - lastBlinkTime >= BLINK_INTERVAL) {
        blinkState = !blinkState;
        lastBlinkTime = currentMillis;
      }
      ledcWrite(CH_YELLOW, blinkState ? 255 : 0);
      ledcWrite(CH_GREEN, blinkState ? 0 : 255);
      ledcWrite(CH_RED, blinkState ? 255 : 0);
      break;
    case 3: // PWM FADE
      if (currentMillis - lastFadeTime >= FADE_INTERVAL) {
        fadeValue += fadeDirection * 5;
        if (fadeValue >= 255) { fadeValue = 255; fadeDirection = -1; }
        if (fadeValue <= 0)   { fadeValue = 0; fadeDirection = 1; }
        ledcWrite(CH_YELLOW, fadeValue);
        ledcWrite(CH_GREEN, fadeValue);
        ledcWrite(CH_RED, fadeValue);
        lastFadeTime = currentMillis;
      }
      break;
  }

  // -------- Update OLED --------
  display.clearDisplay();
  display.setCursor(0, 20);
  switch (ledMode) {
    case 0: display.println("ALL OFF"); break;
    case 1: display.println("ALL ON"); break;
    case 2: display.println("BLINKING"); break;
    case 3: display.println("PWM FADE"); break;
  }
  display.display();
}
