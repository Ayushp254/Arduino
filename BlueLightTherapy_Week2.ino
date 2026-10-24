
#include <SevSegShift.h>
#include <math.h>


#define SHIFT_PIN_SHCP 6
#define SHIFT_PIN_STCP 5
#define SHIFT_PIN_DS 4

SevSegShift sevseg(SHIFT_PIN_DS, SHIFT_PIN_SHCP, SHIFT_PIN_STCP, 1, true);


const byte blue_led = 3;
const byte potentiometer = A0;
const byte light_sensor = A1;
const byte temp_sensor = A3;    

const byte set_button = A4;    
const byte start_button = A2;   
const byte display_button = A5; 

const byte buzzer = 2;
const byte red_led = 7;
const byte green_led = 8;
const byte rgb_blue_led = 13;

const bool RGB_COMMON_ANODE = false;

byte treatmentMinutes = 1;
bool timerRunning = false;
bool timerFinished = false;
unsigned long timerStart = 0;
unsigned long remainingSeconds = 60;

const byte N = 10;
int lightReadings[N] = {0};
int tempReadings[N] = {0};

byte sampleIndex = 0;
byte samplesStored = 0;

int filteredLight = 0;
float temperatureC = 0;
bool temperatureOK = false;
int brightness = 0;
byte warningLevel = 0;

const float FIXED_R = 10000.0;
const float NOMINAL_R = 10000.0;
const float BETA = 3950.0;

const bool LIGHT_CALIBRATED = false;
const float LIGHT_MW_PER_COUNT = 0.0;
const float LIGHT_MW_OFFSET = 0.0;

const int MEDIUM_LIGHT_LIMIT = 50;
const int HIGH_LIGHT_LIMIT = 80;

const bool LIGHT_INCREASES_WITH_BRIGHTNESS = true;

byte displayPage = 0;

void readButtons();
void sampleSensors();
void checkWarnings();
void updateTimer();
void updateBlueLED();
void updateBuzzer();
void updateDisplay();
void printReadings();

