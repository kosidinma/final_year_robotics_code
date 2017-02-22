#include <SoftwareSerial.h> //arduino serial library
SoftwareSerial Freescaleboard(1,0); //set bluetooth module name
#include <Servo.h>
#include "IOpins.h"
//#include "Constants.h"
#define trigPin 10
#define echoPin 12

#define trigPin2 8
#define echoPin2 9


//-------------------------------------------------------------- define global variables --------------------------------------------

unsigned long AnalogueVolts;
unsigned long Volts;
unsigned int LeftAmps;
unsigned int RightAmps;
unsigned long chargeTimer=millis();
unsigned long leftoverload;
unsigned long rightoverload;
int highVolts;
int startVolts;
int Leftspeed=0;
int Rightspeed=0;
int Speed;
int Steer;
byte Charged=1;                                               // 0=Flat battery  1=Charged battery
int Leftmode;                                               // 0=reverse, 1=brake, 2=forward
int Rightmode;                                              // 0=reverse, 1=brake, 2=forward
byte Leftmodechange=0;                                        // Left input must be 1500 before brake or reverse can occur
byte Rightmodechange=0;                                       // Right input must be 1500 before brake or reverse can occur
int LeftPWM=198;                                                  // PWM value for left  motor speed / brake
int RightPWM=198;                                                 // PWM value for right motor speed / brake
//int data = 2;
int servo[7];
int move = 0;
int val;
double vel = 0;
double ratio;
double dark;
double light;
double dist_covered = 0;
long last=0;
int milliseconds=1950; // the time it takes each reading
//-------------------------------------------------------------- define servos ------------------------------------------------------

/*
Servo Servo0;                                                 // define servos
Servo Servo1;                                                 // define servos
Servo Servo2;                                                 // define servos
Servo Servo3;                                                 // define servos
Servo Servo4;                                                 // define servos
Servo Servo5;                                                 // define servos
Servo Servo6;                                                 // define servos
*/
void setup()
{
  //------------------------------------------------------------ Initialize Servos ----------------------------------------------------
 /*
  Servo0.attach(S0);                                          // attach servo to I/O pin
  Servo1.attach(S1);                                          // attach servo to I/O pin
  Servo2.attach(S2);                                          // attach servo to I/O pin
  Servo3.attach(S3);                                          // attach servo to I/O pin
  Servo4.attach(S4);                                          // attach servo to I/O pin
  Servo5.attach(S5);                                          // attach servo to I/O pin
  Servo6.attach(S6);                                          // attach servo to I/O pin
  */


  //------------------------------------------------------------ Initialize I/O pins --------------------------------------------------

  pinMode (Charger,OUTPUT);                                   // change Charger pin to output
  digitalWrite (Charger,0);                                   // disable current regulator to charge battery
  Freescaleboard.begin(9600);
 // Freescaleboard.println("Bluetooth On please press 1 or 0 blink LED ..");
  


  //Serial.begin (9600);
  
}


void loop()
{
  char data = '0';
  bool control = true;
  bool control2 = true;
  //------------------------------------------------------------ Check battery voltage and current draw of motors ---------------------

  //Volts=analogRead(Battery);                                  // read the battery voltage
  LeftAmps=analogRead(LmotorC);                               // read left motor current draw
  RightAmps=analogRead(RmotorC);                              // read right motor current draw
  int distance;
  int distance2;
/*
  Servo0.writeMicroseconds(data);                             // set servo to default position
  Servo1.writeMicroseconds(data);                             // set servo to default position
  Servo2.writeMicroseconds(data);                             // set servo to default position
  Servo3.writeMicroseconds(data);                             // set servo to default position
  Servo4.writeMicroseconds(data);                             // set servo to default position
  Servo5.writeMicroseconds(data);                             // set servo to default position
  Servo6.writeMicroseconds(data);                             // set servo to default position
*/
  // --------------------------------------------------------- Code to drive dual "H" bridges --------------------------------------
                                                    // Serial mode via D0(RX) and D1(TX)
     if (Freescaleboard.available()) //check for serial
 {
                // read the incoming byte:
             //  Freescaleboard.println("Battery Voltage:");k
             // Freescaleboard.println(Volts);
               ////////starttt sensor
               while (1){
                 distance = Ultrasensor();
                 distance2 = Ultrasensor2();
                 AnalogueVolts=analogRead(Battery);  
                 Volts = AnalogueVolts/68.307; //scale factor  
                 speed_check();   
                 if (distance < 45)
                 {
                   control = false;
                 }
                 else
                 {
                   control = true;
                 }
                 if (distance2 < 45)
                 {
                   control2 = false;
                 }
                 else
                 {
                   control2 = true;
                 }
                 
                 data = Freescaleboard.read();
                 mode(data,control,control2,distance,distance2);
                 //double speed_covered = speed_check();
                 Freescaleboard.print("init");  //initial variable for testing
                 Freescaleboard.print(",");
                 Freescaleboard.print(vel);
                 Freescaleboard.print("rpm");
                 Freescaleboard.print(",");
                 Freescaleboard.print(dist_covered);
                 Freescaleboard.print("m");
                 Freescaleboard.print(","); //delimiter byte
                 Freescaleboard.print(Volts);
                 Freescaleboard.print("V");
                 Freescaleboard.println();
                 delay(50);// prepare for next data ...
 }
 }

}



