/*
  ==============================================================================

    Voice.h
    Created: 1 Mar 2024 2:53:12pm
    Author:  garam

  ==============================================================================
*/

#pragma once
#include "Oscillator.h"
#include "Envelope.h"

struct Voice
{
    Oscillator osc1;
    Oscillator osc2;
    Envelope env;

    int note;
    float saw;
    float period;

    float panLeft, panRight;

    // glide
    float target;
    float glideRate;

    void reset()
    {
        note = 0;
        osc1.reset();
        osc2.reset();
        saw = 0.0f;
        env.reset();

        panLeft = 0.707f; // - 3dB
        panRight = 0.707f;
    }

    void release()
    {
        env.release();
    }

    float render(float input)
    {
        // advance oscillators
        float sample1 = osc1.nextSample();
        float sample2 = osc2.nextSample();

        saw = saw * 0.997f + sample1 - sample2; // apply one-pole LP filter

        // sum input
        float output = saw + input;

        // advance envelope
        float envelope = env.nextValue();

        // apply envelope
        return output * envelope;
    }

    void updatePanning()
    {
        // Determine panning based on pitch. lower -> left, high -> right
        float panning = std::clamp((note - 60.0f) /
            24.0f, -1.0f, 1.0f);
        // Constant power panning
        panLeft = std::sin(PI_OVER_4 * (1.0f -
            panning));
        panRight = std::sin(PI_OVER_4 * (1.0f +
            panning));
    }

    void updateLFO()
    {
        // one pole to reach pitch target
        period += glideRate * (target - period);
    }
};