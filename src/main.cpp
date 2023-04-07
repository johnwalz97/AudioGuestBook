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
#define PLAYBACK_BUTTON_PIN 1

AudioInputI2S i2sInput;
AudioRecordQueue queue1;

AudioConnection patchCord1(i2sInput, 0, queue1, 0);

AudioSynthWaveform waveform1;
AudioPlaySdRaw playRaw1;
AudioPlaySdWav playWav1;
AudioMixer4 mixer1;
AudioOutputI2S i2sOutput;

AudioConnection patchCord2(waveform1, 0, mixer1, 3);
AudioConnection patchCord3(playRaw1, 0, mixer1, 2);
AudioConnection patchCord4(playWav1, 0, mixer1, 1);
AudioConnection patchCord5(mixer1, 0, i2sOutput, 0);

AudioControlSGTL5000 sgtl5000_1;

char filename[15];
File frec;

Bounce buttonRecord = Bounce(HOOK_PIN, 40);
Bounce buttonPlay = Bounce(PLAYBACK_BUTTON_PIN, 8);

enum Mode
{
  Initialising,
  Ready,
  Prompting,
  Recording,
  Playing
};
Mode mode = Mode::Initialising;

// ---------------------------- Utility functions ----------------------------

void wait(unsigned int milliseconds)
{
  elapsedMillis msec = 0;

  while (msec <= milliseconds)
  {
    buttonRecord.update();
    buttonPlay.update();
    if (buttonRecord.fell())
      Serial.println("Button (pin 0) Press");
    if (buttonPlay.fell())
      Serial.println("Button (pin 1) Press");
    if (buttonRecord.rose())
      Serial.println("Button (pin 0) Release");
    if (buttonPlay.rose())
      Serial.println("Button (pin 1) Release");
  }
}

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

void dateTime(uint16_t *date, uint16_t *time, uint8_t *ms10)
{
  *date = FS_DATE(year(), month(), day());
  *time = FS_TIME(hour(), minute(), second());
  *ms10 = second() & 1 ? 100 : 0;
}

// ---------------------------- Setup functions ----------------------------

void setup()
{
  Serial.println(__FILE__ __DATE__);

  pinMode(HOOK_PIN, INPUT_PULLUP);
  pinMode(PLAYBACK_BUTTON_PIN, INPUT_PULLUP);

  AudioMemory(60);

  // Enable the audio shield, select input, and enable output
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  sgtl5000_1.volume(.75);
  sgtl5000_1.micGain(20);

  // // Play a beep to indicate system is online
  waveform1.begin(WAVEFORM_SINE);
  waveform1.frequency(440);
  waveform1.amplitude(0.9);
  wait(250);
  waveform1.amplitude(0);
  delay(1000);

  // Initialize the SD card
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

  setSyncProvider(getTeensy3Time);
  FsDateTime::setCallback(dateTime);

  mode = Mode::Ready;
}

// ---------------------------- Main program loop ----------------------------

bool startRecording()
{
  for (uint8_t i = 0; i < 9999; i++)
  {
    snprintf(filename, 11, " %05d.RAW", i);
    if (!SD.exists(filename))
      break;
  }

  frec = SD.open(filename, FILE_WRITE);
  if (frec)
  {
    Serial.print("Recording to ");
    Serial.println(filename);
    queue1.begin();
    return true;
  }

  Serial.println("Unable to open file for recording");
  return false;
}

void continueRecording() {
  if (queue1.available() >= 2) {
    byte buffer[512];
    memcpy(buffer, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    memcpy(buffer+256, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    frec.write(buffer, 512);
  }
}

void stopRecording()
{
  queue1.end();
  while (queue1.available() > 0)
  {
    frec.write((byte *)queue1.readBuffer(), 256);
    queue1.freeBuffer();
  }
  frec.close();
}

void playAllRecordings()
{
  File dir = SD.open("/");

  while (true)
  {
    File entry = dir.openNextFile();
    if (!entry)
    {
      Serial.println("No more files");
      entry.close();
      break;
    }

    int8_t len = strlen(entry.name());
    if (strstr(entry.name() + (len - 4), ".RAW"))
    {
      Serial.print("Now playing ");
      Serial.println(entry.name());

      waveform1.amplitude(0.5);
      wait(250);
      waveform1.amplitude(0);

      playRaw1.play(entry.name());
    }
    entry.close();

    while (playRaw1.isPlaying())
    {
      buttonRecord.update();
      buttonPlay.update();
      if (buttonPlay.rose() || buttonRecord.fell())
      {
        playRaw1.stop();
        break;
      }
    }
  }

  mode = Mode::Ready;
}

void loop()
{
  buttonRecord.update();
  buttonPlay.update();

  switch (mode)
  {
  case Mode::Ready:
    if (buttonRecord.fell())
    {
      Serial.println("Handset lifted");
      mode = Mode::Prompting;
    }
    else if (buttonPlay.rose())
    {
      Serial.println("Button (playButton) Press");
      mode = Mode::Playing;
      playAllRecordings();
    }
    break;

  case Mode::Prompting:
    wait(1000);
    playWav1.play("/greeting.wav");
    delay(25);
    while (playWav1.isPlaying())
    {
      buttonRecord.update();
      if (buttonRecord.rose())
      {
        playWav1.stop();
        mode = Mode::Ready;
        return;
      }
    }
    wait(250);
    Serial.println("Starting Recording");

    // Play the tone sound effect
    waveform1.frequency(440);
    waveform1.amplitude(0.9);
    wait(250);
    waveform1.amplitude(0);

    mode = startRecording() ? Mode::Recording : Mode::Ready;

    break;

  case Mode::Recording:
    if (buttonRecord.rose())
    {
      Serial.println("Stopping Recording");
      stopRecording();

      // Play audio tone to confirm recording has ended
      waveform1.frequency(523.25);
      waveform1.amplitude(0.9);
      wait(50);
      waveform1.amplitude(0);
      wait(50);
      waveform1.amplitude(0.9);
      wait(50);
      waveform1.amplitude(0);

      mode = Mode::Ready;
    }
    else
      continueRecording();
    break;

  case Mode::Playing:
  case Mode::Initialising:
    break;
  }
}
