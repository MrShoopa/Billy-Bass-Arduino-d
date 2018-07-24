/*
  Make a DC Motor Move to Sound.
   This example code is in the public domain.
   Created by Donald Bell, Maker Project Lab (2016).
   Based on Sound to Servo by Cenk Ã–zdemir (2012)
   and DCMotorTest by Adafruit
*/

// include the Adafruit motor shield library
#include <Wire.h>
#include <QueueArray.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"



// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61);

// Select which 'port' M1, M2, M3 or M4. In this case, M1 for mouth and M2 for tail
Adafruit_DCMotor *mouthMotor = AFMS.getMotor(1);
Adafruit_DCMotor *headMotor = AFMS.getMotor(2);
Adafruit_DCMotor *tailMotor = AFMS.getMotor(3);

// Some other Variables we need
int SoundInPin = A1;
int LedPin = 12; //in case you want an LED to activate while mouth moves

//Base system variables
int afmsFreq = 1600; //Default is 1600 (1.6kHz)
int audioSensitivity = 1023;
int motorDelay = 0;

//Audio threshold variables
boolean headMovementEnabled = false; //TODO: head motor is unable to move the head itself (reglue?)
int staticThreshold = 1;
int mouthAudioThreshold = 40;
int tailAudioThreshold = 5;

//Motor speed variables
int mouthMotorSpeed = 200;
int headMotorSpeed = 30;
int tailMotorSpeed = 200;



//Handled all by system (Don't touch)
boolean audioDetected = false;
int audioLastDetected = 0;
QueueArray <int> audioReadingQueue;
boolean headMoved;


// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("sup b");

  AFMS.begin(afmsFreq);
  if (afmsFreq == 0 || afmsFreq == 1600) {
    Serial.println("Adafruit Motor Shield booted");
  } else {
    Serial.println("AFMS enabled with frequency of " + String(afmsFreq));
  }

  // Set the speed to start, from 0 (off) to 255 (max speed)

  //mouth motor
  mouthMotor->setSpeed(0);
  Serial.println("Mouth motor speed set to " + String(mouthMotorSpeed) + ".");
  mouthMotor->run(FORWARD);  // turn on motor
  Serial.println("Mouth motor turned on!");

  mouthMotor->run(RELEASE);
  pinMode(SoundInPin, INPUT);
  pinMode(LedPin, OUTPUT);

  //head motor
  headMotor->setSpeed(0);
  Serial.println("Head motor speed set to " + String(headMotorSpeed) + ".");
  headMotor->run(FORWARD); // turn on motor
  Serial.println("Head motor turned on!");

  headMotor->run(RELEASE);
  pinMode(SoundInPin, INPUT);

  //tail motor
  tailMotor->setSpeed(0);
  Serial.println("Tail motor speed set to " + String(tailMotorSpeed) + ".");
  tailMotor->run(FORWARD); // turn on motor
  Serial.println("Tail motor turned on!");

  tailMotor->run(RELEASE);
  pinMode(SoundInPin, INPUT);

  Serial.println("Billy Bass is GO!");
}

void loop() {
  uint8_t i;

  //Reading values from analog pin
  int sensorValue = analogRead(SoundInPin);
  sensorValue = map(sensorValue, 0, 1023, 0, audioSensitivity); // We Map it here down to the possible range of  movement.
  
  Serial.println("Audio - Input value: " + String(sensorValue));

  //int LEDValue = map(sensorValue,0,512,0,255); // we Map another value of this for LED that can be a integer betwen 0..255
  //int MoveDelayValue = map(sensorValue,0,255,0,sensorValue);  // note normally the 512 is 1023 because of analog reading should go so far, but I changed that to get better readings.

  //Passive movement (Moves the head when it detects any audio, and moves back when it becomes silent for a while)
  if (headMovementEnabled) {
    //Gathering history
    audioReadingQueue.enqueue(sensorValue);
      //Serial.println("QUEUE SIZE: " + String(audioReadingQueue.count()));
    if (audioReadingQueue.count() == 50) {
      if (audioReadingQueue.peek() > staticThreshold) {
        audioLastDetected = 0;
        audioDetected = true;
      } else {
        audioLastDetected++;
        audioDetected = false;
      }
      audioReadingQueue.pop();
      //Serial.println("Audio - Last detected " + String(audioLastDetected) + " steps ago");
    }
    //Moving heads according to case
    if (audioLastDetected < 50 && headMoved == false) {
      headMotor->run(FORWARD);
      for (i = headMotorSpeed; i < 255; i++) {
        headMotor->setSpeed(i);
      }
      headMotor->setSpeed(0);
      headMoved = true;

      Serial.println("I'm ALIVEEEEE");
    } else if (audioLastDetected < 50) {
      //hold position
    } else if (headMoved == true) {
      headMotor->run(BACKWARD);
      for (i = headMotorSpeed; i < 255; i++) {
        headMotor->setSpeed(i);
      }
      headMotor->setSpeed(0);
      headMoved = false;

      Serial.println("Imma be sleepin");
    } else {
      //hold position
    }
  }

  //Active movement
  if (sensorValue > mouthAudioThreshold) { // to cut off some static readings
    delay(motorDelay);  // a static delay to smooth things out...
    // now move the motor
    mouthMotor->run(FORWARD);
    for (i = mouthMotorSpeed; i < 255; i++) {
      mouthMotor->setSpeed(i);
    }

    if (sensorValue > tailAudioThreshold) { // to cut off some static readings
      delay(motorDelay);  // a static delay to smooth things out...
      // now move the motor
      tailMotor->run(BACKWARD);
      for (i = tailMotorSpeed; i < 255; i++) {
        tailMotor->setSpeed(i);
      }
    }

    analogWrite(LedPin, sensorValue);
    // and do that move in this delay time

    mouthMotor->run(RELEASE);
    tailMotor->run(RELEASE);
    delay(motorDelay);
  }



  // Done.
  // turn off the led again.
  analogWrite(LedPin, 0);
  // and this repeats all the time.
}


