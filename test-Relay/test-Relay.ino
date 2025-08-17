const int pin26 = 26;
const int pin27 = 27;

void setup() {
  Serial.begin(115200);
  pinMode(pin26, OUTPUT);
  pinMode(pin27, OUTPUT);

  // Initialize pins LOW
  digitalWrite(pin26, LOW);
  digitalWrite(pin27, LOW);

  Serial.println("Send '26' to toggle pin 26");
  Serial.println("Send '27' to toggle pin 27");
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim(); // Remove any whitespace/newline

    if (input == "26") {
      int currentState = digitalRead(pin26);
      digitalWrite(pin26, !currentState);
      Serial.print("Pin 26 toggled to ");
      Serial.println(!currentState ? "HIGH" : "LOW");
    } 
    else if (input == "27") {
      int currentState = digitalRead(pin27);
      digitalWrite(pin27, !currentState);
      Serial.print("Pin 27 toggled to ");
      Serial.println(!currentState ? "HIGH" : "LOW");
    } 
    else {
      Serial.println("Invalid input. Send '26' or '27' to toggle pins.");
    }
  }
}
