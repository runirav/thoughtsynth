#pragma once

#include <cmath>

// Waveform types
enum class Waveform
{
    SINE,
    SAW,
    SQUARE,
    TRIANGLE
};

// Simple one-pole low-pass filter
class SimpleFilter
{
public:
    SimpleFilter() : z1(0.0f), cutoff(1.0f) {}
    
    void setCutoff(float freq, float sampleRate)
    {
        // Simple coefficient calculation
        float omega = 2.0f * 3.14159265359f * freq / sampleRate;
        cutoff = 1.0f - expf(-omega);
        if (cutoff > 1.0f) cutoff = 1.0f;
        if (cutoff < 0.0f) cutoff = 0.0f;
    }
    
    float process(float input)
    {
        z1 = z1 + cutoff * (input - z1);
        return z1;
    }
    
    void reset() { z1 = 0.0f; }
    
private:
    float z1;
    float cutoff;
};

// Enhanced voice with multiple waveforms and filter
class EnhancedVoice
{
public:
    EnhancedVoice() :
        noteNumber(-1), velocity(0.0f),
        phase(0.0), phaseIncrement(0.0),
        envelope(0.0f), isActive(false), isReleasing(false),
        releaseCounter(0), waveform(Waveform::SINE),
        lfoPhase(0.0), lfoRate(0.0), lfoDepth(0.0f)
    {}
    
    void noteOn(int note, float vel, float sampleRate)
    {
        noteNumber = note;
        velocity = vel;
        sampleRate_ = sampleRate;
        
        // Safety check for sample rate
        if (sampleRate <= 0.0f)
        {
            sampleRate_ = 44100.0f; // Default fallback
            sampleRate = 44100.0f;
        }
        
        // Convert MIDI note to frequency
        float frequency = 440.0f * powf(2.0f, (note - 69) / 12.0f);
        phaseIncrement = (2.0 * 3.14159265359 * frequency) / sampleRate;
        
        phase = 0.0;
        lfoPhase = 0.0;
        envelope = 1.0f;
        isActive = true;
        isReleasing = false;
        releaseCounter = 0;
        
        filter.reset();
        filter.setCutoff(20000.0f, sampleRate); // Wide open initially
    }
    
    void noteOff()
    {
        isReleasing = true;
    }
    
    void setWaveform(Waveform wf) { waveform = wf; }
    
    void setFilterCutoff(float cutoffNormalized)
    {
        // Map 0-1 to 200-20000 Hz (log scale)
        float cutoffHz = 200.0f * powf(100.0f, cutoffNormalized); // 200 to 20kHz
        filter.setCutoff(cutoffHz, sampleRate_);
    }
    
    void setLFO(float rate, float depth)
    {
        lfoRate = rate * 10.0f; // 0-1 → 0-10 Hz
        lfoDepth = depth;
    }
    
    float process()
    {
        if (!isActive) return 0.0f;
        
        // Generate LFO (for vibrato/tremolo)
        float lfo = sinf((float)lfoPhase) * lfoDepth;
        lfoPhase += (2.0 * 3.14159265359 * lfoRate) / sampleRate_;
        if (lfoPhase >= 2.0 * 3.14159265359)
            lfoPhase -= 2.0 * 3.14159265359;
        
        // Generate oscillator sample
        float sample = 0.0f;
        float p = (float)phase;
        
        switch (waveform)
        {
            case Waveform::SINE:
                sample = sinf(p);
                break;
                
            case Waveform::SAW:
                sample = (p / 3.14159265359f) - 1.0f;
                break;
                
            case Waveform::SQUARE:
                sample = (p < 3.14159265359f) ? 1.0f : -1.0f;
                break;
                
            case Waveform::TRIANGLE:
                sample = (p < 3.14159265359f) ?
                    (-1.0f + (2.0f * p / 3.14159265359f)) :
                    (3.0f - (2.0f * p / 3.14159265359f));
                break;
        }
        
        // Apply LFO as tremolo (amplitude modulation)
        sample *= (1.0f + lfo * 0.5f);
        
        // Apply envelope
        sample *= velocity * 0.15f * envelope;
        
        // Apply filter
        sample = filter.process(sample);
        
        // Advance phase
        phase += phaseIncrement;
        if (phase >= 2.0 * 3.14159265359)
            phase -= 2.0 * 3.14159265359;
        
        // Envelope (release)
        if (isReleasing)
        {
            releaseCounter++;
            envelope = 1.0f - (releaseCounter / 22050.0f); // ~0.5 sec
            
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
    float sampleRate_;
    double phase;
    double phaseIncrement;
    float envelope;
    bool isActive;
    bool isReleasing;
    int releaseCounter;
    
    Waveform waveform;
    SimpleFilter filter;
    
    double lfoPhase;
    float lfoRate;
    float lfoDepth;
};
