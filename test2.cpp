#include <Arduino.h>

#include <Bounce2.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <TimeLib.h>

#define SDCARD_CS_PIN 10
#define SDCARD_MOSI_PIN 7
#define SDCARD_SCK_PIN 14
#define HOOK_PIN 0

// we want to stream audio from the mic to the audio output

AudioInputI2S i2sInput;
AudioRecordQueue queue1;
AudioOutputI2S i2sOutput;

AudioConnection patchCord1(i2sInput, 0, queue1, 0);
AudioConnection patchCord2(queue1, 0, i2sOutput, 0);

AudioControlSGTL5000 sgtl5000_1;

void setup()
{
  Serial.println(__FILE__ __DATE__);

  pinMode(HOOK_PIN, INPUT_PULLUP);

  AudioMemory(60);

  // Enable the audio shield, select input, and enable output
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  sgtl5000_1.volume(2);
  sgtl5000_1.micGain(15);
}

void loop()
{
  // nothing to do here
}
