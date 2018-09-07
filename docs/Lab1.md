
# Lab 1

Subteam 1: Felicia, Ben, Ian

Subteam 2: Michael, Ria

Original blink code:

![Original blink code](/media/original_blink.png)

~~~
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
~~~


Uploading Arduino Code:

<iframe width="560" height="315" src="https://www.youtube.com/embed/e6g0XYT9Gvw" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>


On-board LED blinking:

<iframe width="560" height="315" src="https://www.youtube.com/embed/TRF9JSS3JlQ" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>


Modifications to blink code:
 
![Modified blink code (setup)](/media/blink_setup.png)
 

![Modified blink code (loop)](/media/blink_loop.png)

~~~
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(2, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(3, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(7, HIGH);
  digitalWrite(8, HIGH);
  digitalWrite(9, HIGH);
  digitalWrite(10, HIGH);
  digitalWrite(11, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);
  delay(500);  
  // wait for a second
  digitalWrite(2, LOW);   // turn the LED on (LOW is the voltage level)
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
  delay(500);                       // wait for a second
}
~~~

External Blinking LED:

<iframe width="560" height="315" src="https://www.youtube.com/embed/J9jq3WLRFCI" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>


Lots of external blinking LEDs!!! (all analog outputs work):
<iframe width="560" height="315" src="https://www.youtube.com/embed/70jK-FcvBzQ" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>


Potentiometer Wiring Setup:

![Potentiometer Setup](/media/PotentiometerSetup.png)  


Analog Read Function:

![Analog read function](/media/analog_read.png)

~~~
int analogPin = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int val = analogRead(analogPin);
  float inV = val*.0049;
  Serial.print(inV);
  Serial.println (" V");
  delay(1000);
}
~~~

Potentiometer Analog Value Output:

<iframe width="560" height="315" src="https://www.youtube.com/embed/9KBB9-9VRFg" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>


Changing LED Brightness:

<iframe width="560" height="315" src="https://www.youtube.com/embed/9KBB9-9VRFg" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>


Servo Wiring Setup:

![Servo Wiring](/media/servo_wiring.png)
 
 
 Analog Write Function:

![Analog write using potentiometer](/media/PWM.png)

~~~
int analogPin = 0;
int ledPin = 3;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int val = analogRead(analogPin);
  float convert = (float(val)/1023) * 255;
  int pwm = int(convert);
  Serial.println(val);
  Serial.println(pwm);
  analogWrite(ledPin, pwm);
  delay(250);
}
~~~


Servo Motor Controls Using Potentiometer:

<iframe width="560" height="315" src="https://www.youtube.com/embed/B7ZHxNOI4Dc" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>


Watch our robot make a lil square!!

<iframe width="560" height="315" src="https://www.youtube.com/embed/ZfidmBTznY8" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>
