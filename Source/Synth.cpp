/*
  ==============================================================================

    Synth.cpp
    Created: 1 Mar 2024 2:53:03pm
    Author:  garam

  ==============================================================================
*/

#include "Synth.h"
#include "Utils.h"

Synth::Synth()
{
    sampleRate = 44100.0f;
}

void Synth::allocateResources(double sampleRate_, int /*samplesPerBlock*/)
{
    sampleRate = static_cast<float>(sampleRate_);
}

void Synth::deallocateResources()
{
    // do nothing yet
}

void Synth::reset()
{
    voice.reset();
    noiseGen.reset();
    pitchBend = 1.0f; // set to center pos
}

void Synth::render(float** outputBuffers, int sampleCount)
{
    float* outputBufferLeft = outputBuffers[0];
    float* outputBufferRight = outputBuffers[1];
    
    // set osc pitch
    voice.osc1.period = voice.period * pitchBend;
    voice.osc2.period = voice.osc1.period * detune;

    for (int sample = 0; sample < sampleCount; ++sample)
    {
        // Get noise level for current sample
        float noise = noiseGen.nextValue() * noiseMix;

        // Reset output
        float output = 0.0f;
        if (voice.env.isActive()) {
            // Store noise value if loud enough
            output = voice.render(noise);
        }

        // Set output in audio buffer
        outputBufferLeft[sample] = output;
        if (outputBufferRight != nullptr) {
            outputBufferRight[sample] = output;
        }
    }

    // If voice is silent, reset it's envelope
    if (!voice.env.isActive()) {
        voice.env.reset();
    }

    protectYourEars(outputBufferLeft, sampleCount);
    protectYourEars(outputBufferRight, sampleCount);
}

float Synth::calcPeriod(int note) const
{
    // Optimized formula for (sampleRate / freq):
    // sampleRate / (440.0f * std::exp2((float(note - 69) + tune) / 12.0f));
    float period = tune * std::exp(-0.05776226505f *
        float(note));

    // Set limit for highest pitch to avoid BLIT crapping out
    while (period < 6.0f || (period * detune) < 6.0f) {
        period += period;
    }
    return period;
}

void Synth::midiMessage(uint8_t data0, uint8_t data1, uint8_t data2)
{
    switch (data0 & 0xF0) {
        // Note off
    case 0x80:
        noteOff(data1 & 0x7F);
        break;

    // Note on (brackets needed or error)
    case 0x90: {
        uint8_t note = data1 & 0x7F;
        uint8_t velo = data2 & 0x7F;
        if (velo > 0) {
            noteOn(note, velo);
        }
        else {
            noteOff(note);
        }
        break;
    }

    // Pitch bend
    case 0xE0:
        // float(data1&2intoNumber)
        // numer corresponds to 2^(-2*(number/8192)/12
        pitchBend = std::exp(-0.000014102f * float(data1 +
            128 * data2 - 8192));
        break;
    }
}

void Synth::noteOn(int note, int velocity)
{
    voice.note = note;
    // convert note to freq (temperament tuning)
    voice.period = calcPeriod(note);
    // activate the first osc
    voice.osc1.amplitude = (velocity / 127.0f) * 0.5f;
    // voice.osc1.reset(); // reset restarts the phase, so it can sync oscs
    // activate the second osc
    voice.osc2.amplitude = voice.osc1.amplitude * oscMix;
    // voice.osc2.reset(); 

    // Envelope
    Envelope& env = voice.env;
    env.attackMultiplier = envAttack;
    env.decayMultiplier = envDecay;
    env.sustainLevel = envSustain;
    env.releaseMultiplier = envRelease;
    env.attack();
}

void Synth::noteOff(int note)
{
    if (voice.note == note) {
        voice.release();
    }
}