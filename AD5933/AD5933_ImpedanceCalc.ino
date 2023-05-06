/*
ad5933-test
    Reads impedance values from the AD5933 over I2C and prints them serially.
*/

#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

/* SPI libraries for communicating with the LCD screen */
#include <Adafruit_GFX.h>       /* Graphics library */
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_ILI9341.h>   /* Library for the Adafruit eval board */
#include <Adafruit_STMPE610.h>

/* Libraries for WiFi functionality and communicating with remote server using HTTP */
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

/* I2C libraries */
#include "AD5933.h"

#define START_FREQ  (80000)
#define FREQ_INCR   (35000)
#define NUM_INCR    (40)
#define REF_RESIST  (10000)

#define USE_SERIAL Serial

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

#define STMPE_CS 16
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);
#define TFT_CS 0
#define TFT_DC 15
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

boolean RecordOn = false;

#define FRAME_X 120
#define FRAME_Y 180
#define FRAME_W 100
#define FRAME_H 50

#define GREENBUTTON_X FRAME_X
#define GREENBUTTON_Y FRAME_Y
#define GREENBUTTON_W FRAME_W
#define GREENBUTTON_H FRAME_H

using namespace std;

ESP8266WiFiMulti WiFiMulti;

/* SSID & Password settings for WiFi connection */
const char* ssid = "TP-Link_D9CF";
const char* password = "41551172";

// const char* ssid = "SkiCad";
// const char* password = "testing123";

String impedance_exp = "";
String frequencies_exp = "";
String phase_exp = "";

double gain[NUM_INCR+1];
int phase[NUM_INCR+1];

void drawFrame()
{
  tft.drawRect(FRAME_X, FRAME_Y, FRAME_W, FRAME_H, ILI9341_BLACK);
}

void greenBtn()
{
  tft.fillRect(GREENBUTTON_X, GREENBUTTON_Y, GREENBUTTON_W, GREENBUTTON_H, ILI9341_GREEN);
  // tft.fillRect(REDBUTTON_X, REDBUTTON_Y, REDBUTTON_W, REDBUTTON_H, ILI9341_GREEN);
  drawFrame();
  tft.setCursor(GREENBUTTON_X + 8 , GREENBUTTON_Y + (GREENBUTTON_H/3.25));
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(3);
  tft.println("START");
  RecordOn = true;
}

void data_sent()
{
   tft.fillScreen(ILI9341_BLACK);
    // origin = left,top landscape (USB left upper)
   tft.setRotation(1); 
   tft.setTextColor(ILI9341_GREEN);
   tft.setTextSize(2);
   tft.setCursor(110, 85);
   tft.println("DATA SENT");
}

int k = 0;
int j = 0;

void setup(void)
{
  
  // Begin I2C
  Wire.begin();

  // Begin serial at 9600 baud for output
  Serial.begin(9600);
  Serial.println("AD5933 Test Started!");
  
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

  Serial.println("FeatherWing TFT Touch Test!");

  tft.begin();
  if (!ts.begin()) { 
    Serial.println("Unable to start touchscreen.");
  } 
  else { 
    Serial.println("Touchscreen started."); 
  }

  tft.fillScreen(ILI9341_BLACK);
  // origin = left,top landscape (USB left upper)
  tft.setRotation(1); 
  //redBtn();
  tft.setCursor(0, 50);
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(2);
  tft.setCursor(108, 65);
  tft.println("SkiCAD v1.0");
  tft.setCursor(158, 85);
  tft.setTextSize(1);
  tft.println("2023");

  delay(1000);

  // Perform initial configuration. Fail if any one of these fail.
  if (!(AD5933::reset() &&
        AD5933::setInternalClock(true) &&
        AD5933::setStartFrequency(START_FREQ) &&
        AD5933::setIncrementFrequency(FREQ_INCR) &&
        AD5933::setNumberIncrements(NUM_INCR) &&
        AD5933::setPGAGain(PGA_GAIN_X1)))
        {
            Serial.println("FAILED in initialization!");
            while (true) ;
        }

  // Perform calibration sweep
  if (AD5933::calibrate(gain, phase, REF_RESIST, NUM_INCR+1))
  {
    Serial.println("Calibrated!");
    tft.setCursor(125, 95);
    tft.println("Initializing");
    delay(1250);
    tft.setCursor(125, 95);
    tft.println("Initializing.");
    delay(1250);
    tft.setCursor(125, 95);
    tft.println("Initializing..");
    delay(1250);
    tft.setCursor(125, 95);
    tft.println("Initializing...");
    delay(1250);
    tft.setCursor(125, 95);
    tft.println("Initializing....");
    delay(1250);
    tft.setCursor(125, 95);
    tft.println("Initializing.....");
  }
  else
    Serial.println("Calibration failed...");


  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(100, 85);
  tft.println("DEVICE READY");
  tft.setTextSize(1);
  tft.setCursor(105, 105);
  tft.println("Press start to measure");
  greenBtn();
}


