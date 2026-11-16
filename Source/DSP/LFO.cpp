/*
    LFO.cpp — Low Frequency Oscillator for modulation
*/

#include "LFO.h"

namespace vybecode {

void LFO::prepare(const ModuleSpec& spec) {
    sampleRate = spec.sampleRate;
    phase = 0.0;
    currentValue = 0.0f;
}

void LFO::process(juce::AudioBuffer<float>& buffer) {
    const float rate = rateParam ? rateParam->load() : 1.0f;
    const float depth = depthParam ? depthParam->load() : 0.5f;
    const int shape = shapeParam ? static_cast<int>(shapeParam->load()) : 0;

    const double phaseInc = rate / sampleRate;
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample) {
        float lfoValue = computeLFOSample(phase, shape);
        // GL-81a: store a BIPOLAR value (-1..1) so this LFO works as a modulation SOURCE —
        // depth * getCurrentValue() must swing BOTH sides of the base, not only upward. The
        // in-chain tremolo (modGain below) keeps using the unipolar 0..1 lfoValue.
        currentValue = 2.0f * lfoValue - 1.0f;

        // Apply as amplitude modulation: signal *= (1 - depth) + depth * lfoValue
        float modGain = (1.0f - depth) + depth * lfoValue;

        for (int ch = 0; ch < numChannels; ++ch)
            buffer.getWritePointer(ch)[sample] *= modGain;

        phase += phaseInc;
        if (phase >= 1.0)
            phase -= 1.0;
    }
}

void LFO::reset() {
    phase = 0.0;
    currentValue = 0.0f;
}

float LFO::computeLFOSample(double ph, int shape) const {
    // All shapes output 0..1 range
    switch (shape) {
        case 0: // Sine (0 to 1)
            return 0.5f + 0.5f * std::sin(2.0f * juce::MathConstants<float>::pi * static_cast<float>(ph));

        case 1: // Triangle
            return (ph < 0.5) ? static_cast<float>(ph * 2.0)
                              : static_cast<float>(2.0 - ph * 2.0);

        case 2: // Saw (rising)
            return static_cast<float>(ph);

        case 3: // Square
            return (ph < 0.5f) ? 1.0f : 0.0f;

        default:
            return 0.5f;
    }
}

std::vector<std::unique_ptr<juce::RangedAudioParameter>>
LFO::createParameters(const juce::String& prefix) {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "rate", 1},
        prefix + "Rate",
        juce::NormalisableRange<float>(0.01f, 20.0f, 0.01f, 0.4f),
        1.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "depth", 1},
        prefix + "Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{prefix + "shape", 1},
        prefix + "Shape",
        juce::StringArray{"Sine", "Triangle", "Saw", "Square"},
        0));

    return params;
}

void LFO::updateFromAPVTS(juce::AudioProcessorValueTreeState& apvts,
                            const juce::String& prefix) {
    rateParam = getParam(apvts, prefix, "rate");
    depthParam = getParam(apvts, prefix, "depth");
    shapeParam = getParam(apvts, prefix, "shape");
}

} // namespace vybecode
