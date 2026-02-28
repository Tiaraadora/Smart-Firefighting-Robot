#include <ESP32Servo.h>

#define FLAME_LEFT 34
#define FLAME_CENTER 35
#define FLAME_RIGHT 32

#define MOTOR_L1 25
#define MOTOR_L2 26
#define MOTOR_R1 27
#define MOTOR_R2 14

#define RELAY_PIN 21
#define SERVO_PIN 13

#define FIRE_DETECTED_THRESHOLD 2000
#define MOVE_DURATION 500

Servo myServo;

// status
bool fireDetected = false;
bool motorRunning = false;

unsigned long moveStartTime = 0;

// ================= SERVO CONTROL =================
unsigned long previousMillis = 0;
const int interval = 15;   // kecepatan servo

int servoPos = 5;
bool forward = true;

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // relay mati

  pinMode(MOTOR_L1, OUTPUT);
  pinMode(MOTOR_L2, OUTPUT);
  pinMode(MOTOR_R1, OUTPUT);
  pinMode(MOTOR_R2, OUTPUT);

  stopMotors();

  myServo.setPeriodHertz(50);
  myServo.attach(SERVO_PIN, 500, 2400);
  myServo.write(90);

  pinMode(FLAME_LEFT, INPUT);
  pinMode(FLAME_CENTER, INPUT);
  pinMode(FLAME_RIGHT, INPUT);
}

void loop() {

  int left = analogRead(FLAME_LEFT);
  int center = analogRead(FLAME_CENTER);
  int right = analogRead(FLAME_RIGHT);

  int minValue = min(left, min(center, right));

  // ===== DETEKSI API =====
  if (minValue < FIRE_DETECTED_THRESHOLD) {
    fireDetected = true;

    // kalau baru mulai deteksi api
    if (!motorRunning) {
      moveStartTime = millis();
      motorRunning = true;

      if (minValue == left) turnLeft();
      else if (minValue == center) moveForward();
      else turnRight();
    }

  } else {
    fireDetected = false;
    motorRunning = false;
    stopMotors();
  }

  // ===== BATASI MOTOR 3 DETIK =====
  if (motorRunning && millis() - moveStartTime >= MOVE_DURATION) {
    stopMotors();
    motorRunning = false;
  }

  // ===== POMPA AKTIF SELAMA API ADA =====
  if (fireDetected) {
    pumpservo(true);
  } else {
    pumpservo(false);
  }
}

// MOTOR CONTROL

void moveForward() {
  digitalWrite(MOTOR_L1, LOW);
  digitalWrite(MOTOR_L2, HIGH);
  digitalWrite(MOTOR_R1, LOW);
  digitalWrite(MOTOR_R2, HIGH);
}

void turnLeft() {
  digitalWrite(MOTOR_L1, HIGH);
  digitalWrite(MOTOR_L2, LOW);
  digitalWrite(MOTOR_R1, LOW);
  digitalWrite(MOTOR_R2, HIGH);
}

void turnRight() {
  digitalWrite(MOTOR_L1, LOW);
  digitalWrite(MOTOR_L2, HIGH);
  digitalWrite(MOTOR_R1, HIGH);
  digitalWrite(MOTOR_R2, LOW);
}

void stopMotors() {
  digitalWrite(MOTOR_L1, LOW);
  digitalWrite(MOTOR_L2, LOW);
  digitalWrite(MOTOR_R1, LOW);
  digitalWrite(MOTOR_R2, LOW);
}

// PUMP + SERVO

void pumpservo(bool active) {

  if (active) {

    digitalWrite(RELAY_PIN, LOW);  // pompa ON

    if (millis() - previousMillis >= interval) {
      previousMillis = millis();

      myServo.write(servoPos);

      if (forward) {
        servoPos++;
        if (servoPos >= 175) {
          servoPos = 175;
          forward = false;
        }
      } else {
        servoPos--;
        if (servoPos <= 5) {
          servoPos = 5;
          forward = true;
        }
      }
    }

  } else {

    digitalWrite(RELAY_PIN, HIGH);  // pompa OFF
    myServo.write(90);
  }
}