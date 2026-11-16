/*
    ThreeBandMixer.h — 3-band Linkwitz-Riley split with per-band gain and solo
    Category: Filters
    Parameters: low_xover (40-1000 Hz), high_xover (1000-12000 Hz),
                low_gain / mid_gain / high_gain (-24 to +12 dB), solo (choice)
*/

#pragma once
#include "DSPModule.h"

namespace vybecode {

class ThreeBandMixer : public DSPModule {
public:
    void prepare(const ModuleSpec& spec) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>>
        createParameters(const juce::String& prefix) override;
    void updateFromAPVTS(juce::AudioProcessorValueTreeState& apvts,
                          const juce::String& prefix) override;

    juce::String getModuleID() const override { return "three_band_mixer"; }
    juce::String getModuleName() const override { return "3-Band Split Mixer"; }

private:
    std::atomic<float>* lowXoverParam = nullptr;
    std::atomic<float>* highXoverParam = nullptr;
    std::atomic<float>* lowGainParam = nullptr;
    std::atomic<float>* midGainParam = nullptr;
    std::atomic<float>* highGainParam = nullptr;
    std::atomic<float>* soloParam = nullptr;

    // 4th-order Linkwitz-Riley split, standard allpass-compensated 3-way:
    // low = LP(lowX) then AP(highX); mid = HP(lowX)·LP(highX); high = HP(lowX)·HP(highX)
    juce::dsp::LinkwitzRileyFilter<float> lowLP, lowHP;     // split at low_xover
    juce::dsp::LinkwitzRileyFilter<float> highLP, highHP;   // split at high_xover
    juce::dsp::LinkwitzRileyFilter<float> lowAP;            // phase-aligns the low band

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> lowGainSmoother;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> midGainSmoother;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> highGainSmoother;
    double currentSampleRate = 44100.0;
    int numChannels = 2;
};

} // namespace vybecode
