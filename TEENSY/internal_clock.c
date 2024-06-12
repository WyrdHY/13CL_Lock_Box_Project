#include <Arduino.h>

// Parameters
const int freq = 10;                          // Frequency of sine wave
const int sampleRate = 360;                   // Number of samples per second
IntervalTimer myTimer;                        // Timer object
const int dacPin = A21;                       // DAC pin on Teensy 4.1
volatile int index = 0;                       // Index for sine wave calculation
const int maxIndex = sampleRate / freq;       // Max index count
float sineWave[maxIndex];                     // Array to hold sine wave values

void setup() {
  analogWriteResolution(12);                  // 12-bit resolution for the DAC
  pinMode(dacPin, OUTPUT);                    // Set DAC pin as an output

  // Pre-calculate sine wave values
  for (int i = 0; i < maxIndex; ++i) {
    sineWave[i] = sin((2 * PI * i) / maxIndex) * 1650 + 1650; // 0-3.3V range calculation
  }

  // Start timer, set to update DAC at `sampleRate` samples per second
  myTimer.begin(updateSineWave, 1000000 / sampleRate);
}

void loop() {
  // Other tasks can be performed here
  delay(100);  // Example delay
}

// Timer interrupt callback function
void updateSineWave() {
  analogWrite(dacPin, (int)sineWave[index]);  // Write next sine wave value to DAC
  index = (index + 1) % maxIndex;             // Increment index and wrap around
}
