//////////////////////
///8574 functions/////
//////////////////////

#ifndef PCF8574FUNCTIONS_H
#define PCF8574FUNCTIONS_H

#include "GlobalVariables.h"

// Function to read input states from PCF8574
uint8_t readPCF8574() // uint8_t (unsigned integer 8bit type) to read one byte
{
  Wire.requestFrom((uint8_t)PCF8574_ADDRESS, (size_t)1);  // Request 1 byte
  if (Wire.available())
  {
    uint8_t receivedByte = Wire.read(); // Read the byte from I2C

    if (debug == 1)   // Debug stuff, unimportant
    {
      uint8_t inputBits = receivedByte & 0b00001111;
      Serial.print("byte recieved - ");
      Serial.print(inputBits, BIN);
      return (receivedByte);
    }
    else  // If not debugging, return receivedByte
    {
      return receivedByte; 
    }  
  }
  return 0xFF;  // Default if no data is read
}

// Function to write data to PCF8574
void writePCF8574(uint8_t data) // uint8_t (unsigned integer 8bit type) to write one byte
{
    Wire.beginTransmission(PCF8574_ADDRESS);
    Wire.write(data);
    Wire.endTransmission();
}

// Function to detect switch triggers
void readSwitches()
{
  uint8_t inputState = readPCF8574(); // Get value returned from readPCF8574 function
    
  // Extract switch states (P0-P3, default HIGH (bits 1-4 of byte)) // Bit Position (P7 → P0)
  bool switch0 = !(inputState & 0b00000001);  //P0 (bit 1)
  bool switch1 = !(inputState & 0b00000010);  //P1 (bit 2)
  bool switch2 = !(inputState & 0b00000100);  //P2 (bit 3)
  bool switch3 = !(inputState & 0b00001000);  //P3 (bit 4)

  // Toggle flags when switches are pressed
  if (switch0 
  && (millis() - lastPressTime > (debounceDelay)))  // To prevent double triggering
  {
    lastPressTime = millis();        
    switchFlag0 = !switchFlag0;
  }
  if (switch1 
  && (millis() - lastPressTime > debounceDelay)) 
  {
    lastPressTime = millis();        
    switchFlag1 = !switchFlag1;
  }
  if (switch2 
  && (millis() - lastPressTime > debounceDelay)) 
  {
    lastPressTime = millis();        
    switchFlag2 = !switchFlag2;
  }
  if (switch3 
  && (millis() - lastPressTime > debounceDelay)) 
  {
    lastPressTime = millis();        
    switchFlag3 = !switchFlag3;
  }

  if (debug == 1) // Debug output
  {  
    Serial.print("Flags: ");
    Serial.print(switchFlag0);
    Serial.print(switchFlag1);
    Serial.print(switchFlag2);
    Serial.println(switchFlag3);
  }
}

// Function to update LEDs
void writeRGB() // ledExtra is unused
{
  // LED outputs set to P4-P7 (bits 5-8 of byte)
  uint8_t ledState = (ledGreen << 4) | (ledBlue << 5) | (ledRed << 6) | (ledExtra << 7);

  // Keep P0-P3 as inputs by OR'ing with 0b00001111
  writePCF8574(ledState | 0b00001111); //0b00001111 // P7-P0
}

#endif