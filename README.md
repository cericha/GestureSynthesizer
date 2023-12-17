# Harmony In Motion: Collaborative Gesture-Based Sounds

Harmony In Motion is a gesture-controlled MIDI synthesizer with spectrogram that transforms hand movements captured by your computer webcam into musical expressions. Unlike traditional MIDI controllers, Harmony In Motion offers an intuitive and immersive musical experience by allowing users to control pitch, volume, and sound brightness through simple hand gestures.

- [Harmony In Motion: Collaborative Gesture-Based Sounds](#harmony-in-motion-collaborative-gesture-based-sounds)
  - [Required Packages](#required-packages)
  - [Launching the Program](#launching-the-program)
    - [Compiling JUCE GestureSynthesizer](#compiling-juce-gesturesynthesizer)
    - [Python Gesture Contol](#python-gesture-contol)
  - [Gestures](#gestures)
  - [Best Practices](#best-practices)
  - [How MediaPipe Works](#how-mediapipe-works)
    - [Overview](#overview)
    - [BlazePalm Detector](#blazepalm-detector)
    - [Hand Landmark Model](#hand-landmark-model)
    - [Architecture](#architecture)
    - [Implementation](#implementation)
  - [Challenges](#challenges)
  - [Known Issues](#known-issues)
  - [Additional Resources](#additional-resources)

## Required Packages

Python, OpenCV, MediaPipe, Python OSC

- the 'hand_landmarker.task' model must be installed in the same directory as 'GestureTracker.py'

## Launching the Program

### Compiling JUCE GestureSynthesizer

The JUCE program must be compiled in Xcode for Mac and Visual Studio for Windows.

1. Download JUCE https://juce.com/get-juce/
2. Place JUCE in the directory of your choosing.
3. Set Juce Path
4. Open Projucer
5. Find "Global Paths" and set Path to JUCE and JUCE Modules to the correct location
6. Open GestureSynthesizer.jucer to open the project in Projucer
7. There is a button on the top right of Projucer that will open the project in your native IDE.
   For reference https://docs.juce.com/master/tutorial_new_projucer_project.html

### Python Gesture Contol

1. Launch the gesture control portion of the program located in the GestureTracker directory with 'python GestureTracker.py' in the terminal.
2. Begin gesturing to play the instrument.

## Gestures

Position your hands in front of the webcam in an evenly, well lit environment.

After launch, the program is in Mode 1, which allows you to change the octave of the instrument by the number of fingers you have up. Making your hand into a fist sets the gesture control into Mode 2, which controls:

- Pitch by moving your hand horizontally.
- Volume by moving your hand vertically.
- Timbre (sound brightness) by changing the distance between your pointer and index fingers.

You can return to Mode 1 at any time by making an open palm. Gestures to switch modes and set octaves need to be held for 2 seconds for the system to register.

## Best Practices

Ensure good lighting conditions for accurate hand detection.
For the best experience, the camera should have an unobstructed view of your hand, and you should have enough room to maneuver your hand(s) across the camera frame. Move towards and away from the camera as needed.

## How MediaPipe Works

### Overview

MediaPipe is an open-source library developed by Google, designed to seamlessly integrate with OpenCV. Leveraging pre-trained models for hand tracking and face detection, it addresses unique challenges posed by hand tracking. Unlike faces, hands lack high-contrast features, presenting difficulties in accurate detection. Additionally, the large scale span, coupled with articulated fingers, leads to a high number of bounding box ratios. This complexity can result in high computation costs, learning difficulties, and overfitting for certain ratios.To overcome these challenges, MediaPipe employs a two-model solution: a palm detector and a hand landmark model.

### BlazePalm Detector

The palm detector functions by processing the entire image and accurately pinpointing the palm within an oriented bounding box surrounding the hand. This is achieved through the implementation of a single-shot detector model (SSD), eliminating the need for multiple passes to determine hand position. Moreover, the utilization of square bounding boxes, with only one ratio, effectively reduces the required number of anchors. To address potential overlapping bounding boxes, a non-maximum suppression technique is employed. In the training phase, focal loss minimization is applied, accommodating the vast number of anchors needed due to the substantial scale variance inherent in palm detection.

### Hand Landmark Model

The Hand landmark model operates within a cropped image of the hand from the palm detector and produces three distinct outputs:

- 21 2.5D coordinates within the detected hand regions
- Hand flag: presence of a hand
- Handedness: specifies whether it is a left or right hand

In contrast to traditional object detection methods that rely on classification, the hand landmark model utilizes a regression approach. This allows it to directly predict the bounding box coordinates, making the process simpler and more efficient compared to having a separate region proposal network. Different versions of the model are available to accommodate various processing capabilities and accuracy requirements.

### Architecture

When provided with image input, the palm detector utilizes this information to predict the locations of keypoints and generate an oriented bounding box encompassing the hand palms. In a live setting, this segment of the system is executed just once, as a bounding box can be computed from the landmark prediction of the previous frame, serving as input for the current frame. Consequently, palm detection is initiated either at the beginning or when the system loses track of the palm. This approach optimizes efficiency by minimizing the need for continuous recalibration during real-time operation.

### Implementation

By leveraging the precise locations of the 21 landmark positions, our system can interpret and register specific gestures, including fists, open palms, and thumbs up. These gestures serve as intuitive controls to manipulate the sound generated by the synthesizer.

The X and Y coordinates of the wrist landmark, relative to the input image size, are harnessed to regulate the amplitude and frequency of the produced sound. Simultaneously, the distance between the thumb tip and the index finger tip is employed to govern the timbre of the sound. This distance is calculated using linear interpolation, ensuring a dynamic and responsive synthesis experience based on the user's hand gestures.

## Challenges

We experimented with two different approaches to gesture recognition, initially exploring a system that required training a model to recognize various finger articulations and hand placements. However, it became apparent that obtaining sufficient data for adequate model training would be challenging. You can view that version here:
https://github.com/anthonyfwill/sound-by-hand-gestures

We opted for an alternative implementation, integrating Google's Mediapipe model for hand landmark detection. This approach provided us with x, y coordinates, which would supply data to our synthesizer.

Addressing technical challenges, we had to establish a connection between our Python-based gesture recognition program and the C++ program responsible for sound generation. To overcome this, we identified an open-source audio library enabling the creation of a local server to transmit data to our synthesizer client. We also encountered issues with compiling programs on different computers due to differing IDE versions.

## Known Issues

We noticed that the system sometimes confuses which hand is controlling which "voice" when two hands cross over or when hands leave the frame. We will implement a more advanced method of passing data from the gesture control system to the synthesizer in future versions to mitigate this problem.

## Additional Resources

Project Presentation: https://docs.google.com/presentation/d/11XkNUgmywfcmgPJH0sGzcOiOLrgdc08dL8EQ5NtDJRU/edit#slide=id.g1f87997393_0_782
