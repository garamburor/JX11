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

    void reset()
    {
        note = 0;
        osc1.reset();
        osc2.reset();
        saw = 0.0f;
        env.reset();
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
};