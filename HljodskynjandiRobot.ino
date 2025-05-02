///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
/*

  MEKA&STTÆ-C
  Hljóðskynjandi Róbót

  Author: Vilberg Kristinsson
  Date: 1/5/2025

  A robot that detects sound and light, and reacts according to chosen settings.
    3 Sound detectors to calculate directionality
    1 LDR sensor
    2 Servo motors to move arms
    1 Stepper motor to spin body
    1 OLED screen to display menu and info
    4 Buttons to navigate settings
    1 Button to turn ON/OFF
    1 RGB light unit
    2 Green LEDs

*/
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


//////////////////////
///Libraries//////////
//////////////////////

#include <U8g2lib.h>  //  OLED library
#include <Wire.h>     //  I2C library
#include <Servo.h>    //  Servo library
#include <AccelStepper.h> //Stepper library

#include "GlobalVariables.h"
#include "SoundFunctions.h"
#include "pcf8574Functions.h"  
#include "screenAndMotorFunctions.h" 

//////////////////////
///Pins/Addresses/////
//////////////////////

// Servo
const int8_t servoPinR = 9;
const int8_t servoPinL = 10;

// Stepper
AccelStepper stepper(AccelStepper::FULL4WIRE, 2, 7, 4, 8);  // IN1-IN3-IN2-IN4

// LDR
const int8_t ldrPin = A3;

// LED
const int8_t mainLedPin1 = 11;
const int8_t mainLedPin2 = 12;

// OLED
U8G2_SSD1306_48X64_WINSTAR_F_HW_I2C u8g2(U8G2_R3, /* reset=*/ U8X8_PIN_NONE); // Address  = 0x3C  // U8G2_R3 = rotate 0-3 (0,90,180,270)

// 8574
const int8_t PCF8574_ADDRESS = 0x20;


//////////////////////
// Debug//////////////
//////////////////////

bool debug = 0;


//////////////////////
///Delays/////////////
//////////////////////

// Debounce
unsigned long lastPressTime = 0; 
const int debounceDelay = 500;  // milliseconds

// Servo
unsigned long lastServoTime = 0;
const int servoDelay = 800;

// Stepper
unsigned long lastStepperDirectionChange = 0;
const int stepperCheckDelay = 1000;
unsigned long lastStepperDanceTime = 0;
unsigned stepperDanceTimer =  500;

// Beat count
const int bpmIdleTimeout = 5000;  // millis of silence = reset
bool beatDetected = true;

// LED main
unsigned long ledOffTime = 0;
const int ledFlashDuration = 80;  

// Direction check
unsigned long lastDirectionCheckTime = 0;
const int directionCheckDelay = 200;

// Dance delay
unsigned long lastDanceDelay = 0;
const int danceDelay = 150;


//////////////////////
///BPM////////////////
//////////////////////

const uint8_t bpmSmoothingBuffer = 10;
int bpm = 0;
int bpmHistory[bpmSmoothingBuffer];
uint8_t bpmCount = 0;  // Number of valid entries in bpmHistory
uint8_t bpmIndex = 0;


//////////////////////
///Flags//////////////
//////////////////////

bool stepperMoving = false;

// Switch inputs
bool switchFlag0 = false;
bool switchFlag1 = false;
bool switchFlag2 = false;
bool switchFlag3 = false;

// LED outputs
bool ledRed = false;
bool ledGreen = false;
bool ledBlue = false;
bool ledExtra = false;  // Currently unused

// Stepper direction flags
bool stepperDance = false;
int8_t lastDirection = 0;

// LDR sensor value
int ldrValue = 0;
int ldrThreshold = 800;


//////////////////////
///Motor stuff////////
//////////////////////

// Servo class members
Servo armServoRight;
Servo armServoLeft;


//////////////////////
///Sound stuff////////
//////////////////////

// MicValues struct members
MicValues micValueRight;
MicValues micValueLeft;
MicValues micValueFront;

//////////////////////
/// Dance function////
//////////////////////

void dance()
{
  static int8_t lastRandom = -1;
  int8_t choice = random(0, 6); // Random number fom 0-5

  while (choice == lastRandom)  // To stop repeat numbers
  {
    choice = random(0, 6);
  }
  lastRandom = choice;

  if((millis() - lastDanceDelay > (danceDelay)) // If delays are met
  && (millis() - lastServoTime > servoDelay))
  {
    lastDanceDelay = millis();

    switch (choice)
    {
      case 0: 
        armServoRight.write(180);
        armServoLeft.write(0);
        break;
      case 1: 
        armServoRight.write(144);
        armServoLeft.write(36);
        break;
      case 2: 
        armServoRight.write(108);
        armServoLeft.write(72);
        break;
      case 3: 
        armServoRight.write(72);
        armServoLeft.write(108);
        break;
      case 4: 
        armServoRight.write(36);
        armServoLeft.write(144);
        break;
      case 5: 
        armServoRight.write(0);
        armServoLeft.write(180);
        break;
     }
    
    if (stepperDance == false)  // Toggle between stepper two directions corresponding to (true/false)
    {
      stepperDirectionChange(-1, 125, false); // direction, stepperSpeed, stepperWait
      stepperDance = !stepperDance; // Toggle to the other direction
      stepperMoving = true; // Flag

      if ((millis() - lastStepperDanceTime) > (0.1 * stepperDanceTimer))  // Stop after (0.1 * timer)
      {
        stepperDirectionChange(-3, 0, true); // direction, stepperSpeed, stepperWait
        resetMicValues(); // reset mic values to stop contaminated readings (stepper moving sometimes reads as sound)
        stepperMoving = false;
      }
    }
    else
    {
      stepperDirectionChange(-2, -125, false); // direction, stepperSpeed, stepperWait
      stepperDance = !stepperDance;
      stepperMoving = true;

      if ((millis() - lastStepperDanceTime) > (0.1 * stepperDanceTimer))  // Stop after (0.1 * timer)
      {
        stepperDirectionChange(-3, 0, true); // direction, stepperSpeed, stepperWait
        resetMicValues(); // reset mic values to stop contaminated readings (stepper moving sometimes reads as sound)
        stepperMoving = false;
      }
    }

    if ((millis() - lastStepperDanceTime) > (4 * stepperDanceTimer)) 
    {
      lastStepperDanceTime = millis();   // Reset stepperDanceTimer reference
    }
  }
} 

