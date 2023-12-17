// Project: Gesture based audio synthesizer
// Class: Computer Vision
// Professor: Iannos Stamos
// Group Members: Fourcan Abdullah, Anthony F Williams, Jeffrey Hsu, Charles Richards
// Date: December 17th, 2023
//
// Juce based audio synthesizer that receives gesture data through OSC protocol.
// Implements two synthesizers which can vary in frequency, pitch, and timbre
// OSC Data selects frequency, pitch, and timbre. Sound is generated continuously based on these parameters
//
// The following official JUCE tutorials and examples were used for and expanded upon for this implementation
// of a synthesizer:
// https://docs.juce.com/master/tutorial_osc_sender_receiver.html
// https://docs.juce.com/master/tutorial_label.html
// https://docs.juce.com/master/tutorial_slider_values.html
// https://docs.juce.com/master/tutorial_simple_fft.html
// https://docs.juce.com/master/tutorial_sine_synth.html

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
class GestureSynthesizer : public AudioAppComponent, private juce::OSCReceiver, private juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::MessageLoopCallback>, private juce::Timer
{
public:
    //==============================================================================
    
    // Define GestyreSynthesizer Component. Builds UI interface, and initializes all values,
    // and starts listning for OSC messages
    GestureSynthesizer()
        :
         spectrogramImage (juce::Image::RGB, 580, 480, true),
         forwardFFT (fftOrder)
       
