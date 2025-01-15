// Define pins
const int VRxPin = A0;  // X-axis
const int VRyPin = A1;  // Y-axis
const int SWPin  = 2;   // Button
const int relayPin = 11;  // Pin connected to the relay
const int relayPin6 = 6;  // Pin connected to the relay

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);  // Set pin 1 as an output
  pinMode(relayPin6, OUTPUT);
  // Set button pin as input with internal pull-up resistor
  pinMode(SWPin, INPUT_PULLUP);
}

void loop() {
  // Read the joystick's X and Y values
  int xValue = analogRead(VRxPin); // Reads analog value from X-axis
  int yValue = analogRead(VRyPin); // Reads analog value from Y-axis
  
  // Read the button state
  int buttonState = digitalRead(SWPin);
  
  // Print values to the serial monitor
  Serial.print("X: ");
  Serial.print(xValue);
  Serial.print(" | Y: ");
  Serial.print(yValue);
  
  if (xValue <= 100) {
  Serial.print(" | Går til batteriet");
  digitalWrite(relayPin, HIGH);  // Turn the LED on
  delay(1000);                 // Wait for a second
  digitalWrite(relayPin, LOW);   // Turn the LED off
  delay(1000); 

  }
  else if (yValue <= 100) {
    Serial.print(" | Går til Byen");
    digitalWrite(relayPin6, HIGH);  // Turn the LED on
    delay(1000);                 // Wait for a second
    digitalWrite(relayPin6, LOW);   // Turn the LED off
    delay(1000); 

  }
  else{
    Serial.print(" | Gør ingen ting");
  }
  
  Serial.println();
  
  // Add a small delay
  delay(1000);
}
