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
        
        // Define frequency Knob Voice 1
        frequencyKnobVoice1.setRange (0.0, 1.0);
        frequencyKnobVoice1.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        frequencyKnobVoice1.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 150, 25);
        frequencyKnobVoice1.setBounds (10, 40, 180, 180);
        frequencyKnobVoice1.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (frequencyKnobVoice1);
        addAndMakeVisible(frequencyLabelVoice1);
        frequencyLabelVoice1.setJustificationType (juce::Justification::centred);
        frequencyLabelVoice1.setText ("Frequency V1", juce::dontSendNotification);
        frequencyLabelVoice1.attachToComponent (&frequencyKnobVoice1, false);

        // Define Amplitude Knob Voice 1
        amplitudeKnobVoice1.setRange (0.0, 1.0);
        amplitudeKnobVoice1.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        amplitudeKnobVoice1.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 150, 25);
        amplitudeKnobVoice1.setBounds (190, 40, 180, 180);
        amplitudeKnobVoice1.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (amplitudeKnobVoice1);
        addAndMakeVisible(amplitudeLabelVoice1);
        amplitudeLabelVoice1.setJustificationType (juce::Justification::centred);
        amplitudeLabelVoice1.setText ("Amplitude V1", juce::dontSendNotification);
        amplitudeLabelVoice1.attachToComponent (&amplitudeKnobVoice1, false);
        
        // Define Timbre Knob Voice 1
        timbreKnobVoice1.setRange (0.0, 1.0);
        timbreKnobVoice1.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        timbreKnobVoice1.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 150, 25);
        timbreKnobVoice1.setBounds (380, 40, 180, 180);
        timbreKnobVoice1.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (timbreKnobVoice1);
        addAndMakeVisible(timbreLabelVoice1);
        timbreLabelVoice1.setJustificationType (juce::Justification::centred);
        timbreLabelVoice1.setText ("Timbre V1", juce::dontSendNotification);
        timbreLabelVoice1.attachToComponent (&timbreKnobVoice1, false);
        
        // Define frequency Knob Voice 2
        frequencyKnobVoice2.setRange (0.0, 1.0);
        frequencyKnobVoice2.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        frequencyKnobVoice2.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 150, 25);
        frequencyKnobVoice2.setBounds (10, 270, 180, 180);
        frequencyKnobVoice2.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (frequencyKnobVoice2);
        addAndMakeVisible(frequencyLabelVoice2);
        frequencyLabelVoice2.setJustificationType (juce::Justification::centred);
        frequencyLabelVoice2.setText ("Frequency V2", juce::dontSendNotification);
        frequencyLabelVoice2.attachToComponent (&frequencyKnobVoice2, false);

        // Define Amplitude Knob Voice 2
        amplitudeKnobVoice2.setRange (0.0, 1.0);
        amplitudeKnobVoice2.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        amplitudeKnobVoice2.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 150, 25);
        amplitudeKnobVoice2.setBounds (190, 270, 180, 180);
        amplitudeKnobVoice2.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (amplitudeKnobVoice2);
        addAndMakeVisible(amplitudeLabelVoice2);
        amplitudeLabelVoice2.setJustificationType (juce::Justification::centred);
        amplitudeLabelVoice2.setText ("Amplitude V2", juce::dontSendNotification);
        amplitudeLabelVoice2.attachToComponent (&amplitudeKnobVoice2, false);
        
        // Define Timbre Knob Voice 2
        timbreKnobVoice2.setRange (0.0, 1.0);
        timbreKnobVoice2.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        timbreKnobVoice2.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 150, 25);
        timbreKnobVoice2.setBounds (380, 270, 180, 180);
        timbreKnobVoice2.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (timbreKnobVoice2);
        addAndMakeVisible(timbreLabelVoice2);
        timbreLabelVoice2.setJustificationType (juce::Justification::centred);
        timbreLabelVoice2.setText ("Timbre V2", juce::dontSendNotification);
        timbreLabelVoice2.attachToComponent (&timbreKnobVoice2, false);

        // Specify UDP port for OSC messages
        if (! connect (9001))
            showConnectionErrorMessage ("Error: could not connect to UDP port 9001.");
 
        // Route to listen for OSC messages
        addListener (this, "/juce/gestureData");
        
        // Window Size
        setSize (600, 485);
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
    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();

        // Voice 1 parameters
        auto originalPhaseVoice1 = phaseVoice1;
        float currAmplitudeVoice1 = (float)amplitudeKnobVoice1.getValue() / 10.0;
        float magnitudeVoice1 = (float)timbreKnobVoice1.getValue() / 10.0;

        // Voice 2 parameters
        auto originalPhaseVoice2 = phaseVoice2;
        float currAmplitudeVoice2 = (float)amplitudeKnobVoice2.getValue() / 10.0;
        float magnitudeVoice2 = (float)timbreKnobVoice2.getValue() / 10.0;

        for (auto chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
        {
            phaseVoice1 = originalPhaseVoice1;
            phaseVoice2 = originalPhaseVoice2;
            auto* channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);

            float phaseOffset = 0.1f; // To avoid possible wave cancellation
            for (auto i = 0; i < bufferToFill.numSamples; ++i)
            {
                // Voice 1 Synthesizer
                auto voice1Sin = std::sin(phaseVoice1 + phaseOffset);
                auto waveVoice1 = (currAmplitudeVoice1) * (((1.0 - magnitudeVoice1) * voice1Sin) + ((voice1Sin >= 0) ? magnitudeVoice1 : -magnitudeVoice1));
                phaseVoice1 = std::fmod(phaseVoice1 + phaseDeltaVoice1, MathConstants<float>::twoPi);

                // Voice 2 Synthesizer
                auto voice2Sin = std::sin(phaseVoice2);
                auto waveVoice2 = (currAmplitudeVoice2) * (((1.0 - magnitudeVoice2) * voice2Sin) + ((voice2Sin >= 0) ? magnitudeVoice2 : -magnitudeVoice2));
                phaseVoice2 = std::fmod(phaseVoice2 + phaseDeltaVoice2, MathConstants<float>::twoPi);

                // Additive synthesis: combine the two voices
                channelData[i] = waveVoice1 + waveVoice2;
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
    float phaseVoice1       = 0.0f;
    float phaseDeltaVoice1  = 0.0f;
    float frequencyVoice1   = 5000.0f;
    float amplitudeVoice1   = 0.2f;
    float octaveMultiplierVoice1 = 4.0f;
    
    float phaseVoice2       = 0.0f;
    float phaseDeltaVoice2  = 0.0f;
    float frequencyVoice2   = 5000.0f;
    float amplitudeVoice2   = 0.2f;
    float octaveMultiplierVoice2 = 4.0f;
//    float octave_multiplier_min = 1.0f;
//    float octave_multiplier_max = 8.0f;

    double sampleRate = 0.0;
    int expectedSamplesPerBlock = 0;
    Point<float> lastMousePosition;

    void oscMessageReceived(const juce::OSCMessage& message) override
    {
        // Expect [frequency, amplitude, timbre] for each voice, so 6 total data points
        if (message.size() == 6 &&
            message[0].isFloat32() && message[1].isFloat32() && message[2].isFloat32() &&
            message[3].isFloat32() && message[4].isFloat32() && message[5].isFloat32())
        {
            // Voice 1 Set Values
            float magnitude1 = 1.0 - message[1].getFloat32();
            frequencyKnobVoice1.setValue(juce::jlimit(0.0f, 10.0f, message[0].getFloat32()));
            amplitudeKnobVoice1.setValue(juce::jlimit(0.0f, 10.0f, magnitude1));
            timbreKnobVoice1.setValue(juce::jlimit(0.0f, 10.0f, message[2].getFloat32()));

            // Voice 2 Set Values
            float magnitude2 = 1.0 - message[4].getFloat32();
            frequencyKnobVoice2.setValue(juce::jlimit(0.0f, 10.0f, message[3].getFloat32()));
            amplitudeKnobVoice2.setValue(juce::jlimit(0.0f, 10.0f, magnitude2));
            timbreKnobVoice2.setValue(juce::jlimit(0.0f, 10.0f, message[5].getFloat32()));

            int regions = 8;
            float frequencies[]{440.00, 493.88, 554.37, 587.33, 659.25, 739.99, 830.61, 880.00};

            // Get frequency index Voice 1 and calculate phase delta
            int frequency_index1 = static_cast<int>(std::round(frequencyKnobVoice1.getValue() * (regions - 1)));
            frequency_index1 = std::max(0, std::min(frequency_index1, regions - 1));

            frequencyVoice1 = (octaveMultiplierVoice1 / 4.0) * frequencies[frequency_index1];
            phaseDeltaVoice1 = (float)(MathConstants<double>::twoPi * frequencyVoice1 / sampleRate);

            // Get frequency index Voice 2 and calculate phase delta
            int frequency_index2 = static_cast<int>(std::round(frequencyKnobVoice2.getValue() * (regions - 1)));
            frequency_index2 = std::max(0, std::min(frequency_index2, regions - 1));
            
            frequencyVoice2 = (octaveMultiplierVoice2 / 4.0) * frequencies[frequency_index2];
            phaseDeltaVoice2 = (float)(MathConstants<double>::twoPi * frequencyVoice2 / sampleRate);
        }
    }

    void showConnectionErrorMessage (const juce::String& messageText)
    {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                                "Connection error",
                                                messageText,
                                                "OK");
    }
    
    // voice 1
    juce::Slider frequencyKnobVoice1;
    juce::Slider amplitudeKnobVoice1;
    juce::Slider timbreKnobVoice1;
    juce::Label frequencyLabelVoice1;
    juce::Label amplitudeLabelVoice1;
    juce::Label timbreLabelVoice1;
    
    // voice2
    juce::Slider frequencyKnobVoice2;
    juce::Slider amplitudeKnobVoice2;
    juce::Slider timbreKnobVoice2;
    juce::Label frequencyLabelVoice2;
    juce::Label amplitudeLabelVoice2;
    juce::Label timbreLabelVoice2;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GestureSynthesizer)
};
