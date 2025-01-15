const int analogPin = A1; // Analog pin til spændingsmåling
float Vmax = 5.0;         // Arduinos analoge reference (typisk 5V)
const int relayPin = 2;   // vi sætter relæ inputter til Pin 2

void setup() {
  Serial.begin(9600);
  pinMode(analogPin, INPUT);
  pinMode(relayPin, OUTPUT);
}

void loop() {
  int sensorValue = analogRead(analogPin);
  float voltage = sensorValue * (Vmax / 1023.0);
  delay(1000);

  if (voltage > 4.0) {
    digitalWrite(relayPin, LOW);
    NotprintLoop(5);
  } else {
    digitalWrite(relayPin, HIGH);
    NotprintLoop(0);
  }
}
void NotprintLoop (int gridValue) {
  int count = 0;
  while (count < 5) {
    int sensorValue =
    analogRead(analogPin);
    float voltage = sensorValue* (Vmax / 1023.0);
    float gridUsage = gridValue + voltage;
    Serial.print(gridValue);
    Serial.print(" ");
    Serial.print(voltage);
    Serial.print(" ");
    Serial.print(gridUsage);
    Serial.println("");
    count += 1;
    delay(500);
  }
}
