/*
     FabiWare - AsTeRICS Foundation
     For more info please visit: https://www.asterics-foundation.org

     Module: display.cpp - implementation of the Oled display functions 
     The utilized library is https://github.com/greiman/SSD1306Ascii

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; See the GNU General Public License:
   http://www.gnu.org/licenses/gpl-3.0.en.html

*/

#include <Arduino.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include "display.h"

#define SCREEN_ADDRESS 0x3C

#define SCREEN_INT (1<<0)
#define SCREEN_EXT (1<<1)

uint8_t displayAvailable=0;   // indicates if LCD was found on I2C bus
uint8_t countDisplay = 0;     // how many display instances are used in oleds[]
uint8_t displayPaused=0;      // if true, display updates are paused

SSD1306AsciiWire *oleds[2];    // pointers to the display driver class (can be multiple displays)

/**
   @name initDisplay
   @param uint8_t useWire1:  true if Wire1 should be used, else use Wire
   @return uint_t: returns true if display was found, else false

   initialises the i2c-communication and display module, prints out module name
*/
uint8_t initDisplay () {

  Wire.setSDA(PIN_WIRE0_SDA_);
  Wire.setSCL(PIN_WIRE0_SCL_);
  Wire.begin();

  displayAvailable=0;
  countDisplay = 0;

  // check if LCD is found on Wire0 (internal)
  Wire.beginTransmission(SCREEN_ADDRESS);  
  if (!Wire.endTransmission()) {
    displayAvailable |= SCREEN_INT;
    oleds[countDisplay] = new SSD1306AsciiWire(Wire);
    countDisplay++;
  }

  // check if OLED is found on Wire1 (EXT connector)
  Wire1.beginTransmission(SCREEN_ADDRESS);  
  if (!Wire1.endTransmission()) {
    displayAvailable |= SCREEN_EXT;
    oleds[countDisplay] = new SSD1306AsciiWire(Wire1);
    countDisplay++;
  }

  //init all displays
  for(int i = 0; i<countDisplay; i++) {
    oleds[i]->begin(&Adafruit128x32, SCREEN_ADDRESS);
    oleds[i]->setFont(Adafruit5x7_mod);
  }

  return displayAvailable;
}

/**
   @name isDisplayAvailable
   @brief returns true if LC Display is available at interface Wire0 or Wire1
   @return true if LCD was initialized, false if not
*/
uint8_t isDisplayAvailable() {
  return (displayAvailable);
}


/**
   @name displayClear
   @param none
   @return none

   clears the display and sets correct rotation and display mode
*/
void displayClear(void) {
  for(int i = 0; i<countDisplay; i++) {
    oleds[i]->clear();
    oleds[i]->displayRemap(false);
    oleds[i]->setInvertMode(false);
  }
}


/**
   @name displayMessage
   @param char * msg: string which is printed
   @return none

   clears the display and prints message (center line, double sized)
*/
void displayMessage(char * msg) {
  if (!displayAvailable) return;
  displayClear();
  for(int i = 0; i<countDisplay; i++) {
    oleds[i]->set2X();
    oleds[i]->setCursor(5,1);
    oleds[i]->print(msg);  
  }

}

/*
void displayUpdate(void) {
  if ((!displayAvailable) || (displayPaused)) return;
  displayClear();
  for(int i = 0; i<countDisplay; i++) {
    oleds[i]->set2X();
    oleds[i]->setCursor(5,1);
    oleds[i]->print(slotSettings.slotName);  
    oleds[i]->set1X();

    // display modes for FLipPad
    #ifdef FLIPPAD
      oled->setCursor(100,0);
      switch (slotSettings.stickMode) {
        case 0:
        case 1: oleds[i]->print("Stk"); break;
        case 2:
        case 3:
        case 4: oleds[i]->print("Joy"); break;
      }
    #endif  

    oleds[i]->setCursor(100,3);
    switch (slotSettings.bt) {
      case 1: oleds[i]->print("USB"); break;
      case 2: oleds[i]->print("BT"); break;
      case 3: oleds[i]->print("both"); break;
    }
  }
}

*/