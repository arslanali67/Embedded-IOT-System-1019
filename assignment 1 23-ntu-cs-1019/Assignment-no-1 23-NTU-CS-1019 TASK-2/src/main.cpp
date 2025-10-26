
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// pins
#define LED_PIN 16
#define BUZZER_PIN 25
#define BUTTON_PIN 14

// press timing
unsigned long pressStart = 0;
bool buttonState = HIGH;
bool ledState = LOW;

void showMsg(const char *msg)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println(msg);
  display.display();
}

void beep(unsigned int freq = 1000, unsigned long duration = 400)
{
  unsigned long endTime = millis() + duration;
  unsigned long halfPeriod = 1000000L / (freq * 2);
  while (millis() < endTime)
  {
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(halfPeriod);
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(halfPeriod);
  }
}

void setup()
{
  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  showMsg("Ready... press button");
}

void loop()
{
  int readState = digitalRead(BUTTON_PIN);

  // button pressed
  if (readState == LOW && buttonState == HIGH)
  {
    pressStart = millis();
  }

  // button released
  if (readState == HIGH && buttonState == LOW)
  {
    unsigned long pressDuration = millis() - pressStart;

    if (pressDuration < 1500)
    {
      // short press -> toggle led
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      if (ledState)
        showMsg("Short press: LED ON");
      else
        showMsg("Short press: LED OFF");
    }
    else
    {
      // long press -> buzzer tone
      showMsg("Long press detected");
      beep();
    }
  }

  buttonState = readState;
  delay(10);
}
