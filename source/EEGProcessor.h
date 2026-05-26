#pragma once

#include <vector>
#include <cmath>

// Simple EEG band power extractor
// Extracts Alpha, Beta, Theta from raw EEG signal
class EEGBandExtractor
{
public:
    EEGBandExtractor() : 
        alphaPower(0.5f), betaPower(0.5f), thetaPower(0.5f),
        sampleRate(250.0f) // Default BrainFlow synthetic rate
    {
    }
    
    void setSampleRate(float sr) { sampleRate = sr; }
    
    // Process a buffer of EEG samples
    void processBuffer(const float* samples, int numSamples)
    {
        if (numSamples < 32) return; // Need minimum samples
        
        // Simple power estimation in each band
        // In real implementation, use FFT or bandpass filters
        // For now, use simplified frequency analysis
        
        float lowPower = 0.0f;   // Theta (4-8 Hz)
        float midPower = 0.0f;   // Alpha (8-13 Hz)
        float highPower = 0.0f;  // Beta (13-30 Hz)
        
        // Count zero crossings to estimate frequency content
        int lowCross = 0, midCross = 0, highCross = 0;
        
        for (int i = 1; i < numSamples; i++)
        {
            float prev = samples[i-1];
            float curr = samples[i];
            
            if ((prev < 0 && curr >= 0) || (prev >= 0 && curr < 0))
            {
                // Estimate frequency from spacing
                // This is simplified - real impl uses FFT
                lowCross++;
            }
            
            // Accumulate power
            lowPower += curr * curr;
        }
        
        // Normalize and smooth
        float totalPower = lowPower / numSamples;
        
        // Distribute power across bands (simplified)
        // Real implementation: bandpass filter + RMS
        thetaPower = smoothParameter(thetaPower, totalPower * 0.3f, 0.95f);
        alphaPower = smoothParameter(alphaPower, totalPower * 0.5f, 0.95f);
        betaPower = smoothParameter(betaPower, totalPower * 0.2f, 0.95f);
        
        // Clamp to 0-1 range
        thetaPower = clamp(thetaPower, 0.0f, 1.0f);
        alphaPower = clamp(alphaPower, 0.0f, 1.0f);
        betaPower = clamp(betaPower, 0.0f, 1.0f);
    }
    
    // Get current band powers (0-1 normalized)
    float getAlpha() const { return alphaPower; }
    float getBeta() const { return betaPower; }
    float getTheta() const { return thetaPower; }
    
private:
    float alphaPower;
    float betaPower;
    float thetaPower;
    float sampleRate;
    
    float smoothParameter(float current, float target, float smoothing)
    {
        return current * smoothing + target * (1.0f - smoothing);
    }
    
    float clamp(float value, float min, float max)
    {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }
};

// Synthetic EEG generator for emulation
// Generates realistic brain wave patterns without hardware
class SyntheticEEG
{
public:
    SyntheticEEG() :
        phase1(0.0), phase2(0.0), phase3(0.0),
        sampleRate(250.0f),
        statePhase(0.0), transitionPhase(0.0)
    {
        // Start with random initial state
        currentState = rand() % 3; // 0=relaxed, 1=focused, 2=meditative
        nextState = (currentState + 1) % 3;
    }
    
    void setSampleRate(float sr) { sampleRate = sr; }
    
