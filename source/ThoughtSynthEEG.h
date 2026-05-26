#pragma once

#include "public.sdk/source/vst2.x/audioeffectx.h"
#include "EnhancedVoice.h"
#include "EEGProcessor.h"
#include "EEGDisplayWindow.h"
#include <vector>

#define NUM_VOICES 8
#define EEG_BUFFER_SIZE 256

// Main EEG-driven synthesizer plugin
class ThoughtSynthEEG : public AudioEffectX
{
public:
    ThoughtSynthEEG(audioMasterCallback audioMaster);
    ~ThoughtSynthEEG();
    
    // Processing
    virtual void processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames);
    virtual VstInt32 processEvents(VstEvents* events);
    
    // Program/Parameters
    virtual void setProgramName(char* name);
    virtual void getProgramName(char* name);
    virtual bool getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text);
    
    // Plugin properties
    virtual bool getEffectName(char* name);
    virtual bool getVendorString(char* text);
    virtual bool getProductString(char* text);
    virtual VstInt32 getVendorVersion();
    virtual VstInt32 canDo(char* text);
    
    // MIDI
    virtual VstInt32 getNumMidiInputChannels() { return 1; }
    virtual VstInt32 getNumMidiOutputChannels() { return 0; }
    
    // Sample rate
    virtual void setSampleRate(float sr);
    
    // Parameters
    virtual void setParameter(VstInt32 index, float value);
    virtual float getParameter(VstInt32 index);
    virtual void getParameterName(VstInt32 index, char* label);
    virtual void getParameterDisplay(VstInt32 index, char* text);
    virtual void getParameterLabel(VstInt32 index, char* label);
    
private:
    void noteOn(int noteNumber, int velocity);
    void noteOff(int noteNumber);
    void updateParametersFromEEG();
    
    EnhancedVoice voices[NUM_VOICES];
    char programName[32];
    
    // EEG processing
    SyntheticEEG eegGenerator;
    EEGBandExtractor bandExtractor;
    float eegBuffer[EEG_BUFFER_SIZE];
    int eegBufferPos;
    int eegUpdateCounter;
    
    // Current EEG-controlled parameters
    float currentFilterCutoff;
    float currentLFORate;
    float currentLFODepth;
    
    // Waveform selection (could be MIDI CC or automated)
    Waveform currentWaveform;
    
    // EEG toggle parameter
    float eegEnabled; // 0.0 = off, 1.0 = on
    
    // Display window
    EEGDisplayWindow* displayWindow;
};
