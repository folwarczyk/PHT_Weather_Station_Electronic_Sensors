int dustPin = A0;
int ledPin = D2;


float measured_volts = 0;
float calc_voltage = 0;
float dust_density = 0;


void setup() {
  Serial.begin(57600);
  pinMode(ledPin, OUTPUT);

}

void loop() {
  digitalWrite(ledPin, LOW);
  delayMicroseconds(280);
  measured_volts = analogRead(dustPin);

  delayMicroseconds(40);
  digitalWrite(ledPin,HIGH); // turn the LED off
  delayMicroseconds(9680);

  calc_voltage = measured_voltsd * (3.3/1024.0);
  Serial.println("Sensor readings:");
  Serial.println("Signal value = ");
  Serial.println(measured_volts);
  Serial.println("Voltage = ");
  Serial.println(calc_voltage);
  Serial.println("Dust density = ");
  Serial.println(dust_density);
  Serial.println("");
  delay(1000);
}
