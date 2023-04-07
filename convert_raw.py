# convert the raw audio data from the SD card using pyaudio
import glob
import wave

import pyaudio

# audio is signed 16 bit little endian pcm
# mono, 44100 samples per second
FORMAT = pyaudio.paInt16
CHANNELS = 1
RATE = 44100

audio = pyaudio.PyAudio()

# get all .RAW files in the data/ directory
files = glob.glob('data/*.RAW')

for file in files:
    with open(file, 'rb') as f:
        data = f.read()

    # create a wave file
    wave_file = wave.open(file.replace('.RAW', '.wav'), 'wb')
    wave_file.setnchannels(CHANNELS)
    wave_file.setsampwidth(audio.get_sample_size(FORMAT))
    wave_file.setframerate(RATE)
    wave_file.writeframes(data)
    wave_file.close()

audio.terminate()
