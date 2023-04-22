//This example implements a simple sliding On/Off button. The example
// demonstrates drawing and touch operations.
//
//Thanks to Adafruit forums member Asteroid for the original sketch!
//
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>

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

void setup(void)
{
  Serial.begin(115200);
  delay(10);
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
  greenBtn();
  tft.setCursor(0, 50);
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(2);
  tft.setCursor(150, 50);
  tft.println("SkiCAD v1.0");
  tft.setCursor(200, 75);
  tft.setTextSize(1);
  tft.println("2023");
}

void loop()
{
  // See if there's any  touch data for us
  if (!ts.bufferEmpty())
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
        greenBtn();
      }
    }

    Serial.println("TouchScreen Pressed");
  }  
}



