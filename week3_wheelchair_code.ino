
#include <LiquidCrystal.h>
#include <Servo.h>
#include <Stepper.h>

LiquidCrystal lcd(13, 12, 5, 4, 3, 2);

Servo driveServo;

Stepper seatMotor(2048, 11, 9, 10, 8);

const int modeButton = A2;
const int resetButton = A3;
const int upButton = A4;
const int downButton = A5;
const int joyStick=6;

int displayMode = 0;  
int seatAngle = 0;
float calculatedDistance = 0;


int oldModeButton = HIGH;
int oldResetButton = HIGH;

unsigned long previousTime = 0;
unsigned long previousLCD = 0;

void setup() {
  pinMode(modeButton, INPUT_PULLUP);
  pinMode(resetButton, INPUT_PULLUP);
  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);
  pinMode(joyStick,INPUT_PULLUP);

  lcd.begin(16, 2);
  lcd.print("Ready for Use");

  driveServo.attach(7);
  driveServo.writeMicroseconds(1500); 

  seatMotor.setSpeed(10);

  previousTime = millis();
  delay(500);
  lcd.clear();
  Serial.begin(9600);
}

void loop() {

  int joyX = analogRead(A0);
  int joyY = analogRead(A1);


  int servoPulse = 1500;

  if (joyY < 450) {
    servoPulse = map(joyY, 0, 450, 2000, 1500);
  }

  if (joyY > 574) {
    servoPulse = map(joyY, 574, 1023, 1500, 1000);
  }

  driveServo.writeMicroseconds(servoPulse);



float wheelDiameter = 0.60; 
float maxWheelRPM = 10.0;   

int speedPercent = abs(servoPulse - 1500) / 5;

float wheelRPM = maxWheelRPM * speedPercent / 100.0;


float calculatedSpeed = (3.14159 * wheelDiameter * wheelRPM) / 60.0;

unsigned long currentTime = millis();

calculatedDistance = calculatedDistance +
    calculatedSpeed * (currentTime - previousTime) / 1000.0;

previousTime = currentTime;


  int modeState = digitalRead(modeButton);

  if (modeState == LOW && oldModeButton == HIGH) {
    displayMode++;

    if (displayMode > 2) {
      displayMode = 0;
    }

    delay(50);
  }

  oldModeButton = modeState;


  int resetState = digitalRead(resetButton);

  if (displayMode == 1 &&
      resetState == LOW &&
      oldResetButton == HIGH) {

    calculatedDistance = 0;
    delay(50);
  }

  oldResetButton = resetState;


  if (displayMode == 2) {

    if (digitalRead(upButton) == LOW && seatAngle < 45) {
      seatMotor.step(28);
      seatAngle = seatAngle + 5;
      delay(100);
      Serial.println("INCREASE");
    }

    else if (digitalRead(downButton) == LOW && seatAngle > 0) {
      seatMotor.step(-28);
      seatAngle = seatAngle - 5;
      delay(100);
      Serial.println("DECREASE");
    }
  }


  if (millis() - previousLCD >= 250) {
    previousLCD = millis();

    switch (displayMode) {

      case 0:  // SPEED
        lcd.setCursor(0, 0);
        lcd.print("SPEED:        ");
        lcd.setCursor(9, 0);
        lcd.print(calculatedSpeed, 2);
        lcd.print("m/s");

        lcd.setCursor(0, 1);
        lcd.print("X cmd:          ");
        lcd.setCursor(7, 1);

        if (joyX < 400) {
          lcd.print("LEFT");
        }
        else if (joyX > 624) {
          lcd.print("RIGHT");
        }
        else {
          lcd.print("CENTER");
        }
        break;


      case 1:  // DISTANCE
        lcd.setCursor(0, 0);
        lcd.print("DISTANCE (m)        ");

        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.print(calculatedDistance, 2);
        lcd.print(" m");
        break;


      case 2:  // ANGLE
        lcd.setCursor(0, 0);
        lcd.print("Seat angle (cmd)");

        lcd.setCursor(0, 1);
        lcd.print(seatAngle);
        lcd.print(" deg B3+ B4-   ");
        break;
    }
  }
}
