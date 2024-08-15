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

        // Switch from attack to decay  & sustain values
        if (level + target > 3.0f) {
            multiplier = decayMultiplier;
            target = sustainLevel;
        }

        return level;
    }
    
    // Return target if it's still in attack phase
    inline bool isInAttack() const
    {
        return target >= 2.0f;
    }

    void attack()
    {
        // Start envelope above threshold so it's not muted
        level += SILENCE + SILENCE;
        // Target to end attack phase
        target = 2.0f;
        // Set time constant
        multiplier = attackMultiplier;
    }

    void release()
    {
        target = 0.0f;
        multiplier = releaseMultiplier;
    }

    // mute envelope if below SILENCE const
    inline bool isActive() const
    {
        return level > SILENCE;
    }

    void reset()
    {
        level = 0.0f;
        target = 0.0f;
        multiplier = 0.0f;
    }

    float level;

    float attackMultiplier;
    float decayMultiplier;
    float sustainLevel;
    float releaseMultiplier;

private:
    float multiplier;
    float target;

};