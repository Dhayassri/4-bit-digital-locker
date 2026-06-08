#include <Servo.h>

const int passwordPins[] = {2, 3, 4, 5};
const int enterPin = 6;

const int greenLED = 7;
const int redLED = 8;
const int yellowLED = 9;
const int servoPin = 10;
const int buzzerPin = 11;

#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523

const int secretPassword[] = {1, 0, 1, 0};
const int lockedAngle = 0;
const int unlockedAngle = 90;

Servo lockServo;
int wrongAttempts = 0;

int successMelody[] = {NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5};
int successDurations[] = {125, 125, 125, 250};

int failureMelody[] = {NOTE_G4, NOTE_C4};
int failureDurations[] = {200, 400};

// Hardware self-test function
void hardwareTest() {
  Serial.println("Starting hardware self-test...");
  
  // LEDs test
  digitalWrite(greenLED, HIGH);
  delay(500);
  digitalWrite(greenLED, LOW);

  digitalWrite(redLED, HIGH);
  delay(500);
  digitalWrite(redLED, LOW);

  digitalWrite(yellowLED, HIGH);
  delay(500);
  digitalWrite(yellowLED, LOW);

  // Buzzer test
  tone(buzzerPin, NOTE_C4, 300);
  delay(350);
  noTone(buzzerPin);

  // Servo test
  lockServo.write(unlockedAngle);
  delay(1000);
  lockServo.write(lockedAngle);
  delay(500);

  Serial.println("Hardware self-test completed.");
}

// Check if all input pins are connected properly (not floating)
bool checkInputConnections() {
  for (int i = 0; i < 4; i++) {
    int val = digitalRead(passwordPins[i]);
    if (val != HIGH && val != LOW) { // Though digitalRead outputs only HIGH/LOW, floating pins can cause instability
      Serial.print("Error: Password pin ");
      Serial.print(passwordPins[i]);
      Serial.println(" not connected properly.");
      return false;
    }
  }

  int enterVal = digitalRead(enterPin);
  if (enterVal != HIGH && enterVal != LOW) {
    Serial.println("Error: Enter pin not connected properly.");
    return false;
  }
  return true;
}

void playSuccessTune() {
  for (int i = 0; i < 4; i++) {
    tone(buzzerPin, successMelody[i], successDurations[i]);
    delay(successDurations[i] * 1.3);
  }
}

void playFailureTune() {
  for (int i = 0; i < 2; i++) {
    tone(buzzerPin, failureMelody[i], failureDurations[i]);
    delay(failureDurations[i] * 1.3);
  }
}

void indicateHardwareError() {
  // Flash red and yellow LEDs alternately and beep buzzer to indicate error
  while(true) {
    digitalWrite(redLED, HIGH);
    digitalWrite(yellowLED, LOW);
    tone(buzzerPin, NOTE_C4);
    delay(300);
    noTone(buzzerPin);
    delay(300);

    digitalWrite(redLED, LOW);
    digitalWrite(yellowLED, HIGH);
    tone(buzzerPin, NOTE_E4);
    delay(300);
    noTone(buzzerPin);
    delay(300);
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);

  for (int i = 0; i < 4; i++) {
    pinMode(passwordPins[i], INPUT);
  }
  pinMode(enterPin, INPUT);

  lockServo.attach(servoPin);
  lockServo.write(lockedAngle);

  hardwareTest();

  if (!checkInputConnections()) {
    Serial.println("Hardware connection error detected!");
    indicateHardwareError();
  }

  Serial.println("System Initialized. Locker is locked.");
}

void loop() {
  if (wrongAttempts >= 3) {
    digitalWrite(yellowLED, HIGH);
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, LOW);
    while (true);
  }

  static bool lastEnterState = LOW;
  bool currentEnterState = digitalRead(enterPin);

  // Detect rising edge for enter button press with simple debounce
  if (currentEnterState == HIGH && lastEnterState == LOW) {
    delay(50); // simple debounce

    int enteredPassword[4];
    Serial.print("Input: ");
    for (int i = 0; i < 4; i++) {
      enteredPassword[i] = (digitalRead(passwordPins[i]) == HIGH) ? 1 : 0;
      Serial.print(enteredPassword[i]);
    }
    Serial.println();

    bool isCorrect = true;
    for (int i = 0; i < 4; i++) {
      if (enteredPassword[i] != secretPassword[i]) {
        isCorrect = false;
        break;
      }
    }

    if (isCorrect) {
      Serial.println("Access Granted!");
      wrongAttempts = 0;

      playSuccessTune();

      digitalWrite(greenLED, HIGH);
      lockServo.write(unlockedAngle);
      delay(3000);

      digitalWrite(greenLED, LOW);
      lockServo.write(lockedAngle);
      Serial.println("Locker has been re-locked.");

    } else {
      wrongAttempts++;
      Serial.print("Access Denied! Attempt ");
      Serial.print(wrongAttempts);
      Serial.println(" of 3.");

      playFailureTune();

      digitalWrite(redLED, HIGH);
      delay(250);
      digitalWrite(redLED, LOW);
    }
  }

  lastEnterState = currentEnterState;
}
