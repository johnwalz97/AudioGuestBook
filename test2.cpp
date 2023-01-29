#include <Arduino.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#define SDCARD_CS_PIN 10
#define SDCARD_MOSI_PIN 7
#define SDCARD_SCK_PIN 14

AudioPlaySdWav playWav1;
AudioOutputI2S audioOutput;
AudioConnection patchCord1(playWav1, audioOutput);
AudioControlSGTL5000 sgtl5000_1;

void setup()
{
  AudioMemory(8);

  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);

  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN)))
  {
    while (1)
    {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
}

void loop()
{
  // q: what settings should i use for creating the wav file so that the teensy can play it?
  // a: 16 bit, 44.1 kHz, mono
  Serial.println("Playing greeting.wav");
  playWav1.play("greeting.wav");
  while (playWav1.isPlaying()) {}
  delay(1000);
}