    // Generate one sample of synthetic EEG
    float generateSample()
    {
        // REALISTIC BRAIN STATE SIMULATION
        // Simulates transitions between mental states: relaxed, focused, meditative
        
        // Update state transition (every ~5 seconds)
        statePhase += 1.0 / sampleRate;
        if (statePhase >= 5.0) // Transition every 5 seconds
        {
            statePhase = 0.0;
            currentState = nextState;
            nextState = rand() % 3; // Random next state
        }
        
        // Smooth transition between states
        transitionPhase += (1.0 / sampleRate) / 2.0; // 2 second transition
        if (transitionPhase > 1.0) transitionPhase = 1.0;
        
        // Define amplitude targets for each state
        float thetaTarget, alphaTarget, betaTarget;
        
        // Current state amplitudes
        switch (currentState)
        {
            case 0: // Relaxed (high alpha, low beta)
                thetaTarget = 0.3f;
                alphaTarget = 0.8f;
                betaTarget = 0.2f;
                break;
            case 1: // Focused (high beta, medium alpha)
                thetaTarget = 0.2f;
                alphaTarget = 0.4f;
                betaTarget = 0.9f;
                break;
            case 2: // Meditative (high theta, medium alpha)
                thetaTarget = 0.9f;
                alphaTarget = 0.5f;
                betaTarget = 0.1f;
                break;
        }
        
        // Next state amplitudes
        float thetaNext, alphaNext, betaNext;
        switch (nextState)
        {
            case 0:
                thetaNext = 0.3f; alphaNext = 0.8f; betaNext = 0.2f;
                break;
            case 1:
                thetaNext = 0.2f; alphaNext = 0.4f; betaNext = 0.9f;
                break;
            case 2:
                thetaNext = 0.9f; alphaNext = 0.5f; betaNext = 0.1f;
                break;
        }
        
        // Interpolate between states
        float t = transitionPhase;
        float thetaAmp = thetaTarget * (1.0f - t) + thetaNext * t;
        float alphaAmp = alphaTarget * (1.0f - t) + alphaNext * t;
        float betaAmp = betaTarget * (1.0f - t) + betaNext * t;
        
        // Add natural variation (micro-fluctuations)
        thetaAmp += 0.1f * sinf((float)phase1 * 0.1f);
        alphaAmp += 0.15f * sinf((float)phase2 * 0.15f);
        betaAmp += 0.1f * sinf((float)phase3 * 0.2f);
        
        // Clamp to 0-1 range
        thetaAmp = fmaxf(0.0f, fminf(1.0f, thetaAmp));
        alphaAmp = fmaxf(0.0f, fminf(1.0f, alphaAmp));
        betaAmp = fmaxf(0.0f, fminf(1.0f, betaAmp));
        
        // Generate waves with state-dependent amplitudes
        float theta = sinf((float)phase1) * thetaAmp;
        float alpha = sinf((float)phase2) * alphaAmp;
        float beta = sinf((float)phase3) * betaAmp;
        
        // Add realistic noise
        float noise = ((float)rand() / RAND_MAX - 0.5f) * 0.15f;
        
        // Advance phases
        phase1 += (2.0 * 3.14159265359 * 6.0) / sampleRate;    // 6 Hz
        phase2 += (2.0 * 3.14159265359 * 10.0) / sampleRate;   // 10 Hz
        phase3 += (2.0 * 3.14159265359 * 20.0) / sampleRate;   // 20 Hz
        
        // Wrap phases
        if (phase1 >= 2.0 * 3.14159265359) phase1 -= 2.0 * 3.14159265359;
        if (phase2 >= 2.0 * 3.14159265359) phase2 -= 2.0 * 3.14159265359;
        if (phase3 >= 2.0 * 3.14159265359) phase3 -= 2.0 * 3.14159265359;
        
        // Reset transition when starting new state
        if (statePhase < 0.01)
            transitionPhase = 0.0;
        
        return theta + alpha + beta + noise;
    }
    
    // Generate buffer of samples
    void generateBuffer(float* buffer, int numSamples)
    {
        for (int i = 0; i < numSamples; i++)
        {
            buffer[i] = generateSample();
        }
    }
    
private:
    double phase1, phase2, phase3;
    float sampleRate;
    
    // State machine for realistic brain states
    int currentState;  // 0=relaxed, 1=focused, 2=meditative
    int nextState;
    double statePhase;      // Time in current state
    double transitionPhase; // Transition progress 0-1
};
