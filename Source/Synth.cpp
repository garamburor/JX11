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
    for (int v = 0; v < MAX_VOICES; ++v) {
        voices[v].reset();
    }
    noiseGen.reset();
    pitchBend = 1.0f; // set to center pos
}

void Synth::render(float** outputBuffers, int sampleCount)
{
    float* outputBufferLeft = outputBuffers[0];
    float* outputBufferRight = outputBuffers[1];
    
    // set osc pitch
    for (int v = 0; v < MAX_VOICES; ++v) {
        Voice& voice = voices[v];
        if (voice.env.isActive()) {
            voice.osc1.period = voice.period * pitchBend;
            voice.osc2.period = voice.osc1.period * detune;
        }
    }

    for (int sample = 0; sample < sampleCount; ++sample)
    {
        // Get noise level for current sample
        float noise = noiseGen.nextValue() * noiseMix;

        // Reset output
        float outputLeft = 0.0f;
        float outputRight = 0.0f;

        for (int v = 0; v < MAX_VOICES; ++v) {
            Voice& voice = voices[v];
            if (voice.env.isActive()) {
                // Store noise value if loud enough
                float output = voice.render(noise);
                // Apply gain for each channel
                outputLeft += output * voice.panLeft;
                outputRight += output * voice.panRight;
            }
        }

        // Set output in audio buffer
        // If buffer is stereo, send stereo signal
        if (outputBufferRight != nullptr) {
            outputBufferLeft[sample] = outputLeft;
            outputBufferRight[sample] = outputRight;
        } // Otherwise send mono
        else {
            outputBufferLeft[sample] = (outputLeft +
                outputRight) * 0.5f;
        }
    }

    // If voice is silent, reset it's envelope
    for (int v = 0; v < MAX_VOICES; ++v) {
        Voice& voice = voices[v];
        if (!voice.env.isActive()) {
            voice.env.reset();
        }
    }

    protectYourEars(outputBufferLeft, sampleCount);
    protectYourEars(outputBufferRight, sampleCount);
}

int Synth::findFreeVoice() const
{
    int v = 0;
    float l = 100.0f;
    // Iterate through all voices
    for (int i = 0; i < MAX_VOICES; ++i) {
        // Compare voice level of voices that are not in attack phase
        if (voices[i].env.level < l && !voices[i].env.isInAttack()) {
            // Store level of voice
            l = voices[i].env.level;
            // & voice number
            v = i;
        }
    }
    // Return the voice number
    return v;
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

void Synth::startVoice(int v, int note, int velocity)
{
    // convert note to freq (temperament tuning)
    float period = calcPeriod(note);

    Voice& voice = voices[v];
    voice.period = period;
    voice.note = note;
    voice.updatePanning();
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

void Synth::noteOn(int note, int velocity)
{
    int v = 0;
    // Use voice mgmt is polyphony is on
    if (numVoices > 1) {
        v = findFreeVoice();
    }
    startVoice(v, note, velocity);
}

void Synth::noteOff(int note)
{
    for (int v = 0; v < MAX_VOICES; ++v) {
        if (voices[v].note == note) {
            voices[v].release();
            voices[v].note = 0;
        }
    }
}