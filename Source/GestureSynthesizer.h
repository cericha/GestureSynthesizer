/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2022 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:            Gesture Synthesizer
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Simple audio application.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        GestureSynthesizer

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
class GestureSynthesizer : public AudioAppComponent, private juce::OSCReceiver, private juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::MessageLoopCallback>
{
public:
    //==============================================================================
    GestureSynthesizer()
       #ifdef JUCE_DEMO_RUNNER
        : AudioAppComponent (getSharedAudioDeviceManager (0, 2))
       #endif
    {
        setAudioChannels (0, 2);
        
        // Define frequency Knob
        frequencyKnob.setRange (0.0, 1.0);
        frequencyKnob.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        frequencyKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 150, 25);
        frequencyKnob.setBounds (10, 50, 180, 180);
        frequencyKnob.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (frequencyKnob);
        addAndMakeVisible(frequencyLabel);
        frequencyLabel.setJustificationType (juce::Justification::centred);
        frequencyLabel.setText ("Frequency", juce::dontSendNotification);
        frequencyLabel.attachToComponent (&frequencyKnob, false);

        // Define Amplitude Knob
        amplitudeKnob.setRange (0.0, 1.0);
        amplitudeKnob.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        amplitudeKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 150, 25);
        amplitudeKnob.setBounds (190, 50, 180, 180);
        amplitudeKnob.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (amplitudeKnob);
        addAndMakeVisible(amplitudeLabel);
        amplitudeLabel.setJustificationType (juce::Justification::centred);
        amplitudeLabel.setText ("Amplitude", juce::dontSendNotification);
        amplitudeLabel.attachToComponent (&amplitudeKnob, false);
        
        // Define Timbre Knob
        timbreKnob.setRange (0.0, 1.0);
        timbreKnob.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        timbreKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 150, 25);
        timbreKnob.setBounds (380, 50, 180, 180);
        timbreKnob.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (timbreKnob);
        addAndMakeVisible(timbreLabel);
        timbreLabel.setJustificationType (juce::Justification::centred);
        timbreLabel.setText ("Timbre", juce::dontSendNotification);
        timbreLabel.attachToComponent (&timbreKnob, false);

        // Specify UDP port for OSC messages
        if (! connect (9001))
            showConnectionErrorMessage ("Error: could not connect to UDP port 9001.");
 
        // Route to listen for OSC messages
        addListener (this, "/juce/gestureData");
        
        // Window Size
        setSize (600, 250);
    }

    ~GestureSynthesizer() override
    {
        shutdownAudio();
    }

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double newSampleRate) override
    {
        sampleRate = newSampleRate;
        expectedSamplesPerBlock = samplesPerBlockExpected;
    }

    /*  This method generates the actual audio samples.
        In this example the buffer is filled with a sine wave whose frequency and
        amplitude are controlled by the mouse position.
     */
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();
        auto originalPhase = phase;
        float curr_amplitude = (float) amplitudeKnob.getValue() / 10.0;
        float magnitude = (float) timbreKnob.getValue() / 10.0;
        
        for (auto chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
        {
            phase = originalPhase;
            auto* channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
            
            
            for (auto i = 0; i < bufferToFill.numSamples; ++i)
            {
                // Define synthesizer
                channelData[i] = (curr_amplitude) * (((1.0 - magnitude) * std::sin(phase)) + ((std::sin(phase) >= 0) ? magnitude : -magnitude));
                
                phase = std::fmod(phase + phaseDelta, MathConstants<float>::twoPi);
            }
        }
    }

    void releaseResources() override
    {
        // This gets automatically called when audio device parameters change
        // or device is restarted.
    }


    //==============================================================================
    void paint (Graphics& g) override
    {
//        // (Our component is opaque, so we must completely fill the background with a solid colour)
//        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
//
//        auto centreY = (float) getHeight() / 2.0f;
//        auto radius = amplitude * 200.0f;
//
//        if (radius >= 0.0f)
//        {
//            // Draw an ellipse based on the mouse position and audio volume
//            g.setColour (Colours::lightgreen);
//
//            g.fillEllipse (jmax (0.0f, lastMousePosition.x) - radius / 2.0f,
//                           jmax (0.0f, lastMousePosition.y) - radius / 2.0f,
//                           radius, radius);
//        }
//
//        // Draw a representative sine wave.
//        Path wavePath;
//        wavePath.startNewSubPath (0, centreY);
//
//        for (auto x = 1.0f; x < (float) getWidth(); ++x)
//            wavePath.lineTo (x, centreY + amplitude * (float) getHeight() * 2.0f
//                                            * std::sin (x * frequency * 0.0001f));
//
//        g.setColour (getLookAndFeel().findColour (Slider::thumbColourId));
//        g.strokePath (wavePath, PathStrokeType (2.0f));
    }

    void mouseDown (const MouseEvent& e) override
    {
    }

    void mouseDrag (const MouseEvent& e) override
    {
    }

    void mouseUp (const MouseEvent&) override
    {
    }

    void resized() override
    {
        // This is called when the component is resized.
        // If you add any child components, this is where you should
        // update their positions.
    }


private:
    //==============================================================================
    float phase       = 0.0f;
    float phaseDelta  = 0.0f;
    float frequency   = 5000.0f;
    float amplitude   = 0.2f;
    float octave_multiplier = 4.0f;
//    float octave_multiplier_min = 1.0f;
//    float octave_multiplier_max = 8.0f;

    double sampleRate = 0.0;
    int expectedSamplesPerBlock = 0;
    Point<float> lastMousePosition;

    void oscMessageReceived (const juce::OSCMessage& message) override
    {
        if (message.size() == 3 && message[0].isFloat32() && message[1].isFloat32() && message[2].isFloat32()) {
            frequencyKnob.setValue (juce::jlimit (0.0f, 10.0f, message[0].getFloat32()));
            amplitudeKnob.setValue (juce::jlimit (0.0f, 10.0f, message[1].getFloat32()));
            timbreKnob.setValue (juce::jlimit (0.0f, 10.0f, message[2].getFloat32()));
            int regions = 8;
            double region_area = 1.0f / regions;
            // Define A Major scale
            float frequencies[]{440.00, 493.88, 554.37, 587.33, 659.25, 739.99, 830.61, 880.00};
            int frequency_index = std::floor(frequencyKnob.getValue()/region_area);
            frequency = (octave_multiplier / 4.0) * frequencies[7 - frequency_index];
            phaseDelta = (float) (MathConstants<double>::twoPi * frequency / sampleRate);
        }
    }

    void showConnectionErrorMessage (const juce::String& messageText)
    {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                                "Connection error",
                                                messageText,
                                                "OK");
    }
    juce::Slider frequencyKnob;
    juce::Slider amplitudeKnob;
    juce::Slider timbreKnob;
    juce::Label frequencyLabel;
    juce::Label amplitudeLabel;
    juce::Label timbreLabel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GestureSynthesizer)
};
