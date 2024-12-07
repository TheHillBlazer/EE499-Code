/* Team 3- Automatic Delivery Cart Arduino code */
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <Servo.h> //used to drive the car

/*UltraSonic Pins- SENSOR LAYOUT MAY NEED TO BE ADJUSTED */
#define trig1 46 //front sensor
#define echo1 57 //front sensor
#define trig2 44 //right sensor
#define echo2 45 //right sensor
#define trig3 42 //left sensor
#define echo3 43 //left sensor

int frontDistance; //
int leftDistance;
int rightDistance;
int number;// switch case number

/*Used in Motor Driver functions*/
Servo myservo; // Initializing PWM variable name for Neutral and Brake
Servo myservo1; // Initializing PWM variable name for Forward and Reverse
Servo myservo2; //Initializing PWM variable name for Left and Right turn

int motorCase = 0;

void setup() {
 Serial.begin(9600);

 /*Set up for Motor Driver functions*/
 myservo.attach(10); //Initializing PWM Pin for Neutral and Brake
 myservo1.attach(11); //Initializing PWM Pin for Forward and Reverse
 myservo2.attach(12); //Initializing PWM Pin for Left and Right Turn
int pinsIN[] = {53,51,49}; //each pin is echo for ultrasound in this array
int pinsOUT[] = {13,52,50,48}; //each pin is to set for output, pin 13 is for motor relay; 52, 50, 49 are trig on ultrasounds 

  for (int i=0; i<3; i++) {
     pinMode(pinsIN[i],INPUT);
  } //for (int i=0; i=3; i++)
  for ( int i=0; i<4; i++) {
     pinMode(pinsOUT[i],OUTPUT);
  } //for(int i=0; i<4; i++)

  //Serial.println("Ultrasonic sensor:");

}//void setup

void loop() {

 //Ultrasonic Sensor code
 frontDistance = readUltraSound1(); //gets how far objects are in front of robot
 //Serial.println("Ultrasonic sensor Front:");
 rightDistance = readUltraSound2(); //gets how far objects are on the right of the robot
 //Serial.println("Ultrasonic sensor Right:");
 leftDistance = readUltraSound3(); //gets how far objects are on the left of the robot
 //Serial.println("Ultrasonic sensor Left:");
Serial.print("Front:");
Serial.print(frontDistance);
Serial.print("| ");
Serial.print("Right ");
Serial.print(rightDistance);
Serial.print("| ");
Serial.print("Left: ");
Serial.print(leftDistance);
Serial.println(" ");
//compareDist(frontDistance, rightDistance, leftDistance); UNCOMMENT THIS if the switch will be updated as motorCase is updated
  //This if structure is in the function at the bottom but I wasn't sure if the motorCase would update the switch 
  if(frontDistance > 100) {
    motorCase = 2; //case for going forward
    Serial.println( motorCase);
  }//if(frontDistance > 500)
  if(frontDistance < 100 & leftDistance > rightDistance){
    motorCase = 5; //will turn the wheels left
    Serial.println( motorCase);
    delay(1000);
    motorCase = 7; //will turn the wheels back straight
    Serial.println( motorCase);
  }//if(frontDistance < 50 && leftDistance >> rightDistance)
  if(frontDistance < 100 & leftDistance < rightDistance) {
    motorCase = 6; //will turn the wheels to the right
    Serial.println( motorCase);
    delay(1000); 
    motorCase = 7; //will turn the wheels back straight
    Serial.println( motorCase);
  }//if(frontDistance <50 && leftDistance << rightDistance)
  if(frontDistance < 30 & leftDistance <30 & rightDistance <30) {
    motorCase = 1; //this is the neutral case, sometimes needed to stop
    Serial.println("Case" + motorCase);
    motorCase = 3; //this is the brake case
    Serial.println("Case"+ motorCase);
    Serial.println("There is objects blocking my path and I am stuck");
  }//if(frontDistance<30 && leftDistance <30 && rightDistance <30)

 
 /*This read the serial line and uses the input to control the motors based on the switchcase */
 if (Serial.available() > 0) { //Checking for incoming serial data
    motorCase = Serial.read()-'0'; //Converting string encoded data into integer
    Serial.println(motorCase);
    } //if(Serial.available() >0)

 //This swithcase can either be controlled for the serial monitor or from the ultrasound compare distance case
 switch(motorCase){
      case 1:                       //Case for Neutral
      digitalWrite(13,HIGH);
      myservo.writeMicroseconds(1500);
      break;

      case 2:                      //Case for Forward
      digitalWrite(13, LOW);
      myservo1.write(120);
      break;

      case 3:                      //Case for Brake
      digitalWrite(13, HIGH);
      myservo.writeMicroseconds(1200);
      break;

      case 4:                     //Case for Reverse
      myservo.writeMicroseconds(1200); // PWM pulses for break
      delay(1000);
      digitalWrite(13,HIGH); // Switching gear to neutral
      myservo.writeMicroseconds(1500); // PWM pulses for neutral
      delay(1000);
      digitalWrite(13, LOW);
      myservo1.write(25);
      break;

      case 5:                    //Case for Left turn
      myservo2.write(50);
      break;

      case 6:
      myservo2.write(120);  //Case for Right turn
      break;
      
      case 7:                    //Case for wheels going straight
      myservo2.write(90);
      break;

      default:
      break;
   }//switch(number)
  
//Serial.println("Number:" + motorCase);
}//void loop()


