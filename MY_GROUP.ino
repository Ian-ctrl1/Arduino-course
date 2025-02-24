#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// Constants
const int soilSensorPins[] = { A0, A1, A2, A3 };
const int relayPins[] = { 8, 9, 10, 11 };
const int numSensors = 4;
const int moistureThreshold = 20;
const long smsInterval = 60000;  // 3 minutes in milliseconds

const int trigPin = 5;
const int echoPin = 6;
const int waterPumpRelayPin = 4;

// GSM Module
SoftwareSerial SIM900(2, 3);  // TX, RX for SIM900
int _timeout;
String _buffer;
String number = "+639756675501";

// LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);  // Adjust to your LCD I2C address

// Variables
long lastSmsTime = 0;
long duration;
float distance;

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);

  // Initialize GSM Module
  delay(7000);  // delay for 7 seconds to make sure the module gets the signal
  Serial.println("System Started...");
  SIM900.begin(9600);  // Initialize SIM900
  delay(1000);

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Initialize pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(waterPumpRelayPin, OUTPUT);
  digitalWrite(waterPumpRelayPin, HIGH);  // Ensure pump is off initially

  for (int i = 0; i < numSensors; i++) {
    pinMode(soilSensorPins[i], INPUT);
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH);  // Ensure relays are off initially
  }

  // Initial Display
  lcd.setCursor(0, 0);
  lcd.print("GSM-Based Automatic ");
  lcd.setCursor(0, 1);
  lcd.print("   Plant Watering   ");
  lcd.setCursor(0, 2);
  lcd.print("       System       ");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Leader:             ");
  lcd.setCursor(0, 1);
  lcd.print("  Ayson, Ian        ");
  lcd.setCursor(0, 2);
  lcd.print("         Christopher");
  delay(3000);
  lcd.setCursor(0, 0);
  lcd.print("Members:");
  lcd.setCursor(0, 1);
  lcd.print("Agustin, Aloizeus    ");
  lcd.setCursor(0, 2);
  lcd.print("Lucas, Katerine     ");
  lcd.setCursor(0, 3);
  lcd.print("Nesperos, Mary Chris");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Initializing.....   ");
  String message = "GSM-Based Plant Watering System is now fully OPERATIONAL";
  sendSms(message);
  lcd.clear();
}

void loop() {
  // Read soil moisture sensors
  int soilMoisture[numSensors];
  for (int i = 0; i < numSensors; i++) {
    soilMoisture[i] = analogRead(soilSensorPins[i]);
    lcd.setCursor(0, i);
    lcd.print("level ");
    lcd.print(i + 1);
    lcd.print(": ");
    lcd.print(soilMoisture[i]);
    Serial.print("misyu ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(soilMoisture[i]);

    // Check soil moisture level
    if (soilMoisture[i] > moistureThreshold) {
      digitalWrite(relayPins[i], LOW);  // Turn on the corresponding relay
    } else {
      digitalWrite(relayPins[i], HIGH);  // Turn off the corresponding relay
    }
  }

  // Read water level
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = (duration * 0.034 / 2);
  Serial.print("Level : ");
  Serial.println(distance);

  // Water level control
  if (distance > 30) {
    digitalWrite(waterPumpRelayPin, LOW);  // Activate water pump
    Serial.println("Phone distance is less than or equal to 15");
  } else if (distance <= 30) {
    digitalWrite(waterPumpRelayPin, HIGH);  // Deactivate water pump
    Serial.println("Phone distance is greater than 30");
  }

  // Send SMS every 3 minutes
  if (millis() - lastSmsTime >= smsInterval) {
    Serial.print("Send every 3 minutes");
    sendMoistureSms(soilMoisture);
    sendWaterLevelSms(distance);
    lastSmsTime = millis();
  }

  delay(1000);
}

void sendMoistureSms(int soilMoisture[]) {
  String message = "Moisture Levels: ";
  for (int i = 0; i < numSensors; i++) {
    message += "S";
    message += (i + 1);
    message += ": ";
    message += soilMoisture[i];
    if (i < numSensors - 1) message += ", ";
  }
  sendSms(message);
}

void sendWaterLevelSms(float distance) {
  String message = "";
  if (distance <= 10) {
    message = "Water level is greater than 30 cm or nearly full";
  } else if (distance > 10 && distance <= 20) {
    message = "Water level is between 20 cm and 30 cm";
  } else if (distance > 20 && distance <= 30) {
    message = "Water level is between 10 cm and 20 cm";
  } else if (distance > 30) {
    message = "Water level is below threshold";
  }
  sendSms(message);
}

void sendSms(String message) {
  Serial.println("Sending Message");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sending Message    ");
  lcd.setCursor(0, 1);
  lcd.print("Please Wait...      ");
  SIM900.println("AT+CMGF=1");  // Sets the GSM Module in Text Mode
  delay(1000);
  SIM900.println("AT+CMGS=\"" + number + "\"\r");  // Mobile phone number to send message
  delay(1000);
  SIM900.println(message);
  delay(100);
  SIM900.println((char)26);  // ASCII code of CTRL+Z
  delay(5000);
  lcd.clear();
}
