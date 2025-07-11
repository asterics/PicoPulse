#define NEOPIXEL_PIN   10
#define LDO_ENABLE_PIN   7   // Enable pin for the MIC5504 3,3V regulator (VCC supply for sensors)


void enable3V3();
void initPixel();
void setPixel (int r,int g, int b);
void fadePixel ();

