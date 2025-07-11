 // RP Pico Firmware for OneSwitchPulse
 //   uses RP Pico Arduino Core by Earle Philhower (min version 4.5.0)
 //   see: https://github.com/earlephilhower/arduino-pico
 //   important: configure file system in flash memory (1MB for FS under Tools->Flash Size)
 
#include <Joystick.h>
#include <FS.h>
#include <LittleFS.h>

#define SEND_DEBUG_MESSAGES 0

#define NUMBER_OF_BUTTONS 5
int buttonPinMap [NUMBER_OF_BUTTONS] = {0,1,2,3,4};

#define SELECT_BUTTON 0
#define SKIP_BUTTON   1
#define ON_OFF_BUTTON 2
#define SLOWER_BUTTON 3
#define FASTER_BUTTON 4

#define BUZZER_PIN 5

#define PULSING_GAMEPAD_BUTTON 2
#define PULSE_TIME_MIN 200
#define PULSE_TIME_MAX 10000
#define PULSE_TIME_STEP 250
#define PULSE_ON_TIME 200

#define TONE_ON_FREQ 1000
#define CHECK_RESPONSE_COMMAND "pulseControl"
#define RESPONSE "OK\n"

typedef struct {
    uint32_t pulseTime;
    uint32_t firstPulseTime;
    uint8_t  clickActive;
} settings_t;

settings_t settings = {
    .pulseTime = 2000,
    .firstPulseTime = 3000,
    .clickActive = 1
};

#define SETTINGS_FILE "/settings.bin"

int buttonStates[NUMBER_OF_BUTTONS];
int pulseActive = 0;
uint32_t pulseTimestamp=0;
uint32_t pulseOffTimestamp=0;

void writeSettings()
{
  File f = LittleFS.open(SETTINGS_FILE,"w");
  if (!f) {
    Serial.println("could not write settings file");
    return;
  }
  f.write((uint8_t *)&settings, sizeof(settings_t));
  f.close();
}

void readSettings()
{
  if(!LittleFS.exists(SETTINGS_FILE)) {
    Serial.println("could not read settings file");
    return;
  }
  File f = LittleFS.open(SETTINGS_FILE,"r");
  f.readBytes((char *)&settings, sizeof(settings_t));
}

void initStorage()
{
  LittleFS.begin();
  if (!LittleFS.exists(SETTINGS_FILE)) {
    writeSettings();
    if (SEND_DEBUG_MESSAGES) { Serial.println("Settings file initialized!"); }
  }
}

void setActive (int active) {
  pulseActive=active; 
  Serial.print("pulseActive: "); Serial.println(pulseActive); 
  tone(BUZZER_PIN,400+active*600,200);
}

void setClick (uint8_t active) {
  settings.clickActive=active; 
  writeSettings();
}

void setPulseTime(int newTime) {
  if (newTime >= PULSE_TIME_MIN && newTime <= PULSE_TIME_MAX ) {
    settings.pulseTime=newTime; 
  }
  Serial.print("pulseTime: "); 
  Serial.println(settings.pulseTime);
  tone(BUZZER_PIN,1500-settings.pulseTime/7,100);
  writeSettings();
}

void faster() {
  setPulseTime(settings.pulseTime-PULSE_TIME_STEP);
}

void slower() {
  setPulseTime(settings.pulseTime+PULSE_TIME_STEP);
}

void setup() {
  delay (1000);
  Serial.begin(115200);
  Joystick.begin();
  for (uint8_t i = 0; i < NUMBER_OF_BUTTONS; i++) {
     pinMode (buttonPinMap[i], INPUT_PULLUP);
     buttonStates[i]=HIGH;
  }
  pinMode (BUZZER_PIN, OUTPUT);
  tone(BUZZER_PIN, TONE_ON_FREQ, 200);


  initStorage();
  readSettings();
  // Serial.printf("Current settings: pulseTime=%u, firstPulseTime=%u\n", settings.pulseTime,settings.firstPulseTime);

}

void loop() {

  // serial commands
  if (Serial.available()) {
    String cmd=Serial.readStringUntil('\n');
    if (SEND_DEBUG_MESSAGES) { Serial.print("got: "); Serial.println(cmd); }
    if (cmd.equals(CHECK_RESPONSE_COMMAND)) Serial.println(RESPONSE);
    else if (cmd.equals("start")) setActive(1);
    else if (cmd.equals("stop")) setActive(0);
    else if (cmd.equals("toggle")) setActive(!pulseActive);
    else if (cmd.equals("click")) setClick(!settings.clickActive);
    else if (cmd.equals("faster")) faster();
    else if (cmd.equals("slower")) slower();
    else if (cmd.startsWith("time:")) {
      String par = cmd.substring(5);
      par.trim();
      setPulseTime(par.toInt());
    } 
    pulseTimestamp=millis();
  }

  // check user input
  int actState;
  int anyButtonPressed=0;
  for (uint8_t i = 0; i < NUMBER_OF_BUTTONS; i++) {
    if ((actState=digitalRead(buttonPinMap[i])) != buttonStates[i]) {
      buttonStates[i] = actState;
      if (actState==LOW) {
          if (SEND_DEBUG_MESSAGES) { Serial.print("Button "); Serial.print(i+1); Serial.println(" pressed"); }
          Joystick.button(i+1, true);
          switch (i) {
            case FASTER_BUTTON: faster(); break;
            case SLOWER_BUTTON: slower(); break;
            case ON_OFF_BUTTON: setActive(!pulseActive); break;
          }
      } else {
          if (SEND_DEBUG_MESSAGES) { Serial.print("Button "); Serial.print(i+1); Serial.println(" released"); }
          Joystick.button(i+1, false);
      }
    }
    if (!actState) anyButtonPressed=1;
  }
  if (anyButtonPressed) pulseTimestamp=millis(); // reset pulse time if any user activity!


  // perform auto pulse
  if (pulseActive) {
    if (millis()-pulseTimestamp >= settings.pulseTime) {
      pulseTimestamp=millis();
      Joystick.button(PULSING_GAMEPAD_BUTTON, true);
      pulseOffTimestamp=pulseTimestamp+PULSE_ON_TIME;
      if (settings.clickActive) tone(BUZZER_PIN,2000,20);
    }
  }
  if (pulseOffTimestamp >=millis()) {
      pulseOffTimestamp=0;
      Joystick.button(PULSING_GAMEPAD_BUTTON, false);
  }
  
  delay(5); // short delay here for debouncing and limiting max data packet rate
}
