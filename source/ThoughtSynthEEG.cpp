#include "ThoughtSynthEEG.h"
#include <cstring>
#include <cstdlib>
#include <ctime>

// Plugin unique ID
const VstInt32 kUniqueId = CCONST('T', 'E', 'E', 'G');

// Constructor
ThoughtSynthEEG::ThoughtSynthEEG(audioMasterCallback audioMaster)
    : AudioEffectX(audioMaster, 1, 1),  // 1 program, 1 parameter (EEG on/off)
      eegBufferPos(0), eegUpdateCounter(0),
      currentFilterCutoff(0.5f), currentLFORate(0.5f), currentLFODepth(0.2f),
      currentWaveform(Waveform::SINE), eegEnabled(1.0f) // EEG enabled by default
{
    setNumInputs(0);          // No audio inputs (it's a synth)
    setNumOutputs(2);         // Stereo output
    setUniqueID(kUniqueId);   // Unique plugin ID
    canProcessReplacing();    // Supports replacing output
    isSynth();                // This is a synth
    
    // Create display window
    displayWindow = new EEGDisplayWindow();
    displayWindow->create();
    
    strcpy(programName, "EEG Default");
    
    // Initialize random seed for EEG generator
    srand((unsigned int)time(NULL));
    
    // Initialize EEG with default sample rate (will be updated later)
    eegGenerator.setSampleRate(44100.0f);
    bandExtractor.setSampleRate(250.0f);
    
    // Initialize all voices - EEG synth plays continuously!
    for (int i = 0; i < NUM_VOICES; i++)
    {
        voices[i] = EnhancedVoice();
        voices[i].setWaveform(currentWaveform);
        
        // Trigger voices with different notes for rich EEG-driven texture
        // C major chord: C3, E3, G3, C4, E4, G4, C5, E5
        int notes[] = {48, 52, 55, 60, 64, 67, 72, 76};
        voices[i].noteOn(notes[i], 0.5f, 44100.0f); // Start all voices at medium velocity
    }
    
    // Clear EEG buffer
    memset(eegBuffer, 0, sizeof(eegBuffer));
}

ThoughtSynthEEG::~ThoughtSynthEEG()
{
    if (displayWindow)
    {
        displayWindow->destroy();
        delete displayWindow;
    }
}

void ThoughtSynthEEG::setSampleRate(float sr)
{
    AudioEffectX::setSampleRate(sr);
    eegGenerator.setSampleRate(250.0f); // EEG at 250 Hz
    bandExtractor.setSampleRate(250.0f);
}

// Main audio processing with EEG integration
void ThoughtSynthEEG::processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames)
{
    float* outL = outputs[0];
    float* outR = outputs[1];
    
    // Clear output buffers
    memset(outL, 0, sampleFrames * sizeof(float));
    memset(outR, 0, sampleFrames * sizeof(float));
    
    // Generate and process EEG data
    // We generate at a slower rate (250 Hz) than audio (44.1kHz)
    // So we generate one EEG sample every ~176 audio samples at 44.1kHz
    int eegSamplesPerAudioSample = (int)(getSampleRate() / 250.0f);
    
    for (VstInt32 i = 0; i < sampleFrames; i++)
    {
        // Generate EEG sample periodically
        eegUpdateCounter++;
        if (eegUpdateCounter >= eegSamplesPerAudioSample)
        {
            eegUpdateCounter = 0;
            
            // Generate one EEG sample
            eegBuffer[eegBufferPos] = eegGenerator.generateSample();
            eegBufferPos++;
            
            // When buffer is full, extract band powers
            if (eegBufferPos >= EEG_BUFFER_SIZE)
            {
                eegBufferPos = 0;
                bandExtractor.processBuffer(eegBuffer, EEG_BUFFER_SIZE);
                updateParametersFromEEG();
                
                // Update display window with current EEG values
                if (displayWindow)
                {
                    displayWindow->update(
                        bandExtractor.getAlpha(),
                        bandExtractor.getBeta(),
                        bandExtractor.getTheta(),
                        eegEnabled >= 0.5f
                    );
                }
            }
        }
        
        // Process all active voices with current parameters
        float sample = 0.0f;
        
        for (int v = 0; v < NUM_VOICES; v++)
        {
            if (voices[v].active())
            {
                // Update voice parameters from EEG
                voices[v].setFilterCutoff(currentFilterCutoff);
                voices[v].setLFO(currentLFORate, currentLFODepth);
                
                sample += voices[v].process();
            }
        }
        
        // Output to both channels (stereo)
        outL[i] = sample;
        outR[i] = sample;
    }
}

