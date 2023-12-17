"""Small example OSC client

Sends test values
"""
import argparse
import random
import time

from pythonosc import udp_client


if __name__ == "__main__":
  parser = argparse.ArgumentParser()
  parser.add_argument("--ip", default="127.0.0.1",
      help="The ip of the OSC server")
  parser.add_argument("--port", type=int, default=9001,
      help="The port the OSC server is listening on")
  args = parser.parse_args()

  client = udp_client.SimpleUDPClient(args.ip, args.port)


  #Test 1: Linearly Increasing/Decreasing Frequency
  for x in range(20):
    octave = [2, 2]
    values = [x/20.0, 0.5, 0.2, 1.0 - x/20.0, 0.5, 0.2]
    client.send_message("/juce/gestureData", octave + values)
    print(octave + values)
    time.sleep(.5)

  # Test 2: Random Amplitude and Timbre
  for _ in range(10):
    octave = [2, 2]
    values = [random.random(), random.random(), random.random(), random.random(), random.random(), random.random()]
    client.send_message("/juce/gestureData", octave + values)
    print(octave + values)
    time.sleep(.5)

  # Test 3: Fixed Frequency, Varying Amplitude and Timbre
  for x in range(20):
    octave = [2, 2]
    values = [0.5, x/20.0, 1.0 - x/20.0, 0.5, 1.0 - x/20.0, x/20.0]
    client.send_message("/juce/gestureData", octave + values)
    print(octave + values)
    time.sleep(.75)
  
  # Test 4: Octave test
  for x in range(10):
    octave = [random.randint(1,4), random.randint(1,4)]
    values = [x/10.0, 0.5, 0.2, 1.0 - x/10.0, 0.5, 0.2]
    client.send_message("/juce/gestureData", octave + values)
    print(octave + values)
    time.sleep(1)