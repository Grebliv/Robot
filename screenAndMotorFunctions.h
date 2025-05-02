//////////////////////
///Functions//////////
//////////////////////

#ifndef MOTORFUNCTIONS_H
#define MOTORFUNCTIONS_H

#include "GlobalVariables.h"
#include "pcf8574Functions.h"

//////////////////////
///Functions//////////
//////////////////////

// Screen
// Output info on screen
void screenOutput (int direction)
{
  //Default screenfill
  if (direction != -1)
  {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);  // Fontsize = 08
    
    u8g2.drawStr(0, 10, "F:");
    u8g2.setCursor(15, 10);
    u8g2.print(micValueFront.diff);

    u8g2.drawStr(0, 20, "R:");
    u8g2.setCursor(15, 20);
    u8g2.print(micValueRight.diff);

    u8g2.drawStr(0, 30, "L:");
    u8g2.setCursor(15, 30);
    u8g2.print(micValueLeft.diff);

    u8g2.setCursor(0, 40);
    u8g2.print(ldrValue);

    u8g2.setCursor(45, 40);
    u8g2.print(bpm);
  }

  switch (direction)
  {
    case -1:  // Setup mode (Standby)
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);  // Fontsize = 14
      u8g2.drawStr(0, 10, "Setup mode");
      
      if (switchFlag1 == true)
      {
        u8g2.drawStr(0, 20, "Move: ON");
      }
      else
      {
        u8g2.drawStr(0, 20, "Move: OFF");
      }
      if (switchFlag2 == true)
      {
        u8g2.drawStr(0, 30, "LED: ON");
      }
      else
      {
        u8g2.drawStr(0, 30, "LED: OFF");
      }
      if (switchFlag3 == true)
      {
        u8g2.drawStr(0, 40, "Dance: ON");
      }
      else
      {
        u8g2.drawStr(0, 40, "Dance: OFF");
      }
      break;
      
    case 1: // Sound front
      u8g2.drawStr(40, 10, "(F)");
      break;

    case 2: // Sound Right
      u8g2.drawStr(40, 20, "(R)");
      break;

    case 3: // Sound left
      u8g2.drawStr(40, 30, "(L)");
      break;

    case 0:
      break;
  }
  u8g2.sendBuffer();  // Output result to OLED
}


// Stepper
// Run stepper
int stepperDirectionChange(int direction, int stepperSpeed, bool stepperWait)
{
  if ((stepperWait == false)  // Flag to tell stepper whether to wait for stepperCheckDelay
  && (direction != lastDirection))
  {
    lastStepperDirectionChange = millis();
    stepper.setSpeed(stepperSpeed);
  }
  else if ((stepperWait == true)  // Change direction if delay met
  && (millis() - lastStepperDirectionChange > stepperCheckDelay))
  {
    lastStepperDirectionChange = millis();
    stepper.setSpeed(stepperSpeed);
  }
  lastDirection = direction;
}


void runStepper(bool stepperStop)
{
  if(stepperStop == false)
  {
    stepper.runSpeed();  // Execute movement
  }
  else
  {
    stepper.stop();
  }
}

// Stepper instructions depending on sound direction
void stepperMoveDirection()
{
  int direction = soundDirectionCalc(12, 1.035);  // deadZone, factor

  switch (direction)
  {
    case 1:   // F
      if (switchFlag1 == true)
      {
        stepperDirectionChange(direction, 0, false); // stepperSpeed, stepperWait

        // Servo positioning for front
        if ((millis() - lastServoTime > servoDelay)  // Check for servoDelay
        && ((switchFlag3 == false) || (ldrValue >= ldrThreshold))) // Only trigger if dancing is off
        {
          lastServoTime = millis();        
          armServoRight.write(90);  // Move to 90°
          armServoLeft.write(90);  // Move to 90°
        }
      }
      // Call screenOutput function
      screenOutput (direction);
      ledRed = true;
      ledGreen = false;
      ledBlue = false;
      ledExtra = true;
      break;
  
    case 2:   // R
      if (switchFlag1 == true)
      {
          stepperDirectionChange(direction, -500, false);  // direction, stepperSpeed, stepperWait

        // Servo positioning for right
        if (millis() - lastServoTime > servoDelay)
        {
          lastServoTime = millis();        
          armServoRight.write(180);  // Move to 180°
          armServoLeft.write(180);  // Move to 180°
        }
      }
      // Call screenOutput function
      screenOutput (direction);
      ledRed = false;
      ledGreen = false;
      ledBlue = true;
      ledExtra = false;
      break;

    case 3:   // L
      if (switchFlag1 == true)
      {
        stepperDirectionChange(direction, 500, false); // stepperSpeed, stepperWait

        // Servo positioning for left
        if (millis() - lastServoTime > servoDelay)
        {
          lastServoTime = millis();        
          armServoRight.write(0);  // Move to 0°
          armServoLeft.write(0);  // Move to 0°
        }
      }
      // Call screenOutput function
      screenOutput (direction);
      ledRed = false;
      ledGreen = true;
      ledBlue = false;
      ledExtra = false;
      break;

    case 0:
      stepperDirectionChange(direction, 0, true); // stepperSpeed, stepperWait = stop after no sound/direction for timer
      break;
  }
}


#endif