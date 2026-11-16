/*
    OpticalCompressor.h — Opto/LA-2A style compressor with slow,
    program-dependent response. RMS detection. Release time depends
    on how long signal was above threshold.
    Category: Dynamics
    Parameters: threshold_db (-40 to 0), peak_reduction (0-100),
                gain (0-24 dB), mode (compress/limit)
*/

#pragma once
#include "DSPModule.h"

namespace vybecode {

class OpticalCompressorModule : public DSPModule {
public:
    void prepare(const ModuleSpec& spec) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>>
        createParameters(const juce::String& prefix) override;
    void updateFromAPVTS(juce::AudioProcessorValueTreeState& apvts,
                          const juce::String& prefix) override;

    juce::String getModuleID() const override { return "optical_compressor"; }
    juce::String getModuleName() const override { return "Optical Compressor"; }

private:
    std::atomic<float>* thresholdParam = nullptr;
    std::atomic<float>* peakReductionParam = nullptr;
    std::atomic<float>* gainParam = nullptr;
    std::atomic<float>* modeParam = nullptr;

    double sampleRate = 44100.0;
    float rmsLevel = 0.0f;
    float gainReductionDb = 0.0f;
    float timeAboveThreshold = 0.0f; // seconds signal has been above threshold

    juce::SmoothedValue<float> gainSmoother;
};

} // namespace vybecode
