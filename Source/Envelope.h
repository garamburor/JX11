/*
  ==============================================================================

    SourceCode.h
    Created: 12 Aug 2024 8:34:06pm
    Author:  garam

  ==============================================================================
*/

#pragma once

const float SILENCE = 0.0001f; // mute threshold

class Envelope
{
public:
    float nextValue()
    {
        level = multiplier * (level - target) + target;
        return level;
    }

    float level;
    float multiplier;
    float target;
};