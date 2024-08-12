/*
  ==============================================================================

    SourceCode.h
    Created: 12 Aug 2024 8:34:06pm
    Author:  garam

  ==============================================================================
*/

#pragma once

class Envelope
{
public:
    float nextValue()
    {
        level *= 0.9999f;
        return level;
    }

    float level;

};