#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>

// RC522 pins
#define SDA_PIN 5 
#define RST_PIN 4
#define RFID_SCK 18
#define MISO 19
#define MOSI 23
MFRC522 mfrc522(SDA_PIN, RST_PIN);

//LCD pins
#define LED_PIN 13
#define LCD_SDA 21
#define LCD_SCK 22
// Change 0x27 to your LCD address if needed
LiquidCrystal_I2C lcd(0x27, 16, 2);

//Relay pin
#define Relay1_PIN 26
#define Relay2_PIN 27

//buzzer pin
#define buzzer_pin 12

//reed_switch_PIN
#define reed_switch_PIN 14

struct Result {
  int access;
  String text;
  String user_name;
};
String str = "";
String text = "";

const char* ssid = "Your-WiF"; //WiFi without password 
const char* serverName = "https://your-server.com/check_RFID-key.php";
String postData; 

StaticJsonDocument<1024> doc; //1 Key=50 1024=20 key

unsigned long previousMillis = 0;
const long interval = 1000; //milliseconds
bool buzzerState = false;

unsigned long lastReadTime = 0;   // Time we last saw a card
const unsigned long resetInterval = 60000; // 60 sec no card → reset

void setup() {

  //Initializing the serial
  Serial.begin(115200);
  //Initializing the LCD
  Wire.begin(LCD_SDA, LCD_SCK); // SDA = GPIO 21, SCL = GPIO 22
  lcd.begin();
  //lcd.init();
  lcd.backlight();
  pinMode(LED_PIN, OUTPUT);

  //pinMode(reed_switch_PIN, INPUT);
  pinMode(reed_switch_PIN, INPUT_PULLUP);  // Use internal pull-up resistor
  pinMode(buzzer_pin, OUTPUT);
  pinMode(Relay1_PIN, OUTPUT);
  pinMode(Relay2_PIN, OUTPUT);

  digitalWrite(LED_PIN, HIGH);
  digitalWrite(Relay1_PIN, HIGH);
  digitalWrite(Relay2_PIN, HIGH);
  digitalWrite(buzzer_pin, LOW);

  //Initializing the RFID
  
  SPI.begin(RFID_SCK, MISO, MOSI); // SCK=18, MISO=19, MOSI=23
  //mfrc522.PCD_Init();
  resetRC522();
  if (!testRC522()) {
    Serial.println(F("[ERROR] Cannot communicate with RC522. Check wiring!"));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Cannot connected");
    lcd.setCursor(0, 1);
    lcd.print("RFID-RC522");
    while (true) delay(100); // Stop here
  }

  // Start Wi-Fi (even without connecting)
  WiFi.mode(WIFI_STA);  // Station mode
  delay(100);           // Small delay for init
  // Print MAC address
  Serial.print("ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());
  // Connect to  Wi-Fi
  Serial.print("Connecting WiFi");
  Serial.println(ssid);
  WiFi.begin(ssid); // No password
  
  int dotCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.setCursor(0, 0);
    lcd.print("Connecting WiFi");
    // If more than 16 dots, clear
    if (dotCount >= 16) {
      lcd.setCursor(0, 1);
      lcd.print("                "); // Clear line
      dotCount = 0;
    }
    lcd.setCursor(dotCount, 1);
    lcd.print(".");
    dotCount++;
  }

  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wi-Fi status");
  lcd.setCursor(0, 1);
  lcd.print("Connected!");
  delay(3000);
  lcd.clear();

  JsonArray Name = doc.createNestedArray("Name");
    Name.add("Guest1");
    Name.add("Guest2");
  JsonArray Key = doc.createNestedArray("Key");
    Key.add("10E9801C"); //Guest1
    Key.add("73153C31"); //Guest2
  serializeJson(doc, Serial);
  Serial.println();
  serializeJsonPretty(doc, Serial);

  digitalWrite(LED_PIN, LOW);
}

