#include "utils.h"
#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel pixels(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
int brightness=0;

void initPixel() 
{
  pixels.begin();
  pixels.setBrightness(127);
}

void setPixel (int r,int g, int b) {
  uint32_t ledValue=(uint32_t)r + ((uint32_t)g<<8) + ((uint32_t)b<<16);
  pixels.setBrightness(127);
  brightness=127;
  pixels.setPixelColor(0,r,g,b);
  pixels.show();
}

void enable3V3() {
  gpio_init(LDO_ENABLE_PIN);
  gpio_set_dir(LDO_ENABLE_PIN, true);
  gpio_put(LDO_ENABLE_PIN, true);
}

void fadePixel() {
  static int counter=0;
  if (!brightness) return;
  counter++; if (counter<10) return;
  counter=0;
  brightness-=5; if (brightness < 0) brightness=0;
  pixels.setBrightness(brightness);
  pixels.show();
}