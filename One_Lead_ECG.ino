/***********************************************************************************************************
  One_Lead_ECG.ino
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

  //=========================================================================================================

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

      Linke: https://learn.sparkfun.com/tutorials/openlog-hookup-guide/all

  //=========================================================================================================

  Some useful links for setup and library specifications are listed below:
   DIY ECG EKG Portable Heart Monitor: https://www.instructables.com/id/DIY-ECG-EKG-Portable-Heart-Monitor/
   
   Heart Rate Monitor Setup: https://learn.sparkfun.com/tutorials/ad8232-heart-rate-monitor-hookup-guide/all#uploading-the-sketch-and-connecting-with-processing
   Real Time Clock Setup: http://misclab.umeoce.maine.edu/boss/Arduino/bensguides/DS3231_Arduino_Clock_Instructions.pdf
   OpenLog Setup: https://learn.sparkfun.com/tutorials/openlog-hookup-guide/all

   Adafruit GFX Library (graphics): https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives
   Button Hardware: https://www.arduino.cc/en/tutorial/button
   Interrupt Routine: https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/

  //=========================================================================================================
  
  Setpup and instructions (NOT 100% IMPLEMETED YET):
   *The one-lead ECG monitor is meant to be easy to use with little instructions.
   1) Turn Arduino on using button switch.
   2) Wait for screen to become blank (initialize)
   3) After screen prompts user to begin, user can place two fingers from both hands on two separate touch sensors for 30 seconds (data will save)
   4) Turn Arduino off using button switch. 
 ***********************************************************************************************************/

#include <SPI.h>
#include <DS3231.h>
#include <Adafruit_GFX.h> 
#include <Adafruit_ST7735.h>

//=============================================
//DEFINE PINS - BEGIN 
#define __SCK   13  // Clock on LCD
#define __MOSI  11  // MOSI on LCD
#define __CS    10  // Chip Select on LCD
#define __RST   9   // Reset on LCD
#define __DC    8   // Data/Command on LCD (

#define __LON   6   // LO- on Heart Rate Monitor
#define __LOP   5   // LO+ on Heart Rate Monitor

#define BUTTON  2   // Button to start and stop recording data

#define __TX    1
#define __RX    0

#define __ECG   A0
//DEFINE PINS - END
//=============================================

//=============================================
//DEFINE CONSTANTS - BEGIN
#define THRESHOLD 700   // Maximum analog input value (found from trial and error)
#define FLATLINE  50    // Arbitrary flatline pixel height
#define OFFSET    28    // Offset from the top of the screen to begin printing ECG data

#define MAX       28    // Pixel # at top of ECG graph
#define MIN       128   // Pixel # at bottom of ECG graph

#define BLUE      0x001F
#define GREEN     0x07E0
#define CYAN      0x07FF
#define RED       0xF800
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0
#define WHITE     0xFFFF
#define BLACK     0x0000
//DEFINE CONSTANTS - END
//=============================================

//=============================================
//INITIALIZE OBJECTS - BEGIN
DS3231 rtc(SDA, SCL);
Adafruit_ST7735 tft = Adafruit_ST7735(__CS, __DC, __RST);
//INITIALIZE OBJECTS - END
//=============================================

// Set time flag
volatile boolean SetTime = false;

// Set a debugging flag
volatile boolean debug = false;

// Set a debugging counter
int counter = 0;

//The followin flag is set by the interrupt routine
//to force the serial output stream
//to trigger the insertion of a '0' to mark the one-second
//timing 'tick'
//(trying to output this in the interrupt
//handler code, upsets the format of the
//serial numbers, because the '0' insersion
//can take place at any instant - even mid
//number, leading to 5205 (zero in 525)
volatile boolean InsertZero = false;

// SaveData is set true when the button intterupt is triggered
//  Indicates to save data to SD card
volatile boolean SaveData = false;
volatile boolean PressedOnce = false;

// Variables for Button Interrupt
int radius = 4;     

