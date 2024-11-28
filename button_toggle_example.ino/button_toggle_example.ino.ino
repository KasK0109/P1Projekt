const int knap = 2;

bool state;
unsigned long changeMillis;


void setup() {
  // put your setup code here, to run once:
  pinMode(knap, INPUT);
  Serial.begin(9600);
  state = false;
  changeMillis = millis();
}


void loop() {
  // put your main code here, to run repeatedly:
  int newMillis = millis();
  bool buttonState = digitalRead(knap);
  if (buttonState && newMillis - changeMillis > 100) {
    state = !state;
    changeMillis = newMillis;
  }


  // react to state
}