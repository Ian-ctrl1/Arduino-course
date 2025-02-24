#include <Servo.h>
#include <NewPing.h>

// Motor Driver L298N control pins
const int LeftMotorForward = 7;
const int LeftMotorBackward = 6;
const int RightMotorForward = 5;
const int RightMotorBackward = 4;

// Ultrasonic sensor pins
#define TRIG_PIN A1
#define ECHO_PIN A2
#define MAX_DISTANCE 200
#define GARBAGE_DETECTION_RANGE 20  // Garbage is detected within 20 cm

// IR Sensor pin
const int IR_Sensor = A3;

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);
Servo myservo;

boolean goesForward = false;
int distance = 100;
int servoAngle = 0;

void setup() {
    pinMode(LeftMotorForward, OUTPUT);
    pinMode(LeftMotorBackward, OUTPUT);
    pinMode(RightMotorForward, OUTPUT);
    pinMode(RightMotorBackward, OUTPUT);
    pinMode(IR_Sensor, INPUT);

    Serial.begin(9600);

    myservo.attach(10);
    myservo.write(servoAngle);
    delay(1000);

    distance = readPing();
}

void loop() {
    rotateSensor();
    int irValue = digitalRead(IR_Sensor);
    distance = readPing();

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print(" cm | IR Sensor: ");
    Serial.println(irValue == LOW ? "Obstacle Detected" : "Clear");

    if (distance <= 25 || irValue == LOW) {
        avoidObstacle();
    } else if (distance <= GARBAGE_DETECTION_RANGE) {
        Serial.println("Garbage detected! Circular motion for cleanup...");
        circularSweep();
    } else {
        moveForward();
    }
}

// 🔹 Rotate Ultrasonic Sensor to Continuously Scan
void rotateSensor() {
    static int direction = 1;
    myservo.write(servoAngle);
    delay(100);
    servoAngle += (direction * 10);
    if (servoAngle >= 180 || servoAngle <= 0) {
        direction = -direction;
    }
}

// 🔹 Garbage Cleanup Pattern - Move in a Circle
void circularSweep() {
    moveStop();
    delay(100);
    for (int i = 0; i < 3; i++) {
        turnLeftHard();
        delay(300);
        moveForward();
        delay(500);
    }
}

// 🔹 Avoid Obstacles
void avoidObstacle() {
    moveStop();
    delay(100);
    moveBackward();
    delay(400);
    moveStop();
    delay(100);

    int distanceRight = lookRight();
    int distanceLeft = lookLeft();

    Serial.print("Right: ");
    Serial.print(distanceRight);
    Serial.print(" cm | Left: ");
    Serial.println(distanceLeft);

    if (distanceRight > distanceLeft && distanceRight > 30) {
        Serial.println("Turning RIGHT (Best Path)");
        turnRightHard();
    } else if (distanceLeft > distanceRight && distanceLeft > 30) {
        Serial.println("Turning LEFT (Best Path)");
        turnLeftHard();
    } else {
        Serial.println("NO CLEAR PATH - Turning Around!");
        turnAround();
    }
}

// 🔹 Look Right and Left
int lookRight() {
    myservo.write(45);
    delay(350);
    int distance = readPing();
    myservo.write(115);
    return distance;
}

int lookLeft() {
    myservo.write(175);
    delay(350);
    int distance = readPing();
    myservo.write(115);
    return distance;
}

// 🔹 Fast Ultrasonic Sensor Readings
int readPing() {
    int cm = sonar.ping_cm();
    return (cm == 0) ? MAX_DISTANCE : cm;
}

// 🔹 Motor Controls
void moveStop() {
    digitalWrite(LeftMotorForward, LOW);
    digitalWrite(RightMotorForward, LOW);
    digitalWrite(LeftMotorBackward, LOW);
    digitalWrite(RightMotorBackward, LOW);
    Serial.println("Motors Stopped");
}

void moveForward() {
    if (!goesForward) {
        goesForward = true;
        digitalWrite(LeftMotorForward, HIGH);
        digitalWrite(RightMotorForward, HIGH);
        digitalWrite(LeftMotorBackward, LOW);
        digitalWrite(RightMotorBackward, LOW);
        Serial.println("Moving Forward");
    }
}

void moveBackward() {
    goesForward = false;
    digitalWrite(LeftMotorBackward, HIGH);
    digitalWrite(RightMotorBackward, HIGH);
    digitalWrite(LeftMotorForward, LOW);
    digitalWrite(RightMotorForward, LOW);
    Serial.println("Moving Backward");
}

// 🔹 Stronger Turns to Cover Area
void turnRightHard() {
    moveStop();
    delay(100);
    digitalWrite(LeftMotorForward, HIGH);
    digitalWrite(RightMotorBackward, HIGH);
    digitalWrite(LeftMotorBackward, LOW);
    digitalWrite(RightMotorForward, LOW);
    Serial.println("Turning RIGHT Strongly");
    delay(700);
    moveForward();
}

void turnLeftHard() {
    moveStop();
    delay(100);
    digitalWrite(LeftMotorBackward, HIGH);
    digitalWrite(RightMotorForward, HIGH);
    digitalWrite(LeftMotorForward, LOW);
    digitalWrite(RightMotorBackward, LOW);
    Serial.println("Turning LEFT Strongly");
    delay(700);
    moveForward();
}

// 🔹 New: Turn Around for Maximum Coverage
void turnAround() {
    moveStop();
    delay(100);
    digitalWrite(LeftMotorForward, HIGH);
    digitalWrite(RightMotorBackward, HIGH);
    digitalWrite(LeftMotorBackward, LOW);
    digitalWrite(RightMotorForward, LOW);
    Serial.println("TURNING AROUND!");
    delay(1200);
    moveForward();
}
