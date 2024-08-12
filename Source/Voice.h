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
    }

    float render()
    {
        float sample = osc.nextSample();
        saw = saw * 0.997f + sample; // apply one-pole LP filter

        float envelope = env.nextValue();

        return saw * envelope;
    }
};