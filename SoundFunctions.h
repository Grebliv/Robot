//////////////////////
///Sound functions////
//////////////////////

#ifndef SOUNDFUNCTIONS_H
#define SOUNDFUNCTIONS_H

#include "MicValues.h"
#include "GlobalVariables.h"

// Mic pins
const int micPinR = A0;
const int micPinL = A1;
const int micPinF = A2;

// Filter settings 
const float smoothingAlpha = 0.1;      // Lower values = more smoothing (smoothingAlpha = 0.05–0.1)
const float baselineAlpha = 0.05;       // Lower values = more smoothing (baselineAlpha = 0.001–0.05)
const int minBeatThreshold = 3;         // Minimum energy difference to be considered a beat
const int minBeatInterval = 200;        // Ignore very fast beats (milliseconds)

float smoothedMic = 0;
float baselineMic = 0;
float filteredMic = 0;
float lowPassedMicPrev = 0;

// Timers
unsigned long lastBeatTime = 0;
unsigned long currentBeatTime = 0;
unsigned long beatInterval = 0;


// Update the mic values reading pins
void updateMicValues()
{
  micValueRight.updateValue(micPinR);
  micValueLeft.updateValue(micPinL);
  micValueFront.updateValue(micPinF);
}

// Update high and low values
void getMicValuesHighLow()
{
  micValueRight.updateHighLow();
  micValueLeft.updateHighLow();
  micValueFront.updateHighLow();
}

// Get difference between H/L
void getSoundDiff()
{
  micValueRight.updateDiff();
  micValueLeft.updateDiff();
  micValueFront.updateDiff();
}

// Reset mic H/L values to current values
void resetMicValues()
{
  micValueRight.resetHighLow();
  micValueLeft.resetHighLow();
  micValueFront.resetHighLow();
}

// Get direction from difference between H/L calc
int soundDirectionCalc(int deadZone, float factor)
{
  getSoundDiff(); // Get difference between high values and low values

  // Sound Front
  if (((micValueFront.diff - deadZone) > (factor * micValueRight.diff))
  && ((micValueFront.diff - deadZone) > (factor * micValueLeft.diff))
  && ((micValueRight.diff - micValueLeft.diff) < (factor * deadZone)))  
  {
    return 1; // Sound Front
  } 

  // Sound Right
  else if (((micValueRight.diff - deadZone) > (factor * micValueLeft.diff)) 
  && ((micValueRight.diff - deadZone) > (factor * micValueFront.diff))) 
  {
    return 2; // Sound Right
  }

  // Sound Left
  else if (((micValueLeft.diff - deadZone) > (factor * micValueRight.diff)) 
  && ((micValueLeft.diff - deadZone) > (factor * micValueFront.diff)))  
  {
    return 3; // Sound Left
  } 

  // Sound Back
  else if (((micValueRight.diff - deadZone) > (factor * micValueFront.diff))
  && ((micValueLeft.diff - deadZone) > (factor * micValueFront.diff))
  && ((micValueRight.diff - micValueLeft.diff) < (2 * factor * deadZone)))
  {
    if (micValueRight.diff - micValueLeft.diff)
    {
      return 2; // Sound Right
    }    
    else if (micValueLeft.diff - micValueRight.diff) 
    {
      return 3; // Sound Left
    }
  }
  return 0; // Default case if no condition matches
}

// Storing bpm history
void bpmStore(int newBpm) 
{
  bpmHistory[bpmIndex] = newBpm;

  // Update index
  bpmIndex = (bpmIndex + 1) % bpmSmoothingBuffer; // Cycles through the array

  // Increase count
  if (bpmCount < bpmSmoothingBuffer) {
    bpmCount++;
  }

  // Calculate average using only the valid entries
  int total = 0;
  for (int i = 0; i < bpmCount; i++) {
    total += bpmHistory[i];
  }
  bpm = total / bpmCount;
}

float filter(int rawMicValue) 
{
  int micRead = abs(rawMicValue - 512);  // Fix DC offset

  // High-pass filtering (removes slow drift)
  smoothedMic = (smoothingAlpha * micRead) + ((1.0 - smoothingAlpha) * smoothedMic);
  baselineMic = (baselineAlpha * smoothedMic) + ((1.0 - baselineAlpha) * baselineMic);
  float highPassed = smoothedMic - baselineMic;

  return highPassed;
}

void detectBeat() 
{
  //int rawMic = 0;
  int rawMic = analogRead(micPinF);
  int filteredMic =0;
    
  filteredMic = filter(rawMic);

  if (filteredMic > (minBeatThreshold)) 
  {
    currentBeatTime = millis();
    beatInterval = currentBeatTime - lastBeatTime;

    if (beatInterval > minBeatInterval) 
    {
      int rawBPM = (60000 / beatInterval);
      bpmStore(rawBPM); // BPM smoothing
      
      beatDetected = true;
    }
    lastBeatTime = currentBeatTime;
  }
}

#endif