/*
  ==============================================================================

    Synth.h
    Created: 1 Mar 2024 2:53:03pm
    Author:  garam

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Voice.h"
#include "NoiseGenerator.h"

class Synth
{
public:
    Synth();

    void allocateResources(double sampleRAte, int samplesPerBlock);
    void deallocateResources();
    void reset();
    void render(float** outputBuffers, int sampleCount);
    void midiMessage(uint8_t data0, uint8_t data1, uint8_t data2);

    // Voice elements
    float noiseMix;
    float oscMix;
    float detune;

    // Global tuning
    float tune;

    // Envelope elements
    float envAttack;
    float envDecay;
    float envSustain;
    float envRelease;

    // Polyphony
    static constexpr int MAX_VOICES = 8;
    int numVoices;

    // Gain adjustment
    float volumeTrim;
    float velocitySensitivity;
    bool ignoreVelocity;

    // Output Level Slider
    juce::LinearSmoothedValue<float> outputLevelSmoother;

    // Modulation
    const int LFO_MAX = 32; // Downsampling factor
    float lfoInc;
    float vibrato;

    // PWM
    float pwmDepth;

    // Glide
    int glideMode;
    float glideRate;
    float glideBend;

    // Filter
    float filterKeyTracking;
    float filterQ;
    float filterLFODepth;

private:
    void startVoice(int v, int note, int velocity);
    void restartMonoVoice(int note, int velocity);
    void shiftQueuedNotes();
    int nextQueuedNote();
    void noteOn(int note, int velocity);
    void noteOff(int note);
    void controlChange(uint8_t data1, uint8_t data2);

    // Polyphony voice mgmt
    int findFreeVoice() const;

    // Optimized period calculation
    float calcPeriod(int v, int note) const;

    float sampleRate;
    std::array<Voice, MAX_VOICES> voices;
    NoiseGenerator noiseGen;

    float pitchBend;

    bool sustainPedalPressed;

    // Modulation
    void updateLFO();

    int lfoStep;
    float lfo;

    // Mod wheel
    float modWheel;

    // Glide
    int lastNote;
    inline void updatePeriod(Voice& voice)
    {
        voice.osc1.period = voice.period * pitchBend;
        voice.osc2.period = voice.osc1.period * detune;
    }

    bool isPlayingLegatoStyle() const;

    // Filter
    float resonanceCtl;
    float pressure;
    float filterCtl;
    float filterZip;
};