/*
    LFO.h — Low Frequency Oscillator for modulation
    Category: Modulation
    Parameters: rate (0.01-20 Hz), depth (0-1), shape (sine/triangle/saw/square)

    Output is stored internally and can be read via getCurrentValue().
    process() modulates the buffer amplitude by the LFO (tremolo-style).
*/

#pragma once
#include "DSPModule.h"
#include <cmath>

namespace vybecode {

class LFO : public DSPModule {
public:
    void prepare(const ModuleSpec& spec) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>>
        createParameters(const juce::String& prefix) override;
    void updateFromAPVTS(juce::AudioProcessorValueTreeState& apvts,
                          const juce::String& prefix) override;

    juce::String getModuleID() const override { return "lfo"; }
    juce::String getModuleName() const override { return "LFO"; }

    // Read current LFO value (BIPOLAR, -1 to 1) for external modulation routing (GL-81a) —
    // so depth*value sweeps symmetrically. The in-chain tremolo (process) stays unipolar.
    float getCurrentValue() const { return currentValue; }

private:
    std::atomic<float>* rateParam = nullptr;
    std::atomic<float>* depthParam = nullptr;
    std::atomic<float>* shapeParam = nullptr;  // 0=sine, 1=tri, 2=saw, 3=square

    double phase = 0.0;
    double sampleRate = 44100.0;
    float currentValue = 0.0f;

    float computeLFOSample(double ph, int shape) const;
};

} // namespace vybecode
