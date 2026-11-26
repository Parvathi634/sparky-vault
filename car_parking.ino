#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myServo;

const int IR1 = 2;          // Entry IR sensor (LOW when triggered)
const int IR2 = 3;          // Exit IR sensor (LOW when triggered)
const int SERVO_PIN = 4;    // Gate servo signal pin

const int TOTAL_SLOTS = 4;
int availableSlots = TOTAL_SLOTS;

const int GATE_CLOSED_ANGLE = 100;
const int GATE_OPEN_ANGLE   = 0;

bool entryDetected = false;
bool exitDetected  = false;
unsigned long gateOpenTime = 0;
const unsigned long GATE_HOLD_MS = 2000;
bool gateIsOpen = false;

void updateLcdStatus() {
  lcd.setCursor(0, 0);
  if (availableSlots > 0) {
    lcd.print("WELCOME!        ");
  } else {
    lcd.print("PARKING FULL    ");
  }

  lcd.setCursor(0, 1);
  lcd.print("Slots Left: ");
  lcd.print(availableSlots);
  lcd.print("   ");
}

void openGate() {
  myServo.write(GATE_OPEN_ANGLE);
  gateOpenTime = millis();
  gateIsOpen = true;
}

void setup() {
  Serial.begin(9600);

  pinMode(IR1, INPUT_PULLUP);
  pinMode(IR2, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();

  myServo.attach(SERVO_PIN);
  myServo.write(GATE_CLOSED_ANGLE);

  lcd.setCursor(0, 0);
  lcd.print("  ARDUINO SMART ");
  lcd.setCursor(0, 1);
  lcd.print(" PARKING SYSTEM ");
  delay(2000);
  lcd.clear();
  updateLcdStatus();
}

void loop() {
  bool ir1Active = (digitalRead(IR1) == LOW);
  bool ir2Active = (digitalRead(IR2) == LOW);

  // Vehicle arriving at entry
  if (ir1Active && !entryDetected && !exitDetected) {
    entryDetected = true;
    if (availableSlots > 0) {
      availableSlots--;
      openGate();
    } else {
      lcd.setCursor(0, 0);
      lcd.print("   SORRY :(     ");
      lcd.setCursor(0, 1);
      lcd.print("  Parking Full  ");
      delay(2000);
      entryDetected = false;
    }
    updateLcdStatus();
  }

  // Vehicle passing exit
  if (ir2Active && !exitDetected && !entryDetected) {
    exitDetected = true;
    if (availableSlots < TOTAL_SLOTS) {
      availableSlots++;
      openGate();
    }
    updateLcdStatus();
  }

  // Reset flags once sensor clears after gate opens
  if (entryDetected && !ir1Active && gateIsOpen) {
    entryDetected = false;
  }
  if (exitDetected && !ir2Active && gateIsOpen) {
    exitDetected = false;
  }

  // Close gate automatically after hold duration
  if (gateIsOpen && (millis() - gateOpenTime >= GATE_HOLD_MS)) {
    myServo.write(GATE_CLOSED_ANGLE);
    gateIsOpen = false;
  }
}
