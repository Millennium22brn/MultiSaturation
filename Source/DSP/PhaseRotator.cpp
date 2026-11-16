/*
    PhaseRotator.cpp — Multi-stage allpass phase rotator
*/

#include "PhaseRotator.h"

namespace vybecode {

void PhaseRotator::prepare(const ModuleSpec& spec) {
    currentSampleRate = spec.sampleRate;
    numChannels = juce::jmax(1, spec.numChannels);

    // Fixed pre-allocated stage array: one biquad state per channel/stage.
    states.assign((size_t)(numChannels * kMaxStages), BiquadState{});

    lastStages = -1;
    lastFrequency = -1.0f;
    lastSpread = -1.0f;
}

void PhaseRotator::updateCoefficients(int numStages, float frequency, float spread) {
    const float nyquistGuard = 0.45f * (float)currentSampleRate;
    const float centerIndex = (float)(numStages - 1) * 0.5f;

    for (int i = 0; i < numStages; ++i) {
        // Geometric stagger around the center frequency: each stage is
        // offset by up to ±spread octaves across the cascade.
        const float octaveOffset = spread * ((float)i - centerIndex);
        const float fc = juce::jlimit(20.0f, nyquistGuard,
                                      frequency * std::pow(2.0f, octaveOffset));

        // RBJ second-order allpass, Q = 0.707
        const float q = 0.707f;
        const float w0 = 2.0f * juce::MathConstants<float>::pi
                       * fc / (float)currentSampleRate;
        const float cosw0 = std::cos(w0);
        const float alpha = std::sin(w0) / (2.0f * q);
        const float a0 = 1.0f + alpha;

        coeffs[i].b0 = (1.0f - alpha) / a0;
        coeffs[i].b1 = (-2.0f * cosw0) / a0;
        coeffs[i].b2 = (1.0f + alpha) / a0;
        coeffs[i].a1 = (-2.0f * cosw0) / a0;
        coeffs[i].a2 = (1.0f - alpha) / a0;
    }
}

void PhaseRotator::process(juce::AudioBuffer<float>& buffer) {
    const int stagesIdx = stagesParam ? (int)stagesParam->load() : 1;
    const int numStages = juce::jlimit(2, kMaxStages, (stagesIdx + 1) * 2);
    const float frequency = frequencyParam ? frequencyParam->load() : 400.0f;
    const float spread = spreadParam ? spreadParam->load() : 0.5f;

    if (numStages != lastStages
        || frequency != lastFrequency
        || spread != lastSpread) {
        updateCoefficients(numStages, frequency, spread);
        lastStages = numStages;
        lastFrequency = frequency;
        lastSpread = spread;
    }

    const int channels = juce::jmin(buffer.getNumChannels(), numChannels);
    const int numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < channels; ++ch) {
        float* data = buffer.getWritePointer(ch);

        for (int stage = 0; stage < numStages; ++stage) {
            const BiquadCoeffs& c = coeffs[stage];
            BiquadState& s = states[(size_t)(ch * kMaxStages + stage)];

            for (int sample = 0; sample < numSamples; ++sample) {
                const float x = data[sample];
                const float y = c.b0 * x + c.b1 * s.x1 + c.b2 * s.x2
                              - c.a1 * s.y1 - c.a2 * s.y2;
                s.x2 = s.x1; s.x1 = x;
                s.y2 = s.y1; s.y1 = y;
                data[sample] = y;
            }
        }
    }
}

void PhaseRotator::reset() {
    std::fill(states.begin(), states.end(), BiquadState{});
}

std::vector<std::unique_ptr<juce::RangedAudioParameter>>
PhaseRotator::createParameters(const juce::String& prefix) {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{prefix + "stages", 1},
        prefix + "Stages",
        juce::StringArray{"2", "4", "6", "8"},
        1));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "frequency", 1},
        prefix + "Center Freq",
        juce::NormalisableRange<float>(100.0f, 2000.0f, 1.0f, 0.5f),
        400.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "spread", 1},
        prefix + "Spread",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f));

    return params;
}

void PhaseRotator::updateFromAPVTS(juce::AudioProcessorValueTreeState& apvts,
                                     const juce::String& prefix) {
    stagesParam    = getParam(apvts, prefix, "stages");
    frequencyParam = getParam(apvts, prefix, "frequency");
    spreadParam    = getParam(apvts, prefix, "spread");
}

} // namespace vybecode
