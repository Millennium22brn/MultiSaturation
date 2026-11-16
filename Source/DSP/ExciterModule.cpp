/*
    ExciterModule.cpp — Aural exciter via bandpass soft-clipping
*/

#include "ExciterModule.h"
#include <cmath>

namespace vybecode {

void ExciterModule::prepare(const ModuleSpec& spec) {
    currentSampleRate = spec.sampleRate;
    smoothedAmount.reset(spec.sampleRate, 0.05);
    smoothedAmount.setCurrentAndTargetValue(0.5f);
    smoothedMix.reset(spec.sampleRate, 0.05);
    smoothedMix.setCurrentAndTargetValue(0.5f);
    computeHPFCoeffs(3000.0f);
    reset();
}

void ExciterModule::process(juce::AudioBuffer<float>& buffer) {
    const float freq = (freqParam != nullptr) ? freqParam->load() : 3000.0f;
    const float amount = (amountParam != nullptr) ? amountParam->load() : 0.5f;
    const float mix = (mixParam != nullptr) ? mixParam->load() : 0.5f;

    smoothedAmount.setTargetValue(amount);
    smoothedMix.setTargetValue(mix);

    if (std::abs(freq - lastFreq) > 10.0f) {
        computeHPFCoeffs(freq);
        lastFreq = freq;
    }

    const int numChannels = juce::jmin(buffer.getNumChannels(), 2);
    const int numSamples = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample) {
        const float amt = smoothedAmount.getNextValue();
        const float wetDry = smoothedMix.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch) {
            float* channelData = buffer.getWritePointer(ch);
            float input = channelData[sample];

            // Isolate high-frequency content
            float highContent = processBiquad(hpfStates[ch], hpfCoeffs, input);

            // Drive the high content through soft-clipping to generate harmonics
            float driven = highContent * (1.0f + amt * 4.0f);
            float excited = std::tanh(driven) * 0.5f;

            // Mix excited harmonics back with dry signal
            channelData[sample] = input + excited * wetDry;
        }
    }
}

void ExciterModule::reset() {
    for (int ch = 0; ch < 2; ++ch)
        hpfStates[ch] = {};
}

void ExciterModule::computeHPFCoeffs(float freq) {
    const double w0 = juce::MathConstants<double>::twoPi * freq / currentSampleRate;
    const double sinW0 = std::sin(w0);
    const double cosW0 = std::cos(w0);
    const double alpha = sinW0 / (2.0 * 0.707);

    const double b0 = (1.0 + cosW0) / 2.0;
    const double b1 = -(1.0 + cosW0);
    const double b2 = (1.0 + cosW0) / 2.0;
    const double a0 = 1.0 + alpha;
    const double a1 = -2.0 * cosW0;
    const double a2 = 1.0 - alpha;

    const double invA0 = 1.0 / a0;
    hpfCoeffs.b0 = static_cast<float>(b0 * invA0);
    hpfCoeffs.b1 = static_cast<float>(b1 * invA0);
    hpfCoeffs.b2 = static_cast<float>(b2 * invA0);
    hpfCoeffs.a1 = static_cast<float>(a1 * invA0);
    hpfCoeffs.a2 = static_cast<float>(a2 * invA0);
}

float ExciterModule::processBiquad(BiquadState& state, const BiquadCoeffs& c, float input) {
    float output = c.b0 * input + c.b1 * state.x1 + c.b2 * state.x2
                   - c.a1 * state.y1 - c.a2 * state.y2;
    state.x2 = state.x1;
    state.x1 = input;
    state.y2 = state.y1;
    state.y1 = output;
    return output;
}

std::vector<std::unique_ptr<juce::RangedAudioParameter>>
ExciterModule::createParameters(const juce::String& prefix) {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "frequency", 1},
        prefix + "Frequency",
        juce::NormalisableRange<float>(1000.0f, 8000.0f, 1.0f, 0.4f),
        3000.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "amount", 1},
        prefix + "Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "mix", 1},
        prefix + "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f));

    return params;
}

void ExciterModule::updateFromAPVTS(juce::AudioProcessorValueTreeState& apvts,
                                      const juce::String& prefix) {
    freqParam = getParam(apvts, prefix, "frequency");
    amountParam = getParam(apvts, prefix, "amount");
    mixParam = getParam(apvts, prefix, "mix");
}

} // namespace vybecode
