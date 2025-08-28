/*
  ESP32 Digital Dice
  - OLED SSD1306 128x64 (I2C)
  - Pushbutton (INPUT_PULLUP) to roll
  - Shows large number + dice face
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "esp_random.h" // for seeding

// OLED config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1   // Reset pin (not used)
#define OLED_ADDRESS  0x3C // common address; change if needed

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pins
const int buttonPin = 15;   // pushbutton connected to GPIO15 and GND
const int ledPin = 2;       // optional onboard LED to show activity

// Button / debounce
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // ms
int lastButtonState = HIGH;
int buttonState = HIGH;

// Dice
int currentDice = 1;

void drawDiceFace(int x, int y, int size, int value) {
  // Draw rectangle border for dice
  display.drawRoundRect(x, y, size, size, 6, SSD1306_WHITE);

  // dot positions relative to dice
  int dotR = size / 10 + 1;
  int cx = x + size/2, cy = y + size/2;
  int leftX = x + size/4;
  int rightX = x + 3*size/4;
  int topY = y + size/4;
  int bottomY = y + 3*size/4;

  auto drawDot = [&](int px, int py) {
    display.fillCircle(px, py, dotR, SSD1306_WHITE);
  };

  // clear interior by not drawing anything (border drawn")
  // draw dots depending on value
  switch (value) {
    case 1:
      drawDot(cx, cy);
      break;
    case 2:
      drawDot(leftX, topY);
      drawDot(rightX, bottomY);
      break;
    case 3:
      drawDot(leftX, topY);
      drawDot(cx, cy);
      drawDot(rightX, bottomY);
      break;
    case 4:
      drawDot(leftX, topY);
      drawDot(rightX, topY);
      drawDot(leftX, bottomY);
      drawDot(rightX, bottomY);
      break;
    case 5:
      drawDot(leftX, topY);
      drawDot(rightX, topY);
      drawDot(cx, cy);
      drawDot(leftX, bottomY);
      drawDot(rightX, bottomY);
      break;
    case 6:
      drawDot(leftX, topY);
      drawDot(rightX, topY);
      drawDot(leftX, cy);
      drawDot(rightX, cy);
      drawDot(leftX, bottomY);
      drawDot(rightX, bottomY);
      break;
  }
}

void showWelcome() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("ESP32 Digital Dice");
  display.println();
  display.println("Press button to roll!");
  display.display();
}

void showResult(int val) {
  display.clearDisplay();

  // Big number on left
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(4);
  display.setCursor(0, 6);
  display.print(val);

  // Dice face on right
  int diceSize = 48;
  int diceX = 72;
  int diceY = 8;
  drawDiceFace(diceX, diceY, diceSize, val);

  // small text bottom
  display.setTextSize(1);
  display.setCursor(0, 56);
  display.print("Roll:");
  display.print(val);

  display.display();
}

int rollDice() {
  // create a better seed using esp_random
  randomSeed(esp_random());
  int r = random(1, 7); // 1..6
  return r;
}

void rollAnimation() {
  // quick animation: random numbers flashing
  for (int i = 0; i < 10; i++) {
    int r = random(1, 7);
    showResult(r);
    delay(80);
  }
}

void setup() {
  pinMode(buttonPin, INPUT_PULLUP); // button to GND
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  Serial.begin(115200);

  // Init display
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // don't proceed
  }
  display.clearDisplay();
  showWelcome();
  delay(1000);
}

void loop() {
  int reading = digitalRead(buttonPin);

  // Debounce logic
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      // buttonState LOW => pressed (since using INPUT_PULLUP)
      if (buttonState == LOW) {
        digitalWrite(ledPin, HIGH); // activity LED on
        rollAnimation();
        int val = rollDice();
        currentDice = val;
        showResult(val);
        digitalWrite(ledPin, LOW);
      }
    }
  }
  lastButtonState = reading;
}
