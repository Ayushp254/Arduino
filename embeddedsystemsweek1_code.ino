int greenPin= 13;
int sevenSegmentA = 10;
int sevenSegmentB = 9;
int sevenSegmentC = 8;
int sevenSegmentD = 7;
int sevenSegmentE = 6;
int sevenSegmentF = 5;
int sevenSegmentG = 4;
int  buttonPin1state=0;
int buttonPin2state=0;
int buttonPin3state=0;
int buttonPin1=A0;
int buttonPin2=A1;
int buttonPin3=A2;
int greenled_state=0;
int button2counter=1;
int lastButton1state= 0;
bool lastButton2state=LOW;
int buzzer=2;

void sevenSegment(bool Astate, bool Bstate, bool Cstate, 
 bool Dstate, bool Estate, bool Fstate, bool Gstate)
{
  digitalWrite(sevenSegmentA, Astate);
  digitalWrite(sevenSegmentB, Bstate);
  digitalWrite(sevenSegmentC, Cstate);
  digitalWrite(sevenSegmentD, Dstate);
  digitalWrite(sevenSegmentE, Estate);
  digitalWrite(sevenSegmentF, Fstate);
  digitalWrite(sevenSegmentG, Gstate);
}


void setup() {
  // put your setup code here, to run once:
  {
  Serial.begin(9600);
  pinMode(sevenSegmentA, OUTPUT);
  pinMode(sevenSegmentB, OUTPUT);
  pinMode(sevenSegmentC, OUTPUT);
  pinMode(sevenSegmentD, OUTPUT);
  pinMode(sevenSegmentE, OUTPUT);
  pinMode(sevenSegmentF, OUTPUT);
  pinMode(sevenSegmentG, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2,INPUT_PULLUP);
  pinMode(buttonPin3,INPUT_PULLUP);
  Serial.begin(9600);
  
}

}

void loop() {
  // put your main code here, to run repeatedly:
  buttonPin1state=digitalRead(buttonPin1);
 // Serial.println(buttonPin1state);
  //greenled_state=digitalRead(greenPin);
  if (buttonPin1state==0 && lastButton1state==1) 
  {
    button2counter=1;
    if (digitalRead(greenPin)==0)
    {
      digitalWrite(greenPin,1);
      sevenSegment(1,0,0,1,1,1,0);
   //   Serial.println("Turn On");
      delay(1000);
    } 
    else if(digitalRead(greenPin)==1)
    {
      digitalWrite(greenPin,0);
      sevenSegment(1,0,1,1,0,1,1);
 //     Serial.println("Turn Off");
      delay(1000);
    }
  }
 
  lastButton1state=buttonPin1state;



  buttonPin2state=digitalRead(buttonPin2);
  buttonPin3state=digitalRead(buttonPin3);
  Serial.println(buttonPin2state);

if (buttonPin2state == LOW && lastButton2state == HIGH)
{
    button2counter++;
    
    Serial.println("buttonPressed");

    if (button2counter == 2)
    {  
      sevenSegment(1,1,1,1,1,1,1);
      Serial.println("B");

    }
    else if (button2counter == 3)
    {
        sevenSegment(1,0,0,0,1,1,1);
        Serial.println("F");
    }
    else if (button2counter == 4)
    {
        button2counter = 1;
        sevenSegment(1,0,0,1,1,1,0);
    }

  }
  lastButton2state = buttonPin2state;
  if(buttonPin3state==LOW)
    { 
      if(button2counter==2)
      {
        Serial.println("Buzzer for B");
        tone(buzzer, 659, 500);
        delay(500);
      }
      if(button2counter==3)
      { 
        Serial.println("Buzzer for F");
        tone(buzzer, 587, 500);
        delay(500);
      }
      if(button2counter==1)
      {
        Serial.println("Buzzer for C");
        tone(buzzer,523,500);
        delay(500);
      }
    }
}


