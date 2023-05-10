# Measuring Impedance 

This program reads impedance values from the AD5933 over I2C and prints them serially. It includes two methods for performing a frequency sweep: a simple method that does an entire frequency sweep at once and stores the data into arrays for processing afterwards, and a more complex method that removes the frequency sweep abstraction from the simple method to save memory and allow for data to be processed in real-time.

# Prerequisites

An AD5933 device
Arduino Uno or similar
A computer with the Arduino IDE and the AD5933 library installed
Getting Started

# Getting Started
1. Connect the AD5933 to the Arduino following the wiring instructions in the AD5933 library documentation.
2. Open the Arduino IDE and load the AD5933-test sketch.
3. Upload the sketch to the Arduino board.
4. Open the Serial Monitor in the Arduino IDE to view the impedance values.
Usage.

# Usage
The program includes two methods for performing a frequency sweep. Uncomment either the frequencySweepEasy() method or the frequencySweepRaw() method to use it.
The frequencySweepEasy() method does an entire frequency sweep at once and stores the data into arrays for processing afterwards. This is easy-to-use, but doesn't allow you to process data in real time.
The frequencySweepRaw() method removes the frequency sweep abstraction from the frequencySweepEasy() method. This saves memory and allows for data to be processed in real time. However, it's more complex.
The program performs an initial configuration and calibration sweep at startup. If any of these fail, the program will enter an infinite loop.
The program includes an interrupt service routine that triggers a frequency sweep every time a falling edge is detected on the interrupt pin. By default, the interrupt pin is set to pin 15. You can change this by modifying the pin_isr variable.
You can change the sweep parameters (start frequency, frequency increment, number of increments, and reference resistance) by modifying the START_FREQ, FREQ_INCR, NUM_INCR, and REF_RESIST constants, respectively.
The program computes the impedance from the real and imaginary components of the complex impedance using the gain and phase values obtained during calibration.
