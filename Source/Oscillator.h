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

    // for BL saw
    float freq;
    float sampleRate;
    float phaseBL;

    void reset()
    {
        phase = 0.0f;
        phaseBL = -0.5f;
    }

    float nextBandLimitedSample()
    {
        phaseBL += inc;
        if (phaseBL >= 1.0f) {
            phaseBL -= 1.0f;
        }

        float output = 0.0f;
        float nyquist = sampleRate * 0.5f;
        float h = freq;
        float i = 1.0f;
        float m = 0.6366197724f; // 2 / pi
        while (h < nyquist) { // sum harmonics
            output += m * std::sin(TWO_PI * phaseBL * i) / i;
            h += freq;
            i += 1.0f;
            m = -m;
        }

        return output;
    }

    float nextSample()
    {
        return amplitude * nextBandLimitedSample();
    }
};