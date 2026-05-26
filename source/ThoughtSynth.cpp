#include "ThoughtSynth.h"
#include <cstring>

// Plugin unique ID - 4-byte integer (change this for your own plugin!)
// 'TSyn' = 0x5453796E in hex
const VstInt32 kUniqueId = CCONST('T', 'S', 'y', 'n');

// Constructor
ThoughtSynth::ThoughtSynth(audioMasterCallback audioMaster)
    : AudioEffectX(audioMaster, 1, 0)  // 1 program, 0 parameters
{
    setNumInputs(0);          // No audio inputs (it's a synth)
    setNumOutputs(2);         // Stereo output
    setUniqueID(kUniqueId);   // Unique plugin ID
    canProcessReplacing();    // Supports replacing output
    isSynth();                // This is a synth
    
    strcpy(programName, "Default");
    
    // Initialize all voices as inactive
    for (int i = 0; i < NUM_VOICES; i++)
    {
        voices[i] = SineVoice();
    }
}

ThoughtSynth::~ThoughtSynth()
{
}

// Main audio processing
void ThoughtSynth::processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames)
{
    float* outL = outputs[0];
    float* outR = outputs[1];
    
    // Clear output buffers
    memset(outL, 0, sampleFrames * sizeof(float));
    memset(outR, 0, sampleFrames * sizeof(float));
    
    // Process all active voices
    for (VstInt32 i = 0; i < sampleFrames; i++)
    {
        float sample = 0.0f;
        
        for (int v = 0; v < NUM_VOICES; v++)
        {
            if (voices[v].active())
            {
                sample += voices[v].process();
            }
        }
        
        // Output to both channels (stereo)
        outL[i] = sample;
        outR[i] = sample;
    }
}

// Handle MIDI events
VstInt32 ThoughtSynth::processEvents(VstEvents* ev)
{
    for (VstInt32 i = 0; i < ev->numEvents; i++)
    {
        if ((ev->events[i])->type != kVstMidiType)
            continue;
        
        VstMidiEvent* event = (VstMidiEvent*)ev->events[i];
        char* midiData = event->midiData;
        
        VstInt32 status = midiData[0] & 0xf0;  // Status byte
        
        if (status == 0x90)  // Note On
        {
            int note = midiData[1] & 0x7f;
            int velocity = midiData[2] & 0x7f;
            
            if (velocity > 0)
                noteOn(note, velocity);
            else
                noteOff(note);  // velocity 0 = note off
        }
        else if (status == 0x80)  // Note Off
        {
            int note = midiData[1] & 0x7f;
            noteOff(note);
        }
    }
    
    return 1;
}

// Trigger a note on
void ThoughtSynth::noteOn(int noteNumber, int velocity)
{
    // Find a free voice or steal the oldest one
    int freeVoice = -1;
    
    // First, try to find an inactive voice
    for (int i = 0; i < NUM_VOICES; i++)
    {
        if (!voices[i].active())
        {
            freeVoice = i;
            break;
        }
    }
    
    // If no free voice, steal voice 0 (simple voice stealing)
    if (freeVoice == -1)
        freeVoice = 0;
    
    // Start the note
    float normalizedVelocity = velocity / 127.0f;
    voices[freeVoice].noteOn(noteNumber, normalizedVelocity, getSampleRate());
}

// Trigger a note off
void ThoughtSynth::noteOff(int noteNumber)
{
    // Find all voices playing this note and release them
    for (int i = 0; i < NUM_VOICES; i++)
    {
        if (voices[i].matchesNote(noteNumber))
        {
            voices[i].noteOff();
        }
    }
}

// Program name
void ThoughtSynth::setProgramName(char* name)
{
    strncpy(programName, name, 31);
    programName[31] = '\0';
}

void ThoughtSynth::getProgramName(char* name)
{
    strcpy(name, programName);
}

bool ThoughtSynth::getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text)
{
    if (index == 0)
    {
        strcpy(text, programName);
        return true;
    }
    return false;
}

// Plugin identification
bool ThoughtSynth::getEffectName(char* name)
{
    strcpy(name, "ThoughtSynth");
    return true;
}

bool ThoughtSynth::getVendorString(char* text)
{
    strcpy(text, "ThoughtWave");
    return true;
}

bool ThoughtSynth::getProductString(char* text)
{
    strcpy(text, "ThoughtSynth VST2");
    return true;
}

VstInt32 ThoughtSynth::getVendorVersion()
{
    return 1000;  // Version 1.0.0.0
}

VstInt32 ThoughtSynth::canDo(char* text)
{
    if (strcmp(text, "receiveVstEvents") == 0) return 1;
    if (strcmp(text, "receiveVstMidiEvent") == 0) return 1;
    if (strcmp(text, "midiProgramNames") == 0) return 0;
    return -1;  // Unknown
}
