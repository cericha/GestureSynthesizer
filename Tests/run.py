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

    # Test 1: Linearly Increasing/Decreasing Frequency
  for x in range(20):
    values = [x/20, 0.5, 0.2, 1.0 - x/20, 0.5, 0.2]
    client.send_message("/juce/gestureData", values)
    print(values)
    time.sleep(1)

  # Test 2: Random Amplitude and Timbre
  for _ in range(10):
    values = [random.random(), random.random(), random.random(), random.random(), random.random(), random.random()]
    client.send_message("/juce/gestureData", values)
    print(values)
    time.sleep(.5)

  # Test 3: Fixed Frequency, Varying Amplitude and Timbre
  for x in range(20):
    values = [0.5, x/20, 1.0 - x/20, 0.5, 1.0 - x/20, x/20]
    client.send_message("/juce/gestureData", values)
    print(values)
    time.sleep(.5)