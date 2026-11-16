/*
    MidSideProcessor.h — M/S matrix tool (surgical width and center control)
    Category: Spatial
    Parameters: mid_gain (-24 to 12 dB), side_gain (-24 to 12 dB),
                mid_tilt (-6 to 6 dB), side_highpass (20-1000 Hz),
                solo (Stereo / Mid Only / Side Only)

    Engineering-grade M/S matrix: encode M=(L+R)/sqrt2, S=(L-R)/sqrt2,
    independent gain per channel, a tilt filter on mid (shelving pair pivoting
    at 700 Hz), a 2-pole highpass on side (the mono-compatible bass mover),
    decode back. Energy-preserving sqrt2 normalization keeps unity through-pass
    bit-transparent at defaults. Complements stereo_widener (an effect) with a
    surgical tool; solo modes for auditioning.
*/

#pragma once
#include "DSPModule.h"
#include <cmath>

namespace vybecode {

class MidSideProcessor : public DSPModule {
public:
    void prepare(const ModuleSpec& spec) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>>
        createParameters(const juce::String& prefix) override;
    void updateFromAPVTS(juce::AudioProcessorValueTreeState& apvts,
                          const juce::String& prefix) override;

    juce::String getModuleID() const override { return "mid_side_processor"; }
    juce::String getModuleName() const override { return "Mid/Side Processor"; }

private:
    std::atomic<float>* midGainParam = nullptr;
    std::atomic<float>* sideGainParam = nullptr;
    std::atomic<float>* midTiltParam = nullptr;
    std::atomic<float>* sideHighpassParam = nullptr;
    std::atomic<float>* soloParam = nullptr;

    juce::SmoothedValue<float> midGainSmoother;
    juce::SmoothedValue<float> sideGainSmoother;

    double currentSampleRate = 44100.0;

    // Minimal biquad with no-allocation coefficient updates (RBJ forms).
    // juce::dsp::IIR::Coefficients::make* allocates, so coefficients are
    // computed inline here and refreshed only when a parameter moves.
    struct Biquad {
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
        float z1 = 0.0f, z2 = 0.0f;

        float processSample(float x) {
            float y = b0 * x + z1;
            z1 = b1 * x - a1 * y + z2;
            z2 = b2 * x - a2 * y;
            return y;
        }
        void clear() { z1 = z2 = 0.0f; }

        void setLowShelf(float fs, float freq, float gainDb) {
            float A = std::pow(10.0f, gainDb / 40.0f);
            float w = 2.0f * juce::MathConstants<float>::pi * freq / fs;
            float cw = std::cos(w), sw = std::sin(w);
            float alpha = sw / 2.0f * std::sqrt(2.0f);
            float sqA = 2.0f * std::sqrt(A) * alpha;
            float a0 = (A + 1.0f) + (A - 1.0f) * cw + sqA;
            b0 = (A * ((A + 1.0f) - (A - 1.0f) * cw + sqA)) / a0;
            b1 = (2.0f * A * ((A - 1.0f) - (A + 1.0f) * cw)) / a0;
            b2 = (A * ((A + 1.0f) - (A - 1.0f) * cw - sqA)) / a0;
            a1 = (-2.0f * ((A - 1.0f) + (A + 1.0f) * cw)) / a0;
            a2 = ((A + 1.0f) + (A - 1.0f) * cw - sqA) / a0;
        }
        void setHighShelf(float fs, float freq, float gainDb) {
            float A = std::pow(10.0f, gainDb / 40.0f);
            float w = 2.0f * juce::MathConstants<float>::pi * freq / fs;
            float cw = std::cos(w), sw = std::sin(w);
            float alpha = sw / 2.0f * std::sqrt(2.0f);
            float sqA = 2.0f * std::sqrt(A) * alpha;
            float a0 = (A + 1.0f) - (A - 1.0f) * cw + sqA;
            b0 = (A * ((A + 1.0f) + (A - 1.0f) * cw + sqA)) / a0;
            b1 = (-2.0f * A * ((A - 1.0f) + (A + 1.0f) * cw)) / a0;
            b2 = (A * ((A + 1.0f) + (A - 1.0f) * cw - sqA)) / a0;
            a1 = (2.0f * ((A - 1.0f) - (A + 1.0f) * cw)) / a0;
            a2 = ((A + 1.0f) - (A - 1.0f) * cw - sqA) / a0;
        }
        void setHighpass(float fs, float freq) {
            float w = 2.0f * juce::MathConstants<float>::pi * freq / fs;
            float cw = std::cos(w), sw = std::sin(w);
            float alpha = sw / std::sqrt(2.0f); // Butterworth Q
            float a0 = 1.0f + alpha;
            b0 = ((1.0f + cw) / 2.0f) / a0;
            b1 = (-(1.0f + cw)) / a0;
            b2 = b0;
            a1 = (-2.0f * cw) / a0;
            a2 = (1.0f - alpha) / a0;
        }
    };

    // Mid tilt = low shelf -t/2 + high shelf +t/2 pivoting at 700 Hz
    Biquad midTiltLow, midTiltHigh;
    Biquad sideHighpass;

    // Cached parameter values to avoid recomputing coefficients per block
    float cachedTilt = 1e9f;
    float cachedSideHp = 1e9f;
};

} // namespace vybecode
