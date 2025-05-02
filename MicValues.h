//////////////////////
///Mic values/////////
//////////////////////

#ifndef MICVALUES_H
#define MICVALUES_H

struct MicValues
{
  int value = 0;
  int diff = 0;

  static const int BUFFERSIZE = 5; // Number of highs/lows to keep track of
  int highValues[BUFFERSIZE] = {0};  // Array to store the BUFFERSIZE highest highs
  int lowValues[BUFFERSIZE] = {1023}; // Array to store the BUFFERSIZE lowest lows (initialized to max)
  int index = 0;

  // Method to update the current value from a mic pin
  void updateValue(int micPin)
  {
    value = analogRead(micPin);
  }

  // Method to update high and low values based on the current value
  void updateHighLow()
  {
    // Insert the new value into arrays
    insertHigh(value);
    insertLow(value);
    
    // Recalculate the diff
    updateDiff();
  }


  // Method to update the difference between the average high and the average low
  void updateDiff()
  {
    uint16_t totalDiff = 0;

    for (int i = 0; i < BUFFERSIZE; i++) // Iterates through all index values in BUFFERSIZE
    {
      totalDiff += (highValues[i] - lowValues[i]);  // Calculates the difference between values and sums it up
    }

    diff = totalDiff / BUFFERSIZE;  // Divides total value by BUFFERSIZE
  }


  // Method to insert the value into the highValues array if it's among the [BUFFERSIZE] highest
  void insertHigh(int newValue)
  {
    for (int i = 0; i < BUFFERSIZE; i++)  // Iterates through all index values in BUFFERSIZE
    {
      if (newValue > highValues[i]) // If new value is higher than any value in highValues array
      {
        for (int j = BUFFERSIZE - 1; j > i; j--)  // Shift lower values to make room for new high value
        {
          highValues[j] = highValues[j - 1];
        }
        highValues[i] = newValue;
        break;
      }
    }
  }


  // Method to insert the value into the lowValues array if it's among the [BUFFERSIZE] lowest
  void insertLow(int newValue)
  {
    for (int i = 0; i < BUFFERSIZE; i++)  // Iterates through all index values in BUFFERSIZE
    {
      if (newValue < lowValues[i])  // If the new value is lower than any value in lowValues array
      {
        for (int j = BUFFERSIZE - 1; j > i; j--) // Shift higher values to make room for new low value
        {
          lowValues[j] = lowValues[j - 1];
        }
        lowValues[i] = newValue;
        break;
      }
    }
  }


  // Method to reset high and low values to current value
  void resetHighLow()
  {
    for (int i = 0; i < BUFFERSIZE; i++)  // Iterates through all index values in BUFFERSIZE
    {
      highValues[i] = value;
      lowValues[i] = value;
    }
  }
};

#endif