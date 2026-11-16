/*
    PhaseRotator.h — Multi-stage allpass phase rotator
    Category: Utility
    Parameters: stages (2/4/6/8 choice), frequency (100-2000 Hz),
                spread (0-1)

    Cascade of frequency-staggered second-order allpass stages that
    rotates phase to symmetrize waveforms — the broadcast trick for more
    headroom before clipping. Up to 8 allpass biquads with center
    frequencies geometrically staggered around frequency by spread
    (distinct from the single allpass_filter module: this is the
    multi-stage asymmetry-correction tool); magnitude-flat, phase-only
    processing; fixed pre-allocated stage array.
*/

#pragma once
#include "DSPModule.h"
#include <vector>
#include <cmath>

namespace vybecode {

class PhaseRotator : public DSPModule {
public:
    void prepare(const ModuleSpec& spec) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>>
        createParameters(const juce::String& prefix) override;
    void updateFromAPVTS(juce::AudioProcessorValueTreeState& apvts,
                          const juce::String& prefix) override;

    juce::String getModuleID() const override { return "phase_rotator"; }
    juce::String getModuleName() const override { return "Phase Rotator"; }

private:
    static constexpr int kMaxStages = 8;

    struct BiquadCoeffs {
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
    };
    struct BiquadState {
        float x1 = 0.0f, x2 = 0.0f, y1 = 0.0f, y2 = 0.0f;
    };

    void updateCoefficients(int numStages, float frequency, float spread);

    std::atomic<float>* stagesParam = nullptr;
    std::atomic<float>* frequencyParam = nullptr;
    std::atomic<float>* spreadParam = nullptr;

    double currentSampleRate = 44100.0;

    BiquadCoeffs coeffs[kMaxStages];                 // shared across channels
    std::vector<BiquadState> states;                 // [channel * kMaxStages + stage]
    int numChannels = 2;

    // Cached settings so coefficients are recomputed only on change
    int lastStages = -1;
    float lastFrequency = -1.0f;
    float lastSpread = -1.0f;
};

} // namespace vybecode
