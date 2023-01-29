# play the raw audio data from the SD card using pyaudio
import pyaudio

p = pyaudio.PyAudio()

with open('00012.RAW', 'rb') as f:
    data = f.read()

# audio is signed 16 bit little endian pcm
# 1 channel, 44100 samples per second
stream = p.open(format=pyaudio.paInt16,
                channels=1,
                rate=44100,
                output=True)

stream.write(data)
stream.stop_stream()
stream.close()
