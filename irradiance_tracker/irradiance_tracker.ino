// Most ESP32 DevKits have the on-board LED on GPIO 2
const int ledPin = 2; 

void setup() {
  // Initialize the digital pin as an output
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  Serial.println("ESP32 Blink Challenge: Started!");
}

void loop() {
  // Turn the LED on (HIGH is the voltage level)
  digitalWrite(ledPin, HIGH);
  Serial.println("LED is ON");
  delay(1000); // Wait for a second

  // Turn the LED off by making the voltage LOW
  digitalWrite(ledPin, LOW);
  Serial.println("LED is OFF");
  delay(1000); // Wait for a second
}