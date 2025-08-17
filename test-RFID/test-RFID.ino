#include <SPI.h>
#include <MFRC522.h>

// RC522 pins
#define SS_PIN 5
#define RST_PIN 15

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  SPI.begin(18, 19, 23); // SCK=18, MISO=19, MOSI=23
  mfrc522.PCD_Init();
  Serial.println("Place your RFID card near the reader...");
}
String str = "";
void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.print("UID: ");
  str = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    str += mfrc522.uid.uidByte[i] < 0x10 ? "0" : "";
    str += String(mfrc522.uid.uidByte[i], HEX);
    
  }
  str.toUpperCase(); 
  Serial.println(str);
}
