/*
  ==============================================================================

    Oscillator.h
    Created: 10 Mar 2024 3:04:19pm
    Author:  garam

  ==============================================================================
*/

#pragma once


const float PI_OVER_4 = 0.7853981633974483f;
const float PI = 3.1415926535897932f;
const float TWO_PI = 6.2831853071795864f;

class Oscillator
{
public:
    float period = 0.0f;
    float amplitude;

    void reset()
    {
        inc = 0.0f;
        phase = 0.0f;
    }

    float nextSample()
    {
        float output = 0.0f;

        phase += inc; // 1
        if (phase <= PI_OVER_4) { // 2
            // 3
            float halfPeriod = period * 0.5f;
            phaseMax = std::floor(0.5f + halfPeriod) - 0.5f;
            phaseMax *= PI;

            inc = phaseMax / halfPeriod;
            phase = -phase;

            // 4
            if (phase * phase > 1e-9) {
                output = amplitude * std::sin(phase) / phase;
            }
            else {
                output = amplitude;
            }
        }
        else { // 5
            // 6
            if (phase > phaseMax) {
                phase = phaseMax + phaseMax - phase;
                inc = -inc;
            }
            // 7
            output = amplitude * std::sin(phase);
        }

        return output;
    }
private:
    float phase;
    float phaseMax;
    float inc;
};