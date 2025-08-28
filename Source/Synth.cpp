/*
  ==============================================================================

    Synth.cpp
    Created: 1 Mar 2024 2:53:03pm
    Author:  garam

  ==============================================================================
*/

#include "Synth.h"
#include "Utils.h"

// Detuning factor between voices
static const float ANALOG = 0.002f;
static const int SUSTAIN = -1;

Synth::Synth()
{
    sampleRate = 44100.0f;
}

void Synth::allocateResources(double sampleRate_, int /*samplesPerBlock*/)
{
    sampleRate = static_cast<float>(sampleRate_);

    // Set filter's samplerate
    for (int v = 0; v < MAX_VOICES; ++v)
    {
        voices[v].filter.sampleRate = sampleRate;
    }
}

void Synth::deallocateResources()
{
    // do nothing yet
}

void Synth::reset()
{
    for (int v = 0; v < MAX_VOICES; ++v) {
        voices[v].reset();
    }
    noiseGen.reset();
    pitchBend = 1.0f; // set to center pos
    // Set inital value of sustain pedal
    sustainPedalPressed = false;
    // Set sample rate and time constant for one pole
    outputLevelSmoother.reset(sampleRate, 0.05);
    // Reset LFO
    lfo = 0.0f;
    lfoStep = 0;
    // Reset modwheel
    modWheel = 0.0f;
    // Glide
    lastNote = 0;
    // Filter
    resonanceCtl = 1.0f;
    pressure = 0.0f;
}

void Synth::render(float** outputBuffers, int sampleCount)
{
    float* outputBufferLeft = outputBuffers[0];
    float* outputBufferRight = outputBuffers[1];
    
    // set osc pitch
    for (int v = 0; v < MAX_VOICES; ++v) {
        // Render voices
        Voice& voice = voices[v];
        if (voice.env.isActive()) {
            updatePeriod(voice);
            voice.glideRate = glideRate;
            voice.filterQ = filterQ * resonanceCtl;
        }
    }

    for (int sample = 0; sample < sampleCount; ++sample)
    {
        // advance LFO phasor
        updateLFO();

        // Get noise level for current sample
        float noise = noiseGen.nextValue() * noiseMix;

        // Reset output
        float outputLeft = 0.0f;
        float outputRight = 0.0f;

        for (int v = 0; v < MAX_VOICES; ++v) {
            Voice& voice = voices[v];
            if (voice.env.isActive()) {
                // Store noise value if loud enough
                float output = voice.render(noise);
                // Apply gain for each channel
                outputLeft += output * voice.panLeft;
                outputRight += output * voice.panRight;
            }
        }

        // Apply output level
        float outputLevel = outputLevelSmoother.getNextValue();
        outputLeft *= outputLevel;
        outputRight *= outputLevel;

        // Set output in audio buffer
        // If buffer is stereo, send stereo signal
        if (outputBufferRight != nullptr) {
            outputBufferLeft[sample] = outputLeft;
            outputBufferRight[sample] = outputRight;
        } // Otherwise send mono
        else {
            outputBufferLeft[sample] = (outputLeft +
                outputRight) * 0.5f;
        }
    }

    // If voice is silent, reset it's envelope
    for (int v = 0; v < MAX_VOICES; ++v) {
        Voice& voice = voices[v];
        if (!voice.env.isActive()) {
            voice.env.reset();
            voice.filter.reset();
        }
    }

    protectYourEars(outputBufferLeft, sampleCount);
    protectYourEars(outputBufferRight, sampleCount);
}

int Synth::findFreeVoice() const
{
    int v = 0;
    float l = 100.0f;
    // Iterate through all voices
    for (int i = 0; i < MAX_VOICES; ++i) {
        // Compare voice level of voices that are not in attack phase
        if (voices[i].env.level < l && !voices[i].env.isInAttack()) {
            // Store level of voice
            l = voices[i].env.level;
            // & voice number
            v = i;
        }
    }
    // Return the voice number
    return v;
}

float Synth::calcPeriod(int v, int note) const
{
    // Optimized formula for (sampleRate / freq):
    // sampleRate / (440.0f * std::exp2((float(note - 69) + tune) / 12.0f));
    float period = tune * std::exp(-0.05776226505f *
        float(note) + ANALOG * float(v));

    // Set limit for highest pitch to avoid BLIT crapping out
    while (period < 6.0f || (period * detune) < 6.0f) {
        period += period;
    }
    return period;
}

void Synth::midiMessage(uint8_t data0, uint8_t data1, uint8_t data2)
{
    switch (data0 & 0xF0) {
        // Note off
        case 0x80:
            noteOff(data1 & 0x7F);
            break;

        // Note on (brackets needed or error)
        case 0x90: {
            uint8_t note = data1 & 0x7F;
            uint8_t velo = data2 & 0x7F;
            if (velo > 0) {
                noteOn(note, velo);
            }
            else {
                noteOff(note);
            }
            break;
        }

        // Pitch bend
        case 0xE0:
            // float(data1&2intoNumber)
            // numer corresponds to 2^(-2*(number/8192)/12
            pitchBend = std::exp(-0.000014102f * float(data1 +
                128 * data2 - 8192));
            break;
        // Control change
        case 0xB0:
            controlChange(data1, data2);
            break;
        // Channel aftertouch
        case 0xD0:
            pressure = 0.0001f * float(data1 * data1);
            break;
    }
}

