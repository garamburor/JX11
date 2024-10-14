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

private:
    void startVoice(int v, int note, int velocity);
    void noteOn(int note, int velocity);
    void noteOff(int note);

    // Polyphony voice mgmt
    int findFreeVoice() const;

    // Optimized period calculation
    float calcPeriod(int note) const;

    float sampleRate;
    std::array<Voice, MAX_VOICES> voices;
    NoiseGenerator noiseGen;

    float pitchBend;
};