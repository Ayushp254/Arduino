
const int redled_pin = 8; 
const int greenled_pin=7;
const int yellowled_pin=9;
const int button_pin=2;
int button_state=0;
int button_count=0;

void setup() {
  // put your setup code here, to run once:
pinMode(redled_pin, OUTPUT); 
pinMode(greenled_pin,OUTPUT);
pinMode(yellowled_pin,OUTPUT);
pinMode(button_pin, INPUT);

}

void ledBlink(int pinNumber, int numberBlinks, int blinkPeriod){
 for( int i = 0; i <= numberBlinks; i++)
{
digitalWrite(pinNumber, HIGH);
delay(blinkPeriod);
digitalWrite(pinNumber, LOW);
delay(blinkPeriod); 
}
}



void loop() {
  // put your main code here, to run repeatedly:
//digitalWrite(redled_pin, LOW);
//digitalWrite(greenled_pin,LOW);
//digitalWrite(yellowled_pin,LOW);

button_state = digitalRead(button_pin); 

if (button_state==HIGH)
{
  button_count++;
if (button_count==1)
{
  ledBlink(7,5,500);
  Serial.println("Green LED ON");
}
else if (button_count==2)
{
  ledBlink(9,10,200);
  Serial.println("Yellow LED ON");
}
else if (button_count==3)
{
  ledBlink(8,15,100);
  Serial.println("Red LED ON");
}
else if (button_count>3)
{
  button_count=0;
}
}
}



