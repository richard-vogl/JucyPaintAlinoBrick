/*
 ==============================================================================
 
 This file was auto-generated!
 
 ==============================================================================
 */

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "OSCStuff.h"

#define PAINT_CONTROLLER_NR 104
#define MAX_MOD_FREQ1 8

//==============================================================================
class MainContentComponent   : public AudioAppComponent, Timer, MidiInputCallback {
    
public:
    //==============================================================================
    MainContentComponent()
    : phase (0.0f),
    phaseDelta (0.0f),
    frequency (5000.0f),
    amplitude (0.2f),
    sampleRate (0.0),
    expectedSamplesPerBlock (0) {
        
        bitalinoSocketListener = new SocketListener(&bitalinoListener, PORT_BITALINO);
        gestureSocketListener = new SocketListener(&gestureListener, PORT_GESTURE);
        
        StringArray midiDevices = MidiInput::getDevices();
        
        if (midiDevices.size() >= 1) {
            midiInput = MidiInput::openDevice(0, this);
        }
        
        
        setSize (800, 600);
        
        //prepareToPlay will be called in this! init everything before!
        
        
        // specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
        this->startTimer(20);

        
    }
    
    ~MainContentComponent() {
        shutdownAudio();
        
    }
    
    bool init = true;
    int repaintCount = 0;
#define MAX_REPAINT_COUNT 100
    
    //=======================================================================
    void prepareToPlay (int samplesPerBlockExpected, double newSampleRate) override {
        sampleRate = newSampleRate;
        expectedSamplesPerBlock = samplesPerBlockExpected;
        
        if (init) {
            gestureSocketListener->startListening();
            bitalinoSocketListener->startListening();
            init = false;
        }
        if (midiInput != nullptr) {
            midiInput->start();
        }
    }
    
