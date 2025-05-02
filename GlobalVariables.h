//////////////////////
///Global variables///
//////////////////////

#ifndef GLOBALVARIABLES_H
#define GLOBALVARIABLES_H

#include "MicValues.h"  // MicValues struct

//////////////////////
///Pins&Addresses/////
//////////////////////

// Servo
extern const int8_t servoPinR;
extern const int8_t servoPinL;

// LDR
extern const int8_t ldrPin;

// LED main
extern const int8_t mainLedPin1;
extern const int8_t mainLedPin2;

// 8574
extern const int8_t PCF8574_ADDRESS;

// Stepper
extern AccelStepper stepper;

// OLED
extern U8G2_SSD1306_48X64_WINSTAR_F_HW_I2C u8g2;


//////////////////////
// Debug//////////////
//////////////////////

extern bool debug;


//////////////////////
///Delays/////////////
//////////////////////

// Debounce
extern unsigned long lastPressTime; 
extern const int debounceDelay;

// Servo
extern unsigned long lastServoTime;
extern const int servoDelay;

// Stepper
extern unsigned long lastStepperDirectionChange;
extern const int stepperCheckDelay;

// Beat count
extern const int bpmIdleTimeout;
extern bool beatDetected;


//////////////////////
///BPM////////////////
//////////////////////

extern const uint8_t bpmSmoothingBuffer;
extern int bpm;
extern int bpmHistory[];
extern uint8_t bpmCount;
extern uint8_t bpmIndex;


//////////////////////
///Flags//////////////
//////////////////////

// Switch inputs
extern bool switchFlag0;
extern bool switchFlag1;
extern bool switchFlag2;
extern bool switchFlag3;

// LED outputs
extern bool ledRed;
extern bool ledGreen;
extern bool ledBlue;
extern bool ledExtra;

// Stepper
extern int8_t lastDirection;
extern bool stepperMoving;

// LDR sensor value
extern int ldrValue;
extern int ldrThreshold;

//////////////////////
///Motor stuff////////
//////////////////////

// Servo class members
extern Servo armServoRight;
extern Servo armServoLeft;


//////////////////////
///Sound stuff////////
//////////////////////

// MicValues struct members
extern MicValues micValueRight;
extern MicValues micValueLeft;
extern MicValues micValueFront;

#endif