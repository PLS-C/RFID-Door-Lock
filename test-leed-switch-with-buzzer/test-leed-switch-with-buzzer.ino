const int leed_switch_pin = 14;
const int buzzer_pin = 12;

void setup() {
  pinMode(leed_switch_pin, INPUT);
  pinMode(buzzer_pin, OUTPUT);
}

void loop() {
  if(digitalRead(leed_switch_pin)==1){
    digitalWrite(buzzer_pin, HIGH);
  }else{
    digitalWrite(buzzer_pin, LOW);
  }

}