// Update synthesis parameters based on EEG band powers
void ThoughtSynthEEG::updateParametersFromEEG()
{
    if (eegEnabled < 0.5f)
    {
        // EEG disabled - use static values
        currentFilterCutoff = 0.7f;
        currentLFORate = 0.0f;
        currentLFODepth = 0.0f;
        return;
    }
    
    float alpha = bandExtractor.getAlpha();
    float beta = bandExtractor.getBeta();
    float theta = bandExtractor.getTheta();
    
    // Map Alpha (8-13 Hz, relaxation) → Filter Cutoff
    // EXTREME range for obvious effect: 0.1 to 1.0
    currentFilterCutoff = 0.1f + (alpha * 0.9f);
    
    // Map Beta (13-30 Hz, alertness) → LFO Rate  
    // FAST modulation: 0.5 to 1.0 (5-10 Hz wobble)
    currentLFORate = 0.5f + (beta * 0.5f);
    
    // Map Theta (4-8 Hz, meditation) → LFO Depth
    // DEEP modulation: 0.2 to 0.8 (very audible)
    currentLFODepth = 0.2f + (theta * 0.6f);
}

// Handle MIDI events - NOT USED (EEG-only synth)
// This synth is driven by EEG waves, not MIDI!
VstInt32 ThoughtSynthEEG::processEvents(VstEvents* ev)
{
    // EEG-only synth - we don't process MIDI
    // Sound is generated purely from brain wave data
    return 0; // Return 0 to indicate we don't handle MIDI
}

// Trigger a note on
void ThoughtSynthEEG::noteOn(int noteNumber, int velocity)
{
    // Find a free voice or steal the oldest
    int freeVoice = -1;
    
    for (int i = 0; i < NUM_VOICES; i++)
    {
        if (!voices[i].active())
        {
            freeVoice = i;
            break;
        }
    }
    
    if (freeVoice == -1)
        freeVoice = 0; // Voice stealing
    
    float normalizedVelocity = velocity / 127.0f;
    voices[freeVoice].setWaveform(currentWaveform);
    voices[freeVoice].noteOn(noteNumber, normalizedVelocity, getSampleRate());
}

// Trigger a note off
void ThoughtSynthEEG::noteOff(int noteNumber)
{
    for (int i = 0; i < NUM_VOICES; i++)
    {
        if (voices[i].matchesNote(noteNumber))
        {
            voices[i].noteOff();
        }
    }
}

// Program name
void ThoughtSynthEEG::setProgramName(char* name)
{
    strncpy(programName, name, 31);
    programName[31] = '\0';
}

void ThoughtSynthEEG::getProgramName(char* name)
{
    strcpy(name, programName);
}

bool ThoughtSynthEEG::getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text)
{
    if (index == 0)
    {
        strcpy(text, programName);
        return true;
    }
    return false;
}

// Plugin identification
bool ThoughtSynthEEG::getEffectName(char* name)
{
    strcpy(name, "ThoughtSynth EEG");
    return true;
}

bool ThoughtSynthEEG::getVendorString(char* text)
{
    strcpy(text, "Neural Liberation");
    return true;
}

bool ThoughtSynthEEG::getProductString(char* text)
{
    strcpy(text, "ThoughtSynth EEG - Neurofeedback Synthesizer");
    return true;
}

VstInt32 ThoughtSynthEEG::getVendorVersion()
{
    return 2000;  // Version 2.0.0.0
}

VstInt32 ThoughtSynthEEG::canDo(char* text)
{
    if (strcmp(text, "receiveVstEvents") == 0) return 1;
    if (strcmp(text, "receiveVstMidiEvent") == 0) return 1;
    if (strcmp(text, "midiProgramNames") == 0) return 0;
    return -1;
}

// Parameter handling
void ThoughtSynthEEG::setParameter(VstInt32 index, float value)
{
    if (index == 0)
    {
        eegEnabled = value;
    }
}

float ThoughtSynthEEG::getParameter(VstInt32 index)
{
    if (index == 0)
        return eegEnabled;
    return 0.0f;
}

void ThoughtSynthEEG::getParameterName(VstInt32 index, char* label)
{
    if (index == 0)
        strcpy(label, "EEG Modulation");
}

void ThoughtSynthEEG::getParameterDisplay(VstInt32 index, char* text)
{
    if (index == 0)
        strcpy(text, eegEnabled >= 0.5f ? "ON" : "OFF");
}

void ThoughtSynthEEG::getParameterLabel(VstInt32 index, char* label)
{
    if (index == 0)
        strcpy(label, "");
}