//////////////////////
///Setup//////////////
//////////////////////

void setup() 
{
  // Pin setup
  pinMode(ldrPin, INPUT); 
  pinMode(mainLedPin1, OUTPUT); // Main LED 1 
  pinMode(mainLedPin2, OUTPUT); // Main LED 2 
  pinMode(13, OUTPUT);  // Onboard LED-

  // I2C
  Wire.begin(); // Initialize Wire library

  // OLED
  u8g2.begin(); // Initialize u8g2 library

  // PCF8574 - P0-P3 (bits 1-4) = 1 input (pull-up), P4-P7 (bits 5-8) = 0 outputs (default LOW)
  writePCF8574(0b00001111); //(0b00001111); // Bit Position (P7 → P0 (bit 8 → bit 1)

  // Servo
  armServoRight.attach(servoPinR);  //Attach servo arms to pins
  armServoLeft.attach(servoPinL);

  // Stepper
  stepper.setMaxSpeed(1000);  // Max speed (steps/sec)
  stepper.setAcceleration(500); // Acceleration (steps/sec^2)

  if (debug == 1) // Debug stuff, not important (debug == 0 during normal operation)
  {
    Serial.begin (115200);
  }
}

//////////////////////
///Main loop//////////
//////////////////////

void loop() 
{

  // 8574 read/write
  readSwitches();  // Read switches and update flags
  writeRGB();  // Control RGB light

  // Mic
  if (stepperMoving == false) // If stepperMoving flag is not true
  {
    updateMicValues();  // Call function to update mic values
    getMicValuesHighLow();  // Call function to insert High and Low values into corresponding arrays
  }

  // Blinking LEDs turned off after timer
  if (millis() > ledOffTime)
  {
    digitalWrite(mainLedPin1, LOW);
    digitalWrite(mainLedPin2, LOW);
    digitalWrite(13, LOW);
  }

  // Reset BPM if idle (No BPM detected for bpmIdleTimeout millis)
  if (millis() - lastBeatTime > bpmIdleTimeout) 
  {
    bpm = 0;
    for (int8_t i = 0; i < bpmSmoothingBuffer; i++)
    {
      bpmHistory[i] = 0;  // Array that stores BPM history for smoothing out value over time
    }
    bpmIndex = 0; // Tracks current index in buffer
  }

  if (switchFlag0 != true)// If in Standby MENU mode
  {
    screenOutput(-1); // Call screenOutput function with value to show MENU screen
  }
  else if (switchFlag0 == true) // Not in MENU mode
  {
    // LDR
    ldrValue = analogRead(ldrPin);

    if (debug == 1) // Serial monitor ldrValue
    {
      Serial.print(ldrValue);
    }
    
    // Interval for executing on sound values
    if ((millis() - lastDirectionCheckTime > directionCheckDelay)
    && (millis() - lastServoTime > servoDelay)) // To ignore servo (and stepper) noise triggering movement right away
    {
      lastDirectionCheckTime = millis();  // Reset timer
      
      screenOutput(0);  // Show default screen output
      stepperMoveDirection(); // Call stepperMoveDirection function with mic values
      resetMicValues(); // H/L values reset
    }

    detectBeat(); // Call beat detection function
    stepperDanceTimer = (10 * bpm); // Set stepperDanceTimer relative to perceived BPM

    if ((beatDetected == true)  // If beat detected and LDR threshold met (and flag for LED showing == true)
    && (ldrValue < ldrThreshold)
    && (switchFlag2 == true))
    {
      digitalWrite(mainLedPin1, HIGH);  // Set LEDs high
      digitalWrite(mainLedPin2, HIGH);
      digitalWrite(13, HIGH);
      ledOffTime = millis() + ledFlashDuration; // Timer reset
      beatDetected = false; // beatDetected flag set to false
    
      if (switchFlag3 == true)  // If dance flag == true
      {
        dance();  // Call dance function (servo and stepper movement using beat + LEDs)
      }
    }

    // Start/stop stepper according to flags
    if(ldrValue < ldrThreshold)
    {
      if (millis() - lastStepperDirectionChange > (0.75 * stepperCheckDelay))
      {
        runStepper(true); // stepperStop == true
        resetMicValues(); // reset mic values to stop contaminated readings (stepper moving sometimes reads as sound, especially if stuff is bumping around)
      }
      else
      {
        runStepper(false); // stepperStop == false
      }
    }
  }
}
  

