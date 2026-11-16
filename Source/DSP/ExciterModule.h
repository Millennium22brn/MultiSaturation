/*
    ExciterModule.h — Aural exciter via bandpass soft-clipping
    Category: Psychoacoustic
    Parameters: frequency (1000-8000 Hz crossover), amount (0-1), mix (0-1)

    Isolates high-frequency content with a HPF, applies soft-clipping
    to generate new harmonics above the crossover, then mixes the
    enriched signal back. Adds "air" and presence to dull recordings.
*/

#pragma once
#include "DSPModule.h"

namespace vybecode {

class ExciterModule : public DSPModule {
public:
    void prepare(const ModuleSpec& spec) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>>
        createParameters(const juce::String& prefix) override;
    void updateFromAPVTS(juce::AudioProcessorValueTreeState& apvts,
                          const juce::String& prefix) override;

    juce::String getModuleID() const override { return "exciter"; }
    juce::String getModuleName() const override { return "Exciter"; }

private:
    struct BiquadState {
        float x1 = 0.0f, x2 = 0.0f;
        float y1 = 0.0f, y2 = 0.0f;
    };

    struct BiquadCoeffs {
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f;
    };

    // HPF to isolate high-frequency content
    BiquadState hpfStates[2];
    BiquadCoeffs hpfCoeffs;

    std::atomic<float>* freqParam = nullptr;
    std::atomic<float>* amountParam = nullptr;
    std::atomic<float>* mixParam = nullptr;

    juce::SmoothedValue<float> smoothedAmount;
    juce::SmoothedValue<float> smoothedMix;

    float lastFreq = 3000.0f;
    double currentSampleRate = 44100.0;

    void computeHPFCoeffs(float freq);
    float processBiquad(BiquadState& state, const BiquadCoeffs& coeffs, float input);
};

} // namespace vybecode
