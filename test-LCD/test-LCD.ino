#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Change 0x27 to your LCD address if needed
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define LED_PIN 13

void setup() {
  //Initializing the LCD 
  Wire.begin(21, 22); // SDA = GPIO 21, SCL = GPIO 22
  lcd.begin();
  pinMode(LED_PIN, OUTPUT);
  //test
  digitalWrite(LED_PIN, HIGH);
  lcd.setCursor(0, 0);
  lcd.print("1234567890123456");
  lcd.setCursor(0, 1);
  lcd.print("1234567890123456");
  delay(5000);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Hello, ESP32!");
  lcd.setCursor(2, 1);
  lcd.print("LCD Test OK");
  delay(5000);
  lcd.clear();
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  lcd.backlight(); //// Turn screen light ON
  lcd.setCursor(0, 0);
  lcd.print("Backlight: on");
  lcd.setCursor(0, 1);
  lcd.print("LED: on");
  delay(5000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.noBacklight();  // Turn screen light OFF
  lcd.print("Backlight: off");
  delay(5000);
  lcd.clear();

  digitalWrite(LED_PIN, LOW);
  lcd.backlight(); // Turn screen light ON
  lcd.setCursor(0, 0);
  lcd.print("Backlight: on");
  lcd.setCursor(0, 1);
  lcd.print("LED: off");
  delay(5000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.noBacklight();  // Turn screen light OFF
  lcd.print("Backlight: off");
  delay(5000);
  lcd.clear();
}
