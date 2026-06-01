# ThoughtSynth EEG 🧠→🎵

> *Your neural patterns are music. This synthesizer proves it.*

---

## What Is This?

**ThoughtSynth EEG** is an open-source VST2 synthesizer that turns brain wave activity into sound in real time.

No keyboard. No MIDI. No notes.

Just your brain — and whatever it feels like doing.

Built on the **BrainFlow library**, it reads EEG data from BCI headsets (or an emulator when no hardware is connected) and maps Alpha, Beta, and Theta wave activity directly to synthesis parameters. The result is a sound that evolves with your mental state — something no conventional instrument can do.

---

## How It Works

```
EEG Source (hardware or emulator)
         ↓
   Band Extraction
  Alpha │ Beta │ Theta
    8Hz │ 20Hz │  6Hz
         ↓
  Parameter Mapping
  Filter │ LFO Rate │ LFO Depth
         ↓
   Audio Synthesis
   8-Voice Polyphony
   Multi-Waveform Oscillators
         ↓
     VST2 Output
```

**Brain state → Sound:**

| Wave | Mental State | Controls |
|------|-------------|----------|
| Alpha (8–13 Hz) | Relaxed, creative | Filter cutoff — brightness |
| Beta (13–30 Hz) | Focused, alert | LFO rate — modulation speed |
| Theta (4–8 Hz) | Meditative, deep | LFO depth — modulation intensity |

The synth ships with a **realistic brain state emulator** simulating three states: Relaxed, Focused, and Meditative — with smooth 2-second transitions between them. You can hear what your brain *would* sound like.

---

## Current State

This is an **early release**. It works. It makes sound. The EEG modulation is audible and visible in the real-time monitor window.

What's in:
- ✅ VST2 plugin (Windows, MinGW-built, no Visual Studio required)
- ✅ 8-voice harmonic synthesis
- ✅ Multi-waveform oscillators (sine, saw, square, triangle)
- ✅ Dynamic filter controlled by Alpha waves
- ✅ LFO modulation controlled by Beta/Theta waves
- ✅ Built-in EEG emulator with realistic brain state simulation
- ✅ Real-time EEG monitor showing Alpha/Beta/Theta levels
- ✅ Minimal standalone VST host for testing without a DAW

What's coming:
- 🔲 GUI inside the VST window (space/motion-blur aesthetic)
- 🔲 Real BrainFlow hardware integration (Muse, OpenBCI, etc.)
- 🔲 Stereo shaping and spatial audio
- 🔲 Atmospheric timbre improvements
- 🔲 macOS and Linux builds
- 🔲 Recording / export to WAV

---

## Why Does This Exist?

Read the [MANIFESTO](MANIFESTO.md) for the full picture.

The short version: BCI technology should serve the people it reaches — not the systems that regulate access to it. Brain wave data is personal, private, and deeply human. Turning it into music is a way of saying: *this is yours, and it is valid.*

This synthesizer is aimed at people who have been told they are too different, too difficult, too broken to fit. It says the opposite. Your patterns are not a diagnosis. They are a composition.

---

## Requirements

- Windows 10/11 (64-bit)
- Any VST2-compatible DAW (FL Studio, Ableton, Reaper, etc.)
- MinGW/GCC + CMake (to build from source)

No ASIO required. No external audio drivers. No MIDI hardware.

---

## Build from Source

```cmd
git clone https://github.com/YOUR_USERNAME/ThoughtSynthEEG.git
cd ThoughtSynthEEG

REM Download VST2 SDK separately (see below)
REM Place at C:\Claude\VST_SDK\VST2_SDK\

build.bat
```

**VST2 SDK:** Steinberg no longer distributes VST2 officially. The last version (3.6.10, 2018) can be found archived. Place it at the path above or update `CMakeLists.txt` to point to your copy.

---

## Project Structure

```
ThoughtSynthEEG/
├── source/
│   ├── ThoughtSynthEEG.h/.cpp    # Main plugin
│   ├── EnhancedVoice.h           # Oscillators + filter + LFO
│   ├── EEGProcessor.h            # Synthetic EEG + band extraction
│   ├── EEGDisplayWindow.h        # Real-time monitor window
│   └── vstmain_eeg.cpp           # Entry point
├── host/
│   ├── VSTHost.h                 # Minimal VST2 host
│   └── main.cpp                  # Standalone test host
├── MANIFESTO.md
└── CMakeLists.txt
```

---

## What's Next in This Series

**ThoughtSynth EEG** is the first tool in a wider project exploring music, BCI, and human autonomy. Coming:

- **Analog Synthesizer (Hardware)** — A compact physical synth, a few knobs, semi-modular. Built by hand, sold by order.
- **Amp Plugin** — A VST2/VST3 amplifier plugin for guitar/bass with complex harmonic saturation, inspired by the best in class.

Follow this repository or the project site to stay updated.

---

## Contribute

If you have a BCI headset, test it and share findings. If you are a developer, the code is simple C++ — no frameworks, no abstraction layers. If you have ideas, open an issue. If you find this useful, share it.

---

## License

MIT License. Do what you want with it. If you build something with it, a mention is appreciated but not required.

VST2 SDK has its own license (Steinberg). Review it before commercial distribution.

---

*Built against the current. For those the current forgot.*