void setup() {
  Serial.begin(115200);

  pinMode(blue_led, OUTPUT);
  pinMode(red_led, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(rgb_blue_led, OUTPUT);
  pinMode(buzzer, OUTPUT);

  // All three buttons use internal pull-up resistors
  pinMode(set_button, INPUT_PULLUP);
  pinMode(start_button, INPUT_PULLUP);
  pinMode(display_button, INPUT_PULLUP);

  // ---------- DISPLAY SETUP ----------
  byte numDigits = 4;
  byte digitPins[] = {12, 11, 10, 9};
  byte segmentPins[] = {0, 1, 2, 3, 4, 5, 6, 7};

  bool resistorsOnSegments = false;
  byte hardwareConfig = COMMON_CATHODE;
  bool updateWithDelays = false;
  bool leadingZeros = false;
  bool disableDecPoint = false;

  sevseg.begin(
    hardwareConfig,
    numDigits,
    digitPins,
    segmentPins,
    resistorsOnSegments,
    updateWithDelays,
    leadingZeros,
    disableDecPoint
  );

  sevseg.setBrightness(50);
  sevseg.setChars("0000");

  sampleSensors();
  checkWarnings();
}

void loop() {
  sevseg.refreshDisplay();

  readButtons();
  sampleSensors();
  checkWarnings();
  updateTimer();
  updateBlueLED();
  updateBuzzer();
  updateDisplay();
  printReadings();

  sevseg.refreshDisplay();
}

// ---------- BUTTONS ----------

void readButtons() {
  static byte lastSetState = HIGH;
  static byte lastStartState = HIGH;
  static byte lastDisplayState = HIGH;

  static unsigned long lastSetPress = 0;
  static unsigned long lastStartPress = 0;
  static unsigned long lastDisplayPress = 0;

  byte setState = digitalRead(set_button);
  byte startState = digitalRead(start_button);
  byte displayState = digitalRead(display_button);

  unsigned long now = millis();

  // BUTTON 1: Increase treatment time
  if (setState == LOW && lastSetState == HIGH &&
      now - lastSetPress > 200) {

    lastSetPress = now;

    if (!timerRunning) {
      timerFinished = false;
      treatmentMinutes++;

      if (treatmentMinutes > 6) {
        treatmentMinutes = 1;
      }

      remainingSeconds = treatmentMinutes * 60UL;

      Serial.print("Timer Set: ");
      Serial.println(treatmentMinutes);
    }
  }

  // BUTTON 2: Start / Stop / Reset timer
  if (startState == LOW && lastStartState == HIGH &&
      now - lastStartPress > 200) {

    lastStartPress = now;

    if (timerFinished) {
      timerFinished = false;
      remainingSeconds = treatmentMinutes * 60UL;
    }
    else if (timerRunning) {
      timerRunning = false;
      remainingSeconds = treatmentMinutes * 60UL;
    }
    else if (warningLevel < 2) {
      timerRunning = true;
      timerStart = now;
    }
  }

  // BUTTON 3: Change display mode
  if (displayState == LOW && lastDisplayState == HIGH &&
      now - lastDisplayPress > 200) {

    lastDisplayPress = now;

    displayPage++;

    if (displayPage > 3) {
      displayPage = 0;
    }
  }

  lastSetState = setState;
  lastStartState = startState;
  lastDisplayState = displayState;
}


// ---------- SENSOR READINGS ----------
void sampleSensors() {
  static unsigned long lastSample = 0;

  if (samplesStored > 0 &&
      millis() - lastSample < 100) {
    return;
  }

  lastSample = millis();

  brightness = map(
    analogRead(potentiometer),
    0, 1023, 0, 255
  );

  lightReadings[sampleIndex] = analogRead(light_sensor);
  tempReadings[sampleIndex] = analogRead(temp_sensor);

  if (samplesStored < N) {
    samplesStored++;
  }

  sampleIndex++;

  if (sampleIndex >= N) {
    sampleIndex = 0;
  }

  long lightSum = 0;
  long tempSum = 0;

  for (byte i = 0; i < samplesStored; i++) {
    lightSum += lightReadings[i];
    tempSum += tempReadings[i];
  }

  filteredLight = lightSum / samplesStored;
  float adc = (float)tempSum / samplesStored;

  temperatureOK = (adc > 1.0 && adc < 1022.0);

  if (temperatureOK) {
    float thermistorR =
      FIXED_R * adc / (1023.0 - adc);

    float kelvin =
      1.0 / (1.0 / 298.15 +
      log(thermistorR / NOMINAL_R) / BETA);

    temperatureC = kelvin - 273.15;
  }
}


  
void checkWarnings() {
  warningLevel = 0;

  if (!temperatureOK || temperatureC > 43.0) {
    warningLevel = 2; 
  }
  else if (temperatureC >= 41.0) {
    warningLevel = 1; 
  }
  
  int lightPercent = filteredLight * 100L / 1023L;

  if (!LIGHT_INCREASES_WITH_BRIGHTNESS) {
    lightPercent = 100 - lightPercent;
  }

  if (lightPercent >= HIGH_LIGHT_LIMIT) {
    warningLevel = 2;  
  }
  else if (lightPercent >= MEDIUM_LIGHT_LIMIT &&
           warningLevel < 1) {
    warningLevel = 1;
  }

  bool redOn = warningLevel > 0;
  bool greenOn = warningLevel == 1;

  if (RGB_COMMON_ANODE) {
    digitalWrite(red_led, !redOn);
    digitalWrite(green_led, !greenOn);
    digitalWrite(rgb_blue_led, HIGH);
  }
  else {
    digitalWrite(red_led, redOn);
    digitalWrite(green_led, greenOn);
    digitalWrite(rgb_blue_led, LOW);
  }
}

void updateTimer() {
  if (timerRunning) {
    unsigned long totalMilliseconds =
      (unsigned long)treatmentMinutes * 60000UL;

    unsigned long elapsed = millis() - timerStart;

    if (elapsed >= totalMilliseconds) {
      timerRunning = false;
      timerFinished = true;
      remainingSeconds = 0;
    }
    else {
      remainingSeconds =
        (totalMilliseconds - elapsed + 999UL) / 1000UL;
    }
  }
}

void updateBlueLED() {
  if (!timerRunning ||
      warningLevel == 2 ||
      brightness == 0) {

    digitalWrite(blue_led, LOW);
  }
  else if (brightness == 255) {
    digitalWrite(blue_led, HIGH);
  }
  else {
    const unsigned long period = 1538UL;

    unsigned long onTime =
      period * brightness / 255UL;

    if (micros() % period < onTime) {
      digitalWrite(blue_led, HIGH);
    }
    else {
      digitalWrite(blue_led, LOW);
    }
  }
}

// ---------- BUZZER ----------
void updateBuzzer() {
  static bool beeping = false;

  bool shouldBeep =
    timerFinished && (millis() % 1000UL < 500UL);

  if (shouldBeep && !beeping) {
    tone(buzzer, 2000);
    beeping = true;
  }
  else if (!shouldBeep && beeping) {
    noTone(buzzer);
    beeping = false;
  }
}


void updateDisplay() {
  static unsigned long lastUpdate = 0;
  static byte lastPage = 255;

  if (millis() - lastUpdate < 100 && displayPage == lastPage) {
    return;
  }

  lastUpdate = millis();
  lastPage = displayPage;

  switch (displayPage) {

    case 0:
      sevseg.setNumber(brightness * 100L / 255);
      break;

    case 1:
      sevseg.setNumber(filteredLight * 100L / 1023);
      break;

    case 2:
      if (temperatureOK && temperatureC >= 0 && temperatureC <= 99.9) {
        sevseg.setNumberF(temperatureC, 1);
      }
      else {
        sevseg.setChars("----");
      }
      break;

    case 3:
      int minutes = remainingSeconds / 60;
      int seconds = remainingSeconds % 60;
      sevseg.setNumber(minutes * 100 + seconds, 2);
      break;
  }
}

void printReadings() {
  static unsigned long lastPrint = 0;

  if (millis() - lastPrint >= 500) {
    lastPrint = millis();

    Serial.print("Brightness: ");
    Serial.println(brightness);

    Serial.print("Filtered Light: ");
    Serial.println(filteredLight);

    Serial.print("Temperature: ");

    if (temperatureOK) {
      Serial.println(temperatureC);
    }
    else {
      Serial.println("Sensor Error");
    }

    Serial.print("Time Remaining: ");
    Serial.println(remainingSeconds);

    Serial.print("Display Mode: ");
    Serial.println(displayPage);

    Serial.println("---------------");
  }
}
