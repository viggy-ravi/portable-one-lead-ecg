# portable-one-lead-ecg
  The One-lead ECG is a portable ECG monitor with touch sensing capacitive sensors for pulse detection

    Group 5: Truc Do, Matthew Nepo, Vignesh Ravindranath, Jacob Scheftel
    Written by: Vignesh Ravindranath
  
  The One-lead ECG is a portable ECG monitor with touch sensing capacitive
  sensors for pulse detection. The device uses:
  
    1.44" LCD TFT 128x128 Pixel Display (ST7735)
    Heart Rate Monitor (AD8232)
    Real Time Clock (Adafruit DS3231)
    OpenLog (DEV-13712)
    Toggle Switch (any will work)
  
  The screen will be divided into two sections, one to display date and time and one to display the ECG graph.
  The screen is 128x128. The first section will be 28x128 and the second section will be 100x128.
    
  Download these libraries for this project:
  
    The LCD requires the Adafruit_ST7735 library which can be downloaded from the Arduino IDE itself:
    1) Go to Sketch -> Include Library -> Manage Libraries
    2) Type in ST7735 and install the library by Adafruit
    Git link: https://github.com/adafruit/Adafruit-ST7735-Library
    
  The LCD also requies a graphics library: Adafruit_GFX. It can be downloaded the same way as the ST7735.
    
    Git link: https://github.com/adafruit/Adafruit-GFX-Library
      
  The Real Time Clock requires it's own library to access dates and times. There are many libraries for this
  but in this project the DS3231 library is used:
  
    1) Download the .zip file from the link below and move the file to the library folder under the Arduino folder
    2) Go to Sketch -> Include Library -> Add ZIP Library
    3) Click on the .zip file
      
    Link: http://www.rinkydinkelectronics.com/library.php?id=73
  
  OpenLog requires firmware and two libraries (SdFat and SerialPort). Download zip from link below and include libraries:
    
    Link: https://learn.sparkfun.com/tutorials/openlog-hookup-guide/all
  
  Some useful links for setup and library specifications are listed below:
  
    DIY ECG EKG Portable Heart Monitor: https://www.instructables.com/id/DIY-ECG-EKG-Portable-Heart-Monitor/
    Heart Rate Monitor Setup: https://learn.sparkfun.com/tutorials/ad8232-heart-rate-monitor-hookup-guide/all#uploading-the-sketch-and-connecting-with-processing
    Real Time Clock Setup: http://misclab.umeoce.maine.edu/boss/Arduino/bensguides/DS3231_Arduino_Clock_Instructions.pdf
    OpenLog Setup: https://learn.sparkfun.com/tutorials/openlog-hookup-guide/all
    Adafruit GFX Library (graphics): https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives
    Button Hardware: https://www.arduino.cc/en/tutorial/button
    Interrupt Routine: https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
   
  Setpup and instructions:

    *The one-lead ECG monitor is meant to be easy to use with little instructions.
    1) Turn Arduino on using button switch.
    2) Wait for screen to become blank (initialize)
    3) After screen prompts user to begin, user can place two fingers from both hands on two separate touch sensors for 30 seconds (data will save)
    4) Turn Arduino off using button switch. 