void Synth::startVoice(int v, int note, int velocity)
{
    // convert note to freq (temperament tuning)
    float period = calcPeriod(v, note);

    Voice& voice = voices[v];
    voice.target = period; // Set desired period

    int noteDistance = 0;
    // Calc distance only if there's a previous note played
    if (lastNote > 0) {
        if ((glideMode == 2) || ((glideMode == 1) && isPlayingLegatoStyle())) {
            noteDistance = note - lastNote;
        }
    }
    // set period to glide from
    voice.period = period * std::pow(1.059463094359f, float(noteDistance) - glideBend);
    // Limit voice period value
    if (voice.period < 6.0f) { voice.period = 6.0f;  }

    // Update last note and current note
    lastNote = note;
    voice.note = note;
    voice.updatePanning();

    // Velocity curve
    float vel = 0.004f * float((velocity + 64) * (velocity + 64)) - 8.0f;
    // activate the first osc
    voice.osc1.amplitude = volumeTrim * vel; //  (velocity / 127.0f) * 0.5f;
    // voice.osc1.reset(); // reset restarts the phase, so it can sync oscs
    // activate the second osc
    voice.osc2.amplitude = voice.osc1.amplitude * oscMix;
    // voice.osc2.reset(); 
    voice.cutoff = sampleRate / (period * PI); //set filter's cutoff related to note
    voice.cutoff = std::exp(velocitySensitivity * float(velocity - 64));

    // Modulation
    if (vibrato == 0.0f && pwmDepth > 0.0f) {
        voice.osc2.squareWave(voice.osc1, voice.period);
    }

    // Envelope
    Envelope& env = voice.env;
    env.attackMultiplier = envAttack;
    env.decayMultiplier = envDecay;
    env.sustainLevel = envSustain;
    env.releaseMultiplier = envRelease;
    env.attack();
}

void Synth::restartMonoVoice(int note, int velocity)
{
    // Calculate period
    float period = calcPeriod(0, note);
    // Assign values to voice 0
    Voice& voice = voices[0];
    voice.target = period; // if glide is set
    // If no glide is set
    if (glideMode == 0) { voice.period = period; }
    // Set level above threshold so it's not muted
    voice.env.level += SILENCE + SILENCE;
    voice.note = note;
    voice.updatePanning();
}

void Synth::shiftQueuedNotes()
{
    for (int tmp = MAX_VOICES - 1; tmp > 0; tmp--) {
        // Trigger release of voice
        voices[tmp].release();
        voices[tmp].note = voices[tmp - 1].note;
    }
}

int Synth::nextQueuedNote()
{
    int held = 0;
    for (int v = MAX_VOICES - 1; v > 0; v--) {
        if (voices[v].note > 0) { held = v; }
    }

    if (held > 0) {
        int note = voices[held].note;
        voices[held].note = 0;
        return note;
    }

    return 0;
}

void Synth::noteOn(int note, int velocity)
{
    if (ignoreVelocity) { velocity = 80; }

    int v = 0;
    // If monophonic
    if (numVoices == 1) {
        if (voices[0].note > 0) {
            // Shift voices
            shiftQueuedNotes();
            // Retrigger voice 0
            restartMonoVoice(note, velocity); // Legato
            return;
        }
    }
    else { // Polyphonic
        v = findFreeVoice();
    }
    startVoice(v, note, velocity);
}

void Synth::noteOff(int note)
{
    if ((numVoices == 1) && (voices[0].note == note)) {
        int queuedNote = nextQueuedNote();
        if (queuedNote > 0) {
            restartMonoVoice(queuedNote, -1);
        }
    }

    for (int v = 0; v < MAX_VOICES; v++) {
        if (voices[v].note == note) {
            if (sustainPedalPressed) {
                voices[v].note = SUSTAIN;
            } else {
                voices[v].release();
                voices[v].note = 0;
            }
        }
    }
}

void Synth::controlChange(uint8_t data1, uint8_t data2)
{
    switch (data1) {
        // Sustain pedal
        case 0x40:
            sustainPedalPressed = (data2 >= 64);
            // If off, mute sustained voices
            if (!sustainPedalPressed) {
                noteOff(SUSTAIN);
            }
            break;
        // Mod Wheel
        case 0x01:
            modWheel = 0.000005f * float(data2 * data2);
            break;
        // Unaccounted messages reset synth voices.
        default:
            if (data1 >= 0x78) {
                for (int v = 0; v < MAX_VOICES; ++v) {
                    voices[v].reset();
                }
                sustainPedalPressed = false;
            }
            break;
        // Filter resonance
        case 0x47:
            resonanceCtl = 154.0f / float(154 - data2);
            break;
    }
}

void Synth::updateLFO()
{
    // Condition to run in lower sample rate
    if (--lfoStep <= 0) {
        lfoStep = LFO_MAX;

        lfo += lfoInc; // Increment phasor
        if (lfo > PI) { lfo -= TWO_PI; } // Reset phasor if out of bounds

        const float sine = std::sin(lfo); // Get sine from phasor

        // Set modulation for pitch
        float vibratoMod = 1.0f + sine * (modWheel + vibrato);
        float pwm = 1.0f + sine * (modWheel + pwmDepth);

        float filterMod = filterKeyTracking + (filterLFODepth + pressure) * sine;

        // add mod to each voice
        for (int v = 0; v < MAX_VOICES; ++v) {
            Voice& voice = voices[v];
            if (voice.env.isActive()) {
                voice.osc1.modulation = vibratoMod;
                voice.osc2.modulation = pwm;
                voice.filterMod = filterMod;
                // Glide
                voice.updateLFO();
                updatePeriod(voice);
            }
        }
    }
}

// Check voices that are still playing
bool Synth::isPlayingLegatoStyle() const
{
    int held = 0;
    for (int i = 0; i < MAX_VOICES; ++i) {
        if (voices[i].note > 0) { held += 1; }
    }
    return held > 0;
}