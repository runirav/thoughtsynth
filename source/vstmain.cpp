#include "ThoughtSynth.h"

// VST Plugin Entry Point
AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
    return new ThoughtSynth(audioMaster);
}

// DLL export for Windows
#ifdef _WIN32
#include <windows.h>

extern "C" {
    __declspec(dllexport) AEffect* VSTPluginMain(audioMasterCallback audioMaster)
    {
        if (!audioMaster(0, audioMasterVersion, 0, 0, 0, 0))
            return 0;  // Old host version
        
        AudioEffect* effect = createEffectInstance(audioMaster);
        if (!effect)
            return 0;
        
        return effect->getAeffect();
    }
    
    // Support older hosts (renamed to avoid conflict with C main)
    __declspec(dllexport) AEffect* MAIN(audioMasterCallback audioMaster)
    {
        return VSTPluginMain(audioMaster);
    }
}

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved)
{
    return TRUE;
}
#endif
