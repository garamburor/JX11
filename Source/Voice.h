/*
  ==============================================================================

    Voice.h
    Created: 1 Mar 2024 2:53:12pm
    Author:  garam

  ==============================================================================
*/

#pragma once
#include "Oscillator.h"

struct Voice
{
    Oscillator osc;

    int note;

    void reset()
    {
        note = 0;
        osc.reset();
    }

    float render()
    {
        return osc.nextSample();
    }
};