    {
        setOpaque (true);
        setAudioChannels (0, 2);
        startTimerHz (60);
        
        // Voice 1 Knobs
        initializeKnob(frequencyKnobVoice1, frequencyLabelVoice1, "Frequency V1", 10, 40);
        initializeKnob(amplitudeKnobVoice1, amplitudeLabelVoice1, "Amplitude V1", 190, 40);
        initializeKnob(timbreKnobVoice1, timbreLabelVoice1, "Timbre V1", 380, 40);
        
        // Voic 2 Knobs
        initializeKnob(frequencyKnobVoice2, frequencyLabelVoice2, "Frequency V2", 10, 270);
        initializeKnob(amplitudeKnobVoice2, amplitudeLabelVoice2, "Amplitude V2", 190, 270);
        initializeKnob(timbreKnobVoice2, timbreLabelVoice2, "Timbre V2", 380, 270);

        // Specify UDP port for OSC messages
        if (! connect (OSCPORT))
            showConnectionErrorMessage("Error: could not connect to UDP port " + juce::String(OSCPORT) + ".");
 
        // Route to listen for OSC messages
        addListener (this, OSCROUTE);
        
        // Window Size
        setSize (580, 480);
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

    /*  Generates a block of audio samples which are placed into a buffer for immediate playback
        Audio value is taken from current knob values, and two synthesizer voices are calculated
        for x discrete values in time if the size of the block is x.
     
        Two voices are calculated, using current knob values for pitch, frequency, and timbre.
        Timbre is a range between 0 and 1 where 0 is a pure sine wave, and 1 is a pure square wave.
        Any value in between is a linear mix of the two.
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

        // Calculate for both audio channels
        for (auto chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
        {
            phaseVoice1 = originalPhaseVoice1;
            phaseVoice2 = originalPhaseVoice2;
            auto* channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);

            float phaseOffset = 0.1f; // To avoid possible wave cancellation add some small offset
            
            // Calculate for each sample in block then place in buffer
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
                pushNextSampleIntoFifo (channelData[i]);
            }
        }
    }

    void releaseResources() override
    {
    }


    //==============================================================================
    
    // Initialize and draw spectrogram and background
    void paint (Graphics& g) override
    {
        g.fillAll (juce::Colours::black);
        g.drawImageWithin(spectrogramImage, 0, 0, getWidth(), getHeight(), RectanglePlacement::fillDestination);
        g.setOpacity (1.0f);
    }

    // Helper function to initialize knobs.
    // Creates knob, sets style to rotary, sizes knob, attaches a label, centeres text, and makes visible
    // Currently set such that knobs are not controllable by mouse, but only by OSC messages
    void initializeKnob(juce::Slider& knob, juce::Label& label, const String& labelText, int x, int y)
    {
        // Define and initialize knob
        knob.setRange(0.0, 1.0);
        knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        knob.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 150, 25);
        knob.setBounds(x, y, 180, 180);
        knob.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(knob);

        // Define and initialize label
        label.setJustificationType(juce::Justification::centred);
        label.setText(labelText, juce::dontSendNotification);
        label.attachToComponent(&knob, false);
        addAndMakeVisible(label);
    }
    

    // Allows for dynamic adjustement of UI if window is resized (currently empty)
    void resized() override
    {
    }

    // Every timestep in timer draw next line of spectrogram
    // Implementation for spectrogram mostly comes from JUCE official tutorial on spectrograms
    void timerCallback() override
    {
        if (nextFFTBlockReady)
        {
            drawNextLineOfSpectrogram();
            nextFFTBlockReady = false;
            repaint();
        }
    }

    // Helper function for spectrogram.
    // Implementation for spectrogram mostly comes from JUCE official tutorial on spectrograms
    void pushNextSampleIntoFifo (float sample) noexcept
    {
        if (fifoIndex == fftSize)
        {
            if (! nextFFTBlockReady)
            {
                std::fill (fftData.begin(), fftData.end(), 0.0f);
                std::copy (fifo.begin(), fifo.end(), fftData.begin());
                nextFFTBlockReady = true;
            }
 
            fifoIndex = 0;
        }
 
        fifo[(size_t) fifoIndex++] = sample;
    }
    
    // Draw spectrogram as background to entire synthesizer (one line at a time)
    // Implementation for spectrogram mostly comes from JUCE official tutorial on spectrograms
    // Modifications were made to adjust scaling, and fit for this apps purposes
    void drawNextLineOfSpectrogram()
    {
        auto rightHandEdge = spectrogramImage.getWidth() - 1;
        auto imageHeight   = spectrogramImage.getHeight();
 
        spectrogramImage.moveImageSection (0, 0, 1, 0, rightHandEdge, imageHeight);
 
        forwardFFT.performFrequencyOnlyForwardTransform (fftData.data());
 
        auto maxLevel = juce::FloatVectorOperations::findMinAndMax (fftData.data(), fftSize / 2);
 
        for (auto y = 1; y < imageHeight; ++y)
        {
            auto skewedProportionY = 1.0f - std::exp (std::log ((float) y / (float) imageHeight) * 0.07f);
            auto fftDataIndex = (size_t) juce::jlimit (0, fftSize / 2, (int) (skewedProportionY * fftSize / 2));
            auto level = juce::jmap (fftData[fftDataIndex], 0.0f, juce::jmax (maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);
 
            spectrogramImage.setPixelAt (rightHandEdge, y, juce::Colour::fromHSV (level, 1.0f, level, 1.0f));
        }
    }

    // Spectrogram public data members
    static constexpr auto fftOrder = 10;
    static constexpr auto fftSize  = 1 << fftOrder;
private:
    //==============================================================================
    
    // Define OSC port and route
    const OSCAddress OSCROUTE = "/juce/gestureData";
    const int OSCPORT = 9001;
    
    // Define spectrogram private data members
    juce::Image spectrogramImage;
    juce::dsp::FFT forwardFFT;
    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData;
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;
    
    // Voice 1 values
    float phaseVoice1       = 0.0f;
    float phaseDeltaVoice1  = 0.0f;
    float frequencyVoice1   = 5000.0f;
    float amplitudeVoice1   = 0.2f;
    float octaveMultiplierVoice1 = 1.0f;
    
    // Voice 2 values
    float phaseVoice2       = 0.0f;
    float phaseDeltaVoice2  = 0.0f;
    float frequencyVoice2   = 5000.0f;
    float amplitudeVoice2   = 0.2f;
    float octaveMultiplierVoice2 = 1.0f;
    
    // Voice 1 UI Components
    juce::Slider frequencyKnobVoice1;
    juce::Slider amplitudeKnobVoice1;
    juce::Slider timbreKnobVoice1;
    juce::Label frequencyLabelVoice1;
    juce::Label amplitudeLabelVoice1;
    juce::Label timbreLabelVoice1;
    
    // Voice 2 UI Components
    juce::Slider frequencyKnobVoice2;
    juce::Slider amplitudeKnobVoice2;
    juce::Slider timbreKnobVoice2;
    juce::Label frequencyLabelVoice2;
    juce::Label amplitudeLabelVoice2;
    juce::Label timbreLabelVoice2;
    
    double sampleRate = 0.0;
    int expectedSamplesPerBlock = 0;
    Point<float> lastMousePosition;

    // Function triggered whenever an OSC message is received on defined OSCROUTE and OSCPORT.
    // Acts as a server listeing to messages.
    // Expects 8 OSC messages in an array in following format:
    // int, int, float, float, float, float, float, float
    // The first two ints correspond to each voices octave as an int (1,2,3,4) respectively
    // The first three floats correspond to one voices values (frequency, magnitude, timbre)
    // Float values are expected to be between 0.0 and 1.0. Values are scaled / normalized after receiving them.
    void oscMessageReceived(const juce::OSCMessage& message) override
    {
        // Message is in correct format
        if (message.size() == 8 &&
            message[0].isInt32() && message[1].isInt32() &&
            message[2].isFloat32() && message[3].isFloat32() && message[4].isFloat32() &&
            message[5].isFloat32() && message[6].isFloat32() && message[7].isFloat32())
        {
            // Set octave multiplier
            octaveMultiplierVoice1 = static_cast<float>(message[0].getInt32());
            octaveMultiplierVoice2 = static_cast<float>(message[1].getInt32());
            
            // Error if octaveMultiplier is 0, default to 1.0
            if (octaveMultiplierVoice1 < 1.0)
                octaveMultiplierVoice1 = 1.0f;
            if (octaveMultiplierVoice2 < 1.0)
                octaveMultiplierVoice2 = 1.0f;
            
            // Voice 1 Set Values
            float magnitude1 = 1.0 - message[3].getFloat32();
            frequencyKnobVoice1.setValue(juce::jlimit(0.0f, 10.0f, message[2].getFloat32()));
            amplitudeKnobVoice1.setValue(juce::jlimit(0.0f, 10.0f, magnitude1));
            timbreKnobVoice1.setValue(juce::jlimit(0.0f, 10.0f, message[4].getFloat32()));

            // Voice 2 Set Values
            float magnitude2 = 1.0 - message[6].getFloat32();
            frequencyKnobVoice2.setValue(juce::jlimit(0.0f, 10.0f, message[5].getFloat32()));
            amplitudeKnobVoice2.setValue(juce::jlimit(0.0f, 10.0f, magnitude2));
            timbreKnobVoice2.setValue(juce::jlimit(0.0f, 10.0f, message[7].getFloat32()));
            int regions = 8;
            
            // Define Frequency Range
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

    // OSC Connection Error Alert Message (From JUCE example on OSC)
    void showConnectionErrorMessage (const juce::String& messageText)
    {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                                "Connection error",
                                                messageText,
                                                "OK");
    }
    

    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GestureSynthesizer)
    
};
