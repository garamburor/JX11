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
    Oscillator osc;
    Envelope env;

    int note;
    float saw;

    void reset()
    {
        note = 0;
        osc.reset();
        saw = 0.0f;
        env.reset();
    }

    void release()
    {
        env.release();
    }

    float render(float input)
    {
        float sample = osc.nextSample();
        saw = saw * 0.997f + sample; // apply one-pole LP filter

        // sum input
        float output = saw + input;

        float envelope = env.nextValue();

        return output * envelope;
    }
};