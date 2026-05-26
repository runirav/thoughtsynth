#pragma once

#include "public.sdk/source/vst2.x/audioeffectx.h"
#include <cmath>
#include <vector>

#define NUM_VOICES 8

// Simple sine wave voice for polyphonic synthesis
class SineVoice
{
public:
    SineVoice() : noteNumber(-1), velocity(0.0f), phase(0.0), phaseIncrement(0.0),
                  envelope(0.0f), isActive(false), isReleasing(false), releaseCounter(0) {}
    
    void noteOn(int note, float vel, float sampleRate)
    {
        noteNumber = note;
        velocity = vel;
        
        // Convert MIDI note to frequency
        float frequency = 440.0f * powf(2.0f, (note - 69) / 12.0f);
        phaseIncrement = (2.0 * 3.14159265359 * frequency) / sampleRate;
        
        phase = 0.0;
        envelope = 1.0f;
        isActive = true;
        isReleasing = false;
        releaseCounter = 0;
    }
    
    void noteOff()
    {
        isReleasing = true;
    }
    
    float process()
    {
        if (!isActive) return 0.0f;
        
        // Generate sine wave
        float sample = sinf((float)phase) * velocity * 0.15f * envelope;
        
        // Advance phase
        phase += phaseIncrement;
        if (phase >= 2.0 * 3.14159265359)
            phase -= 2.0 * 3.14159265359;
        
        // Simple envelope (release only for now)
        if (isReleasing)
        {
            releaseCounter++;
            envelope = 1.0f - (releaseCounter / 22050.0f); // ~0.5 sec release at 44.1kHz
            
            if (envelope <= 0.0f)
            {
                isActive = false;
                envelope = 0.0f;
                return 0.0f;
            }
        }
        
        return sample;
    }
    
    bool matchesNote(int note) const { return noteNumber == note && isActive; }
    bool active() const { return isActive; }
    
private:
    int noteNumber;
    float velocity;
    double phase;
    double phaseIncrement;
    float envelope;
    bool isActive;
    bool isReleasing;
    int releaseCounter;
};

// Main VST2 plugin class
class ThoughtSynth : public AudioEffectX
{
public:
    ThoughtSynth(audioMasterCallback audioMaster);
    ~ThoughtSynth();
    
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
    
private:
    void noteOn(int noteNumber, int velocity);
    void noteOff(int noteNumber);
    
    SineVoice voices[NUM_VOICES];
    char programName[32];
};