void loop() {
  //Door alarm system
  int doorState = digitalRead(reed_switch_PIN);
  if (doorState == HIGH) {  // Door opened
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      buzzerState = !buzzerState;               // Toggle buzzer state
      digitalWrite(buzzer_pin, buzzerState ? HIGH : LOW);
      unsigned long delta_t = currentMillis - previousMillis;
    }
    //Serial.println("Door opened! Alarm beeping");
  } else {
    digitalWrite(buzzer_pin, LOW);               // Turn buzzer OFF
    //Serial.println("Door closed. Alarm OFF");
    buzzerState = false;
    previousMillis = millis();
  }

  // Auto reset if no card detected for resetInterval
  if (millis() - lastReadTime > resetInterval) {
    hardResetRC522();
    lastReadTime = millis(); // Reset timer
  }
  //Access system
  if (!mfrc522.PICC_IsNewCardPresent()){
    delay(100);
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial()){
    delay(100);
    return;
  }
  // Got a card — update last read time
  lastReadTime = millis();
  
  Serial.print("UID: ");
  str = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    str += mfrc522.uid.uidByte[i] < 0x10 ? "0" : "";
    str += String(mfrc522.uid.uidByte[i], HEX);
    
  }
  str.toUpperCase(); 
  Serial.println(str);
  digitalWrite(LED_PIN, HIGH);
  
  Result output = check_id(str);
  if (output.access == 1){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(output.user_name);
    lcd.setCursor(0, 1);
    text = "OPEN ";
    text += output.text;
    lcd.print(text);
    digitalWrite(Relay1_PIN, LOW);
  }else{
    lcd.clear();
    lcd.setCursor(0, 0);
    text = "UID=";
    text += str;
    lcd.print(text);
    lcd.setCursor(0, 1);
    text = "LOCKED ";
    text += output.text;
    lcd.print(text);
  }
  delay(5000);
  digitalWrite(Relay1_PIN, HIGH);
  lcd.clear();
  digitalWrite(LED_PIN, LOW);

  // Halt card and stop crypto
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

Result check_id(String input){
  Result r;
  //Offline
  r.access = 0;
  r.text = "(Offline)";
  for(int i=0;i<doc["Name"].size();i++){
    String Name_0=doc["Name"][i];
    String Key_0=doc["Key"][i];
    Serial.println(Name_0);
    Serial.println(Key_0);
    Serial.println(input);
    if (input == Key_0){
      r.access = 1;
      r.text = "(Offline)";
      r.user_name = Name_0;
      return(r); 
    }
  }
  //Online
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName); // Specify the URL
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    // Create POST data
    postData = "Room=room_name&room_key=aqwsderfgtyh&RFID=" + input; // post data to server
    // Send HTTP POST request
    int httpResponseCode = http.POST(postData);
    String payload = "";
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      payload = http.getString();
      Serial.println("Response: " + payload);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end(); // Free resources
    if (payload.indexOf("result=allow") >= 0) {
      Serial.println("Access allowed!");
      r.access = 1;
      r.text = "(Online)";
      r.user_name = getValueForKey(payload);
    } else {
      Serial.println("Access denied!");
      r.access = 0;
      r.text = "(Online)";
      r.user_name = "";
    }
  }else{
    Serial.println("WiFi Disconnected");
  }
  return r;
}

String getValueForKey(String data){
  String searchKey = "[user_name] => ";
  int startIndex = data.indexOf(searchKey);
  if (startIndex == -1) return ""; // key not found
  startIndex += searchKey.length();
  int endIndex = data.indexOf('\n', startIndex);
  if (endIndex == -1) endIndex = data.length();
  String user_name = data.substring(startIndex, endIndex);
  user_name.trim(); // remove spaces or newlines
  return user_name;
  
}

// ===== RESET FUNCTION =====
void resetRC522() {
  Serial.println(F("[INFO] Hard reset RC522..."));
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, LOW);
  delay(50);
  digitalWrite(RST_PIN, HIGH);
  delay(50);

  mfrc522.PCD_Init();
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  mfrc522.PCD_AntennaOn();
}

// ===== TEST FUNCTION =====
bool testRC522() {
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("[TEST] RC522 VersionReg: 0x"));
  Serial.println(v, HEX);

  if (v == 0x91 || v == 0x92) {
    Serial.println(F("[PASS] RC522 is responding."));
    return true;
  } else {
    Serial.println(F("[FAIL] RC522 not responding."));
    return false;
  }
}

void hardResetRC522() {
  Serial.println("[INFO] Hard reset RC522...");
  digitalWrite(RST_PIN, LOW);
  delay(50);
  digitalWrite(RST_PIN, HIGH);
  delay(50);
  mfrc522.PCD_Init();
  Serial.print("[TEST] RC522 VersionReg: 0x");
  Serial.println(mfrc522.PCD_ReadRegister(mfrc522.VersionReg), HEX);
}