void loop(void)
{
  // Easy to use method for frequency sweep

  // See if there's any  touch data for us
  if ((!ts.bufferEmpty()) && (k == 0))
  {   
    // Retrieve a point  
    TS_Point p = ts.getPoint(); 
    // Scale using the calibration #'s
    // and rotate coordinate system
    p.x = map(p.x, TS_MINY, TS_MAXY, 0, tft.height());
    p.y = map(p.y, TS_MINX, TS_MAXX, 0, tft.width());
    int y = tft.height() - p.x;
    int x = p.y;

    if ((x > GREENBUTTON_X) && (x < (GREENBUTTON_X + GREENBUTTON_W)))
    {
      if (!((y < GREENBUTTON_Y) && (y >= (GREENBUTTON_Y + GREENBUTTON_H))))
      {
        Serial.println("Green btn hit");
        tft.fillScreen(ILI9341_BLACK);
        k++;
        if(k > 1){
        j = 1;
        }
      }
    }

    Serial.println("TouchScreen Pressed");
  }  

  //frequencySweepEasy();

  // Delay
  // delay(5000);
  
  if((j == 0)&&(k == 1))
  {
    // Complex but more robust method for frequency sweep
    frequencySweepRaw();
    // Delay
    delay(5000);
    // wait for WiFi connection

    if((WiFiMulti.run() == WL_CONNECTED)) {
    HTTPClient http;
    WiFiClient client;
    
    USE_SERIAL.print("[HTTP] begin...\n");
    // configure Firebase api
    http.begin(client, "http://skicad.pythonanywhere.com/sendToFirebase");
    
    USE_SERIAL.print("[HTTP] POST...\n");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Freq",frequencies_exp);
    http.addHeader("Phase",phase_exp);
    http.addHeader("Impedance",impedance_exp);
    
    // start connection and send HTTP headers. replace name and data values with your own.
    int httpCode = http.POST("{\"frequency\":1234,\"data\":21.5}");
    
    if(httpCode > 0)
    {
        // HTTP header has been send and Server response header has been handled
        USE_SERIAL.printf("[HTTP] POST... code: %d\n", httpCode);
        // file found at server
            String payload = http.getString();
            USE_SERIAL.println(payload);
            data_sent();
    }
    else
    {
        USE_SERIAL.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    
    http.end();
   }

   delay(10000);
   j = 1;
  }

}

// Easy way to do a frequency sweep. Does an entire frequency sweep at once and
// stores the data into arrays for processing afterwards. This is easy-to-use,
// but doesn't allow you to process data in real time.
void frequencySweepEasy() {
    // Create arrays to hold the data
    int real[NUM_INCR+1], imag[NUM_INCR+1];

    // Perform the frequency sweep
    if (AD5933::frequencySweep(real, imag, NUM_INCR+1)) {
      // Print the frequency data
      int cfreq = START_FREQ/1000;
      for (int i = 0; i < NUM_INCR+1; i++, cfreq += FREQ_INCR/1000) {
        // Print raw frequency data
        Serial.print(cfreq);
        Serial.print(": R=");
        Serial.print(real[i]);
        Serial.print("/I=");
        Serial.print(imag[i]);
        Serial.print(" θ:");
        Serial.print((float)phase[i]/100);

        // Compute impedance
        double magnitude = sqrt(pow(real[i], 2) + pow(imag[i], 2));
        double impedance = 1/(magnitude*gain[i]);
        Serial.print("  |Z|=");
        Serial.println(impedance);
      }
      Serial.println("Frequency sweep complete!");
    } else {
      Serial.println("Frequency sweep failed...");
    }
}

// Removes the frequencySweep abstraction from above. This saves memory and
// allows for data to be processed in real time. However, it's more complex.
void frequencySweepRaw() {
    // Create variables to hold the impedance data and track frequency
    tft.fillScreen(ILI9341_BLACK);
    tft.setRotation(1); 
    tft.setTextColor(ILI9341_GREEN);
    tft.setTextSize(2);
    tft.setCursor(90, 85);
    tft.println("MEASURING.");
    delay(1250);
    tft.setCursor(90, 85);
    tft.println("MEASURING..");
    delay(1250);

    int real, imag, i = 0, cfreq = START_FREQ/1000;
    // int j = 0;


    // Initialize the frequency sweep
    if (!(AD5933::setPowerMode(POWER_STANDBY) &&          // place in standby
          AD5933::setControlMode(CTRL_INIT_START_FREQ) && // init start freq
          AD5933::setControlMode(CTRL_START_FREQ_SWEEP))) // begin frequency sweep
         {
             Serial.println("Could not initialize frequency sweep...");
         }

    tft.setTextSize(2);
    tft.setCursor(90, 85);
    tft.println("MEASURING...");
    delay(1250);

    frequencies_exp = "";
    impedance_exp = "";
    phase_exp = "";

    // Perform the actual sweep
    while ((AD5933::readStatusRegister() & STATUS_SWEEP_DONE) != STATUS_SWEEP_DONE) {
        // Get the frequency data for this frequency point
        if (!AD5933::getComplexData(&real, &imag)) {
            Serial.println("Could not get raw frequency data...");
        }

        // Print out the frequency data
        Serial.print(cfreq);
        Serial.print(": R=");
        Serial.print(real);
        Serial.print("/I=");
        Serial.print(imag);
        Serial.print(" θ:");
        Serial.print((float)phase[i]/100);

        // Compute impedance
        double magnitude = sqrt(pow(real, 2) + pow(imag, 2));
        double impedance = 1/(magnitude*gain[i]);
        Serial.print("  |Z|=");
        Serial.println(impedance);

        // Increment the frequency
        i++;
        cfreq += FREQ_INCR/1000;
        frequencies_exp += String((int)cfreq);
        frequencies_exp += "|";
        impedance_exp += String(impedance, 4);
        impedance_exp += "|";
        phase_exp += String((float)phase[i]/100, 4);
        phase_exp += "|";

        AD5933::setControlMode(CTRL_INCREMENT_FREQ);
    }

    tft.setTextSize(2);
    tft.setCursor(90, 85);
    tft.println("MEASURING....");
    delay(1250);

    Serial.println("Frequency sweep complete!");

    // Set AD5933 power mode to standby when finished
    if (!AD5933::setPowerMode(POWER_STANDBY))
        Serial.println("Could not set to standby...");
    
    tft.setTextSize(2);
    tft.setCursor(90, 85);
    tft.println("MEASURING.....");
    delay(1250);
}
