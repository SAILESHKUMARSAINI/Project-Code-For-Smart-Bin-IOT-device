// ============================================================
//  Smart Recycling Bin
//  Sensors: Ultrasonic (lid, dry, wet), IR, Moisture
//  Actuators: 2x Servo, Buzzer, I2C LCD 16x2
// ============================================================

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// ── Pin Definitions ─────────────────────────────────────────
#define TRIG_LID   2
#define ECHO_LID   3
#define IR_PIN     4
#define MOIST_PIN  5
#define SERVO_LID  6
#define SERVO_FLAP 7
#define TRIG_DRY   8
#define ECHO_DRY   9
#define TRIG_WET   A0
#define ECHO_WET   A1
#define BUZZER     12

// ── Constants ───────────────────────────────────────────────
const float     BIN_HEIGHT     = 30.0;   // cm (sensor to bin bottom)
const float     LID_TRIGGER    = 15.0;   // cm — hand distance to open lid
const unsigned long LID_DURATION = 5000; // ms lid stays open
const int       FULL_THRESHOLD = 90;     // % fill to trigger full-bin alert

// ── Globals ─────────────────────────────────────────────────
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo lidServo, flapServo;

bool          lidOpen      = false;
bool          wasteDetected = false;
bool          binFullAlert  = false;
unsigned long lidOpenTime  = 0;

// ── Setup ────────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);

  // Ultrasonic pins
  pinMode(TRIG_LID,  OUTPUT); pinMode(ECHO_LID,  INPUT);
  pinMode(TRIG_DRY,  OUTPUT); pinMode(ECHO_DRY,  INPUT);
  pinMode(TRIG_WET,  OUTPUT); pinMode(ECHO_WET,  INPUT);

  // Sensor + actuator pins
  pinMode(IR_PIN,    INPUT);
  pinMode(MOIST_PIN, INPUT);
  pinMode(BUZZER,    OUTPUT);

  // Servos
  lidServo.attach(SERVO_LID);
  flapServo.attach(SERVO_FLAP);
  lidServo.write(0);    // lid closed
  flapServo.write(90);  // flap neutral

  // LCD boot screen
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcdPrint(0, "Smart Recycle");
  lcdPrint(1, "Bin  ON  :)");
  delay(2000);
  lcd.clear();

  // Boot beep
  tone(BUZZER, 2000, 500); delay(600);
  tone(BUZZER, 1500, 500); delay(600);

  Serial.println("SYSTEM READY");
}

// ── Main Loop ────────────────────────────────────────────────
void loop() {
  float dryDist = measureDistance(TRIG_DRY, ECHO_DRY);
  float wetDist = measureDistance(TRIG_WET, ECHO_WET);
  int   dryLevel = getFillLevel(dryDist);
  int   wetLevel = getFillLevel(wetDist);

  // LCD row 0: fill levels
  String row0 = "Dry:" + String(dryLevel) + "% Wet:" + String(wetLevel) + "%";
  lcdPrint(0, row0);

  // ── Full-bin alert (locks lid) ───────────────────────────
  if (dryLevel >= FULL_THRESHOLD || wetLevel >= FULL_THRESHOLD) {
    if (!binFullAlert) {
      lcdPrint(1, "BIN FULL! LOCKED");
      Serial.println("BIN FULL — LID LOCKED");
      binFullAlert = true;
    }
    tone(BUZZER, 2500, 200);
    delay(300);
    return; // skip everything else while bin is full
  }

  binFullAlert = false;

  // ── Lid open on hand detection ───────────────────────────
  if (!lidOpen && measureDistance(TRIG_LID, ECHO_LID) < LID_TRIGGER) {
    lidServo.write(90);
    lidOpen       = true;
    lidOpenTime   = millis();
    wasteDetected = false;
    lcdPrint(1, "Lid Open...");
    Serial.println("LID OPEN");
    delay(700);
  }

  // ── Auto-close lid after timeout ────────────────────────
  if (lidOpen && (millis() - lidOpenTime > LID_DURATION)) {
    lidServo.write(0);
    flapServo.write(90); // reset flap to neutral
    lidOpen = false;
    lcdPrint(1, "Lid Closed");
    Serial.println("LID CLOSED");
  }

  // ── Waste sorting (runs 1 s after lid opens) ─────────────
  if (lidOpen && !wasteDetected && (millis() - lidOpenTime > 1000)) {
    bool isIR  = (digitalRead(IR_PIN)    == LOW); // object detected
    bool isWet = (digitalRead(MOIST_PIN) == LOW); // moisture detected

    if (isIR || isWet) {
      wasteDetected = true;

      bool dry = isIR && !isWet;
      flapServo.write(dry ? 30 : 150);           // 30 = dry side, 150 = wet side
      lcdPrint(1, dry ? ">> DRY Waste" : ">> WET Waste");
      Serial.println(dry ? "SORTED: DRY" : "SORTED: WET");

      delay(1800);         // hold flap so waste falls through
      flapServo.write(90); // return flap to neutral
    }
  }

  delay(200);
}

// ── Helper: measure distance via HC-SR04 ─────────────────────
float measureDistance(uint8_t trigPin, uint8_t echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // 30 ms timeout
  if (duration == 0) return 999.0;               // no echo → treat as empty/error
  return duration * 0.034 / 2.0;
}

// ── Helper: convert distance to fill % ───────────────────────
int getFillLevel(float dist) {
  const float EMPTY_DIST = BIN_HEIGHT - 2.0; // ~28 cm = 0 %
  const float FULL_DIST  = 5.0;              //   5 cm = 100 %

  if (dist >= EMPTY_DIST) return 0;
  if (dist <= FULL_DIST)  return 100;
  return (int)((EMPTY_DIST - dist) * 100.0 / (EMPTY_DIST - FULL_DIST));
}

// ── Helper: print to LCD with padding ────────────────────────
void lcdPrint(int row, String msg) {
  lcd.setCursor(0, row);
  // Pad to 16 chars to clear leftover characters
  while (msg.length() < 16) msg += ' ';
  lcd.print(msg.substring(0, 16));
}