//Read sensor data for each Ultrasound sensor
//front sensor on pins 52 and 53
float readUltraSound1 () {
  digitalWrite(trig1, LOW);
  delayMicroseconds(2);
  digitalWrite(trig1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig1, LOW);
  float frontDistance = pulseIn(echo1, HIGH)/58.00; //Equivalent to (340m/s*1us)/1
  return frontDistance;
}//float readUltraSound1

//right sensor on pins 50 and 51
float readUltraSound2 () {
  digitalWrite(trig2, LOW);
  delayMicroseconds(2);
  digitalWrite(trig2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig2, LOW);
  float rightDistance = pulseIn(echo2, HIGH)/58.00; //Equivalent to (340m/s*1us)/1
  return rightDistance;
}//float readUltraSound2

//left sensor on pins 48 and 49
float readUltraSound3 () {
  digitalWrite(trig3, LOW);
  delayMicroseconds(2);
  digitalWrite(trig3, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig3, LOW);
  float leftDistance = pulseIn(echo3, HIGH)/58.00; //Equivalent to (340m/s*1us)/1
  return leftDistance;
}//float readUltraSound3

//This function will compare the distance and the variable motorCase will be updated accordingly
/*int compareDist(frontDistance, rightDistance, leftDistance) {
  if(frontDistance > 500) {
    motorCase = 2; //case for going forward
  }//if(frontDistance > 500)
  if(frontDistance < 50 && leftDistance >> rightDistance){
    motorCase = 5; //will turn the wheels left
    delay(1000);
    motorCase = 7; //will turn the wheels back straight
  }//if(frontDistance < 50 && leftDistance >> rightDistance)
  if(frontDistance <50 && leftDistance << rightDistance) {
    motorCase = 6; //will turn the wheels to the right
    delay(1000); 
    motorCase = 7; //will turn the wheels back straight
  }//if(frontDistance <50 && leftDistance << rightDistance)
  if(frontDistance <30 && leftDistance <30 && rightDistance <30) {
    motorCase = 1; //this is the neutral case, sometimes needed to stop
    motorCase = 3; //this is the brake case
    Serial.println("There is objects blocking my path and I am stuck");
  }//if(frontDistance<30 && leftDistance <30 && rightDistance <30)
}int compareDist(frontDistance, rightDistance, leftDistance)*/


