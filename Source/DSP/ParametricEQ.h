/*
    ParametricEQ.h — 3-band parametric EQ (low shelf, mid peak, high shelf)
    Category: Effects
    Parameters: low_gain, low_freq, mid_gain, mid_freq, mid_q, high_gain, high_freq
*/

#pragma once
#include "DSPModule.h"

namespace vybecode {

class ParametricEQ : public DSPModule {
public:
    void prepare(const ModuleSpec& spec) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>>
        createParameters(const juce::String& prefix) override;
    void updateFromAPVTS(juce::AudioProcessorValueTreeState& apvts,
                          const juce::String& prefix) override;

    juce::String getModuleID() const override { return "parametric_eq"; }
    juce::String getModuleName() const override { return "Parametric EQ"; }

private:
    std::atomic<float>* lowGainParam = nullptr;
    std::atomic<float>* lowFreqParam = nullptr;
    std::atomic<float>* midGainParam = nullptr;
    std::atomic<float>* midFreqParam = nullptr;
    std::atomic<float>* midQParam = nullptr;
    std::atomic<float>* highGainParam = nullptr;
    std::atomic<float>* highFreqParam = nullptr;

    // Per-channel filters: [channel][band]
    // Band 0 = low shelf, 1 = mid peak, 2 = high shelf
    static constexpr int maxChannels = 2;
    static constexpr int numBands = 3;
    juce::dsp::IIR::Filter<float> filters[maxChannels][numBands];

    double currentSampleRate = 44100.0;

    void updateCoefficients();
};

} // namespace vybecode
