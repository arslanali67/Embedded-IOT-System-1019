#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Pin Definitions ---
#define BTN_PIN   25
#define LED_PIN   18
#define BUZZER    19

// --- LED PWM Setup (ESP32) ---
#define CH_LED    0
#define PWM_FREQ  5000
#define PWM_RES   8   // 8-bit (0–255)

// --- Debounce and Press Detection ---
bool lastButtonState = HIGH;
bool buttonPressed = false;
unsigned long pressStartTime = 0;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_MS = 30;
const unsigned long LONG_PRESS_MS = 1500;

// --- LED & Buzzer states ---
bool ledState = false;
bool buzzerOn = false;
unsigned long buzzerStartTime = 0;
const unsigned long BUZZER_DURATION = 200;

void setup() {
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  // Setup LED PWM
  ledcSetup(CH_LED, PWM_FREQ, PWM_RES);
  ledcAttachPin(LED_PIN, CH_LED);
  ledcWrite(CH_LED, 0);

  // Setup OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println("Ready");
  display.display();
}

void loop() {
  unsigned long currentMillis = millis();
  bool reading = digitalRead(BTN_PIN);

  // Debounce button
  if (reading != lastButtonState) {
    lastDebounceTime = currentMillis;
    lastButtonState = reading;
  }

  if ((currentMillis - lastDebounceTime) > DEBOUNCE_MS) {
    // Button pressed
    if (!buttonPressed && reading == LOW) {
      buttonPressed = true;
      pressStartTime = currentMillis;
    }

    // Button released
    if (buttonPressed && reading == HIGH) {
      unsigned long pressDuration = currentMillis - pressStartTime;
      buttonPressed = false;

      if (pressDuration >= LONG_PRESS_MS) {
        // --- Long Press ---
        digitalWrite(BUZZER, HIGH);
        buzzerOn = true;
        buzzerStartTime = currentMillis;

        display.clearDisplay();
        display.setCursor(0, 20);
        display.println("Long Press");
        display.display();
      } else {
        // --- Short Press ---
        ledState = !ledState;
        ledcWrite(CH_LED, ledState ? 255 : 0);

        display.clearDisplay();
        display.setCursor(0, 20);
        display.println(ledState ? "LED ON" : "LED OFF");
        display.display();
      }
    }
  }

  // Turn off buzzer after duration
  if (buzzerOn && (currentMillis - buzzerStartTime >= BUZZER_DURATION)) {
    digitalWrite(BUZZER, LOW);
    buzzerOn = false;
  }
}