char buf[21]; /* buffer to hold the text of the date - ready to print */
char fileName[50];
char day[4];  /* buffer used to hold the string value of the day. Eg "Tue" */

int xPos = 0;
int increment = 4;

volatile boolean firstDraw = false;

volatile int oldVal_TFT;
volatile int newVal_TFT;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//================================================================================
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void setup(void) {  
  SetTime = false;
  debug = false;
  SaveData = false;
  PressedOnce = false;
  
  Serial.begin(9600);

  pinMode(BUTTON, INPUT);

  // Attach an interrupt to the ISR vector
  // Parameter 1: interrupt number - convention to use digitalPinToInterrupt(pin)
  // Parameter 2: Interrupt Service Routine function
  // Parameter 3: mode (LOW, CHANGE, RISING, FALLING)
  attachInterrupt(digitalPinToInterrupt(BUTTON), SaveInfo_ISR, FALLING);

  /* REAL TIME CLOCK SETUP */
  rtc.begin();
  
  /*The following lines can be uncommented to set the date and time */
  if(SetTime == true){
    rtc.setDOW(MONDAY);         // Set Day-of-Week to MONDAY
    rtc.setTime(05, 23, 00);     // Set the time to 12:00:00 (24hr format)
    rtc.setDate(06, 05, 2019);   // Set the date to 29 April 2019  
  }

  /* LCD SETUP */
  tft.initR(INITR_144GREENTAB);   // initialize a ST7735S chip, black tab
  tft.fillScreen(BLACK);
  tft.setRotation(3);

  /* HEART RATE MONITOR SETUP */
  pinMode(__LOP, INPUT); // Setup for leads off detection LO +
  pinMode(__LON, INPUT); // Setup for leads off detection LO -

  /* INSTRUCTIONS */
  if(debug == false){
    displayInstructionsTFT();  
  }

  print_timeTFT();
  dateTimeOutput();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//================================================================================
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void loop() {
  /* Serial output date/time and analog input */
  if(SaveData == true){
    Serial.println();
    dateTimeOutput();
    Serial.print("\t");
    heartRateOutput(); 
  }  
  
  /* Draw ECG data (analog data) to LCD screen */
  draw();

  /* Ensure enough time is given to draw on LCD screen */
  delay(100); 
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//================================================================================
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void heartRateOutput()
{
  counter += 1;

  /*
  AD8232 sets LO- (LON) and LO+ (LOP) to high (1) if an electrode is not connected properly
  Check to see if all inputs are connected properly
  */
  if((digitalRead(__LOP) == 1)||(digitalRead(__LON) == 1)){
    Serial.print("Bad connection");
  }
  else{
    // print the value of analog input 0:
    Serial.print(counter);
    Serial.print("\t");
    Serial.print(analogRead(A0));
  }
  
  /* Wait for a bit to keep serial data from saturating */
  delay(10);  
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//================================================================================
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SaveInfo_ISR() 
{  
  if(PressedOnce == false){
    SaveData = true;
    AddRecordingCircle();
    PressedOnce = true;
    Serial.println("DoW Day.Month.Year -- Hour:Minute:Second Counter  Data");
  }
  else{
    SaveData = false;
    RemoveRecordingCircle();
    PressedOnce = false;
  }
  delay(200);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//================================================================================
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void AddRecordingCircle()
{
  tft.fillCircle((128 - radius - 1), (128 - radius - 1), radius, RED);  
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//================================================================================
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void RemoveRecordingCircle()
{
  tft.fillCircle((128 - radius - 1), (128 - radius - 1), radius, BLACK);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//================================================================================
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void draw()
{     
  if (InsertZero == true) {
    // put a vertical line on the screen
    // to mark a one second interval
    tft.drawLine(xPos - 1, 0, xPos - 1, 239, RED);
    InsertZero = false;
  }

  xPos += increment;
  if (xPos > 128) {
    xPos = 0;
    print_timeTFT();
  }
    
  /*
  From trial and error, analog input is not higher than 700 or lower than 0
  Due to sensitivity of current sensor, values between 350 and 365 are overrided to the FLATLINE value
  Conversion factor: Max analog input = 700 and Max pixel height = 100
  By divided analog input by 7, the ECG data can fit onto the LCD screen in the given area.
  */
  int temp;
  oldVal_TFT = newVal_TFT;
  newVal_TFT  = -1*(analogRead(__ECG) / 7) + 128; //convert to a coordinate value 0 - 100
  
  if (xPos > 0) {    
    
    if(firstDraw == false){
      oldVal_TFT = OFFSET + FLATLINE;
      newVal_TFT = OFFSET + FLATLINE;
      firstDraw = true;
    }
    
    tft.drawLine(xPos - increment, oldVal_TFT, xPos, newVal_TFT, CYAN);  
  }

  if(debug == true){
    Serial.print("\t");
    Serial.print(newVal_TFT);  
    Serial.print("\t");
    Serial.print(oldVal_TFT);
  }

  if(PressedOnce == true){
    AddRecordingCircle();  
  }
  
  delay(10);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//================================================================================
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void dateTimeOutput()
{
  /* Send Day-of-Week */
  Serial.print(rtc.getDOWStr());
  Serial.print(" ");
  
//  /* Send date */
//  Serial.print(rtc.getDateStr());
//  Serial.print(" -- ");
//
//  /* Send time */
//  Serial.print(rtc.getTimeStr());
  
  /* Wait one second before repeating :) */
  delay (10);  
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//================================================================================
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void displayInstructionsTFT()
{
  /* Instructions: */
  tft.println("Hello welcome to yourportable ECG!");
  tft.println("\nTo being, place your thumbs on the touch  sensors pads below.");
  tft.println("\nTo record data, pressthe button on the    left once...");
  tft.println("and to stop recordingpress the button     again!");
  tft.println("\n:)");

  /* Wait period for user to read instructions fully */
  delay(15000);

  /* Clear screen */
  tft.fillScreen(BLACK);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//================================================================================
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void print_timeTFT()
/* Put the RTC current time onto the LCD or serial window  */
{
  createTimeString();
  tft.fillScreen(BLACK);
  tft.setCursor(0, 0);
  //colours eg BLACK, YELLOW, RED, GREEN, WHITE
  tft.setTextColor(YELLOW);
  //text size 1=tiny, 2 onwards gets bigger but blocky after 3
  //text size of 7 puts about 8 characters across the wide screen
  tft.setTextSize(1);
  tft.println(buf);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//================================================================================
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void createTimeString()
//create a time string and put it in buf, eg "Thu 29-09-2016 17:20"
{
  /* read the time from the RTC */
  Time t = rtc.getTime();
  /*Get a string version of the day */
  switch (t.dow)
  {
    case 1: strcpy(day, "Mon"); break;
    case 2: strcpy(day, "Tue"); break;
    case 3: strcpy(day, "Wed"); break;
    case 4: strcpy(day, "Thu"); break;
    case 5: strcpy(day, "Fri"); break;
    case 6: strcpy(day, "Sat"); break;
    case 7: strcpy(day, "Sun"); break;
  }
  
  /* Format "Thu 29-09-2016 17:20:25" */
  /* Buffer size is 21 to cut off :seconds so entire string will fit across 1.44" screen */
  /* Segmentation Fault occurs when you delete t.sec and respective string section */
  snprintf(buf, sizeof(buf), "%s %02d-%02d-%04d %02d:%02d:%02d", day, t.date, t.mon, t.year, t.hour, t.min, t.sec);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//================================================================================
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void createFileName()
////create a filename eg "append 17122520.txt" and put it in buf (YR MO DY HR eg:8pm Dec 25th 2017)
//// only standard 8.3 filenames are accepted
//{
///* read the time from the RTC */
//  Time t = rtc.getTime();
//  
//  snprintf(fileName,sizeof(fileName), "append %02d%02d%02d%02d.txt", t.mon, t.date, t.hour, t.min);
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//================================================================================
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
