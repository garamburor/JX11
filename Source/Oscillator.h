/*
  ==============================================================================

    Oscillator.h
    Created: 10 Mar 2024 3:04:19pm
    Author:  garam

  ==============================================================================
*/

#pragma once

const float TWO_PI = 6.2831853071795864f;

class Oscillator
{
public:
    float amplitude;
    float inc;
    float phase;

    void reset()
    {
        phase = 0.0f;
    }

    float nextSample()
    {
        phase += inc;
        if (phase >= 1.0f) {
            phase -= 1.0f;
        }

        return amplitude * (2.0f * phase - 1.0f);
    }
};