void mode( char data, bool control, bool control2, int distance, int distance2)
{
    if (data == '2' && control ) //move forward
    { 
      // left motor forward    
      analogWrite(LmotorA,0);
      analogWrite(LmotorB,LeftPWM);
      // right motor forward
      analogWrite(RmotorA,0);
      analogWrite(RmotorB,RightPWM);
    }
    if (data == '2' && !control ) //don't move as there's obstacle
    { 
      analogWrite(LmotorA,LeftPWM);
      analogWrite(LmotorB,LeftPWM);
      // right motor break
      analogWrite(RmotorA,RightPWM);
      analogWrite(RmotorB,RightPWM);
    }
    if (data == '0' || data == 0)//stop
    { 
      // left motor break   
      analogWrite(LmotorA,LeftPWM);
      analogWrite(LmotorB,LeftPWM);
      // right motor break
      analogWrite(RmotorA,RightPWM);
      analogWrite(RmotorB,RightPWM);
    }
    if (data == '1' && control2) //move back
    { 
      // left motor reverse   
      analogWrite(LmotorA,LeftPWM);
      analogWrite(LmotorB,0);
      // right motor reverse
      analogWrite(RmotorA,RightPWM);
      analogWrite(RmotorB,0);
    }
    if (data == '1' && !control2 ) //don't move as there's obstacle
    { 
      analogWrite(LmotorA,LeftPWM);
      analogWrite(LmotorB,LeftPWM);
      // right motor break
      analogWrite(RmotorA,RightPWM);
      analogWrite(RmotorB,RightPWM);
    }
    if (data == '3') //right turn
    { 
      // left motor forward   
      analogWrite(LmotorA,0);
      analogWrite(LmotorB,255);
      // right motor stop
      analogWrite(RmotorA,255);
      analogWrite(RmotorB,255);
    }
    if (data == '4') //left turn
    { 
      // left motor stop  
      analogWrite(LmotorA,255);
      analogWrite(LmotorB,255);
      // right motor forward
      analogWrite(RmotorA,0);
      analogWrite(RmotorB,255);
    }
     if (data == '5') //hand control
    { 
      if (distance < 8) //slow right turn
      {
      // left motor forward   
      analogWrite(LmotorA,0);
      analogWrite(LmotorB,255);
      // right motor stop
      analogWrite(RmotorA,255);
      analogWrite(RmotorB,255);
      }
      if (distance2 < 8) //slow left turn
    { 
      // left motor stop  
      analogWrite(LmotorA,255);
      analogWrite(LmotorB,255);
      // right motor forward
      analogWrite(RmotorA,0);
      analogWrite(RmotorB,255);
    }
    if (distance >=10 && distance2 >= 10) //stop
    { 
      analogWrite(LmotorA,LeftPWM);
      analogWrite(LmotorB,LeftPWM);
      // right motor break
      analogWrite(RmotorA,RightPWM);
      analogWrite(RmotorB,RightPWM);
    }
    }
    
    
  
}

long microsecondsToInches(long microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}

int Ultrasensor ()
{
  // establish variables for duration of the ping,
  // and the distance result in inches and centimeters:
  long duration, inches, cm;

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(trigPin, OUTPUT);// attach pin 3 to Trig
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(trigPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode (echoPin, INPUT);//attach pin 4 to Echo
  duration = pulseIn(echoPin, HIGH);

  // convert the time into a distance
  inches = microsecondsToInches(duration);
  cm = microsecondsToCentimeters(duration);
   
  return cm;
}

long microsecondsToInches2(long microseconds2)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds2 / 74 / 2;
}

long microsecondsToCentimeters2(long microseconds2)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds2 / 29 / 2;
}

int Ultrasensor2 ()
{
  // establish variables for duration of the ping,
  // and the distance result in inches and centimeters:
  long duration, inches, cm;

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(trigPin2, OUTPUT);// attach pin 3 to Trig
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(5);
  digitalWrite(trigPin2, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode (echoPin2, INPUT);//attach pin 4 to Echo
  duration = pulseIn(echoPin2, HIGH);

  // convert the time into a distance
  inches = microsecondsToInches2(duration);
  cm = microsecondsToCentimeters2(duration);
  
  return cm;
}


void speed_check()
{
     int val = analogRead(A1);
     if (val >= 100)
       dark=dark+1;
     else
       light=light+1;
     if ((millis() - last) >= milliseconds) //take each reading after 1.95 second
     {
       if (dark == 0 || light == 0)
         ratio = 0.0;
       else
         ratio = light/(dark+light);
       vel = (ratio / 0.3459) * 217.1429/3.0; 
       if (vel > 217)
         vel = 217.1429/3.0;
       last = millis();
       dark = 0;
       light = 0;
       dist_covered = dist_covered + (vel * 0.06 * 0.10472 * milliseconds/1000.0); //v = rw
     }
     else
     vel = vel;
}