    /*  This method generates the actual audio samples.
     In this example the buffer is filled with a sine wave whose frequency and
     amplitude are controlled by the mouse position.
     */
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override {
        float pitch = gestureListener.getPitch();
        float yaw = gestureListener.getYaw();
        float roll = gestureListener.getRoll();
        
        float switchState = gestureListener.getSwitchState();
        
        float accel = gestureListener.getAccelMag();
        
        float emgStd = bitalinoListener.getEmgStd();

        
       // frequency = emgStd;
        amplitude = (180.0f+yaw) / 360.0f  * 2;

        
        if (controllerValue > 10) {
            // do runnaway freq;
            frequency += controllerValue*2;
        } else {
            frequency = lastMidiFreq;
        }
        
        float useFrequency = frequency + accel/200.0f - 21;
        if (switchState <= 0) {
            useFrequency = frequency *2;
        }
        
        if (!noteOn) {
            
            amplitude = 0;
            
        }
        
        if (usedAmplitude > amplitude) {
            usedAmplitude-=0.05;
        } else if (usedAmplitude < amplitude) {
            usedAmplitude+=0.1;
        }
        
        if (usedAmplitude <= 0.01) {
            usedAmplitude = 0;
        }
        
        float modFrequ1 = (180.0f + pitch) / 360.0f * MAX_MOD_FREQ1;
        float cut = (180.0f + roll) / 360.0f  * 0.9 + 0.1;
        
        phaseDelta = 2.0f * float_Pi * useFrequency / sampleRate;
        modPhaseDelta = 2.0f * float_Pi *modFrequ1 / sampleRate;
        phaseDeltaBass = 2.0f * float_Pi * frequencyBass/ sampleRate;
        
        bufferToFill.clearActiveBufferRegion();
        const float originalPhase = phase;
        const float originalBassPhase = phaseBass;
        
        float basscut = ((emgStd/50.0f) + 1.0f) / 4.0f;
        
        int chan = 0;
            phase = originalPhase;
            phaseBass = originalBassPhase;
        
        // we know it's stereo - so fill both:
            float* const channelData = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
            float* const channelData2 = bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample);

            
            for (int i = 0; i < bufferToFill.numSamples ; ++i)
            {
                
                float random  = (float)std::rand() / (float)RAND_MAX;
                channelData[i] = usedAmplitude *  std::min(cut, std::max(std::sin (phase), -cut)) * (std::sin (modPhase) + 2)/3 +
                1.3f*std::min(basscut, std::max(std::sin (phaseBass), -basscut));
                
                channelData2[i] = channelData[i];
                
                // increment the phase step for the next sample
                phase = std::fmod (phase + phaseDelta, float_Pi * 2.0f);
                modPhase = std::fmod (modPhase + modPhaseDelta, float_Pi * 2.0f);
                
                phaseBass = std::fmod (phaseBass + phaseDeltaBass, float_Pi * 2.0f);
            }

        
        repaintCount++;
        if (repaintCount > MAX_REPAINT_COUNT) {
            repaintCount = 0;
        }

    }
    
    void releaseResources() override  {
        // This gets automatically called when audio device paramters change
        // or device is restarted.
        
        //        bitalinoSocketListener->stopListening();
        //        gestureSocketListener->stopListening();
        
         if (midiInput != nullptr) {
             midiInput->stop();
         }
    }
    
    
    //=======================================================================
    void paint (Graphics& g) override
    {
        
        float pitch = gestureListener.getPitch();
        float yaw = gestureListener.getYaw();
        float roll = gestureListener.getRoll();
        
        float switchState = gestureListener.getSwitchState();
        
        float accel = gestureListener.getAccelMag();
        
        float emgStd = bitalinoListener.getEmgStd();
        
        float * emgBuffer = bitalinoListener.getEmgBuffer();
        int curBufferIdx = bitalinoListener.getCurEmgBuffIdx();
        
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (Colours::black);
        
        g.setColour(Colours::whitesmoke);
        g.fillRect(10, 0, 10, 200 + (int) pitch);
        g.fillRect(30, 0, 10, 200 + (int) roll);
        g.fillRect(50, 0, 10, 200 + (int) yaw);
        
        g.fillRect(70, 0, 10, 200 + (int) accel/100);
        
        g.setColour(Colours::wheat);
        g.fillRect(110, 0, 10, 200 + (int) emgStd);
        
        const float centreY = getHeight() / 2.0f;
        const float radius = amplitude * 200.0f;
        
        // Draw an ellipse based on the mouse position and audio volume
        if (noteOn) {
        g.setColour (Colours::lightgreen);
        g.fillEllipse  (100 - radius / 2.0f,
                        100 - radius / 2.0f,
                        radius, radius);
        }
        
        // draw a representative sinewave
        Path wavePath;
        wavePath.startNewSubPath (0, centreY);
        
      //  for (float x = 1.0f; x < getWidth(); ++x) {
        for (float x = 0.0f; x < BUFFER_SIZE; x++) {
            //wavePath.lineTo (x, centreY + amplitude * getHeight() * 2.0f
              //               * std::sin (x * frequency * 0.0001f));
            float xCord = x*2;
            float yCord = std::min(std::max(0.0f, (float) emgBuffer[(int)(x+curBufferIdx) % BUFFER_SIZE]/10), (float)getWidth());
            wavePath.lineTo (xCord, yCord);
            if ((int)x == curBufferIdx) {
                g.setColour (Colours::red);
                g.fillEllipse  (xCord,
                                yCord,
                                4, 4);
            }
        }
        g.setColour (Colours::grey);
        g.strokePath (wavePath, PathStrokeType (2.0f));
    }
    
    virtual void timerCallback() {
        repaint();
    }
    
    
    virtual void handleIncomingMidiMessage (MidiInput* source,
                                            const MidiMessage& message) {
        //int channel = message.getChannel();
        
        if (message.isNoteOn()) {
            if (message.getNoteNumber() < 48) {
                //bass
                frequencyBass =message.getMidiNoteInHertz(message.getNoteNumber());
                
            } else {
                this->frequency = message.getMidiNoteInHertz(message.getNoteNumber());
                this->lastMidiFreq = frequency;
                this->noteOn = true;
            }
        }
        
        if (message.isNoteOff() || message.isAllNotesOff() || message.isAllSoundOff()) {
            
            if (message.getNoteNumber() < 48) {
                //bass
                frequencyBass = 0;
                
            } else {
                this->noteOn = false;
            }
        }
        
        if (message.isController()) {
            int controllerNr = message.getControllerNumber();
            int controllerValue = message.getControllerValue();
            
            if (controllerNr == PAINT_CONTROLLER_NR) {
                this->controllerValue = controllerValue;
            }
        }
    }
    
    // Mouse handling..
    void mouseDown (const MouseEvent& e) override {
        mouseDrag (e);
    }
    
    void mouseDrag (const MouseEvent& e) override {
        lastMousePosition = e.position;
        repaint();
    }
    
    void mouseUp (const MouseEvent&) override  {
        repaint();
    }
    
    void resized() override   {

    }
    
    
private:
    //==============================================================================
    float phase;
    float modPhase;
    float phaseDelta;
    float modPhaseDelta;
    float frequency;
    float lastMidiFreq;
    float amplitude;
    float usedAmplitude = 0;
    
    float frequencyBass = 0;
    float phaseBass;
    float phaseDeltaBass;
    
    int controllerValue;
    
    double sampleRate;
    
    int expectedSamplesPerBlock;
    Point<float> lastMousePosition;
    
    ScopedPointer<SocketListener> bitalinoSocketListener;
    ScopedPointer<SocketListener> gestureSocketListener;
    
    GesturePacketListener gestureListener;
    BitalinoPacketListener bitalinoListener;
    
    ScopedPointer<MidiInput> midiInput;

    bool noteOn = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


Component* createMainContentComponent() { return new MainContentComponent(); };

#endif  // MAINCOMPONENT_H_INCLUDED
