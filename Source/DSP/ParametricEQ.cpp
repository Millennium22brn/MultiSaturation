/*
    ParametricEQ.cpp — 3-band parametric EQ (low shelf, mid peak, high shelf)
*/

#include "ParametricEQ.h"

namespace vybecode {

void ParametricEQ::prepare(const ModuleSpec& spec) {
    currentSampleRate = spec.sampleRate;

    juce::dsp::ProcessSpec pspec;
    pspec.sampleRate = spec.sampleRate;
    pspec.maximumBlockSize = static_cast<juce::uint32>(spec.maxBlockSize);
    pspec.numChannels = 1;

    for (int ch = 0; ch < maxChannels; ++ch)
        for (int band = 0; band < numBands; ++band)
            filters[ch][band].prepare(pspec);

    updateCoefficients();
}

void ParametricEQ::process(juce::AudioBuffer<float>& buffer) {
    updateCoefficients();

    const int numChannels = juce::jmin(buffer.getNumChannels(), maxChannels);
    const int numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch) {
        float* chData = buffer.getWritePointer(ch);
        for (int sample = 0; sample < numSamples; ++sample) {
            float val = chData[sample];
            for (int band = 0; band < numBands; ++band)
                val = filters[ch][band].processSample(val);
            chData[sample] = val;
        }
    }
}

void ParametricEQ::reset() {
    for (int ch = 0; ch < maxChannels; ++ch)
        for (int band = 0; band < numBands; ++band)
            filters[ch][band].reset();
}

void ParametricEQ::updateCoefficients() {
    float lowGain = (lowGainParam != nullptr) ? lowGainParam->load() : 0.0f;
    float lowFreq = (lowFreqParam != nullptr) ? lowFreqParam->load() : 80.0f;
    float midGain = (midGainParam != nullptr) ? midGainParam->load() : 0.0f;
    float midFreq = (midFreqParam != nullptr) ? midFreqParam->load() : 1000.0f;
    float midQ = (midQParam != nullptr) ? midQParam->load() : 1.0f;
    float highGain = (highGainParam != nullptr) ? highGainParam->load() : 0.0f;
    float highFreq = (highFreqParam != nullptr) ? highFreqParam->load() : 8000.0f;

    // Low shelf coefficients
    auto lowCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        currentSampleRate, lowFreq, 0.707f,
        juce::Decibels::decibelsToGain(lowGain));

    // Mid peak coefficients
    auto midCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        currentSampleRate, midFreq, midQ,
        juce::Decibels::decibelsToGain(midGain));

    // High shelf coefficients
    auto highCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        currentSampleRate, highFreq, 0.707f,
        juce::Decibels::decibelsToGain(highGain));

    for (int ch = 0; ch < maxChannels; ++ch) {
        *filters[ch][0].coefficients = *lowCoeffs;
        *filters[ch][1].coefficients = *midCoeffs;
        *filters[ch][2].coefficients = *highCoeffs;
    }
}

std::vector<std::unique_ptr<juce::RangedAudioParameter>>
ParametricEQ::createParameters(const juce::String& prefix) {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "low_gain", 1},
        prefix + "Low Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "low_freq", 1},
        prefix + "Low Freq",
        juce::NormalisableRange<float>(20.0f, 500.0f, 1.0f, 0.4f),
        80.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "mid_gain", 1},
        prefix + "Mid Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "mid_freq", 1},
        prefix + "Mid Freq",
        juce::NormalisableRange<float>(200.0f, 8000.0f, 1.0f, 0.3f),
        1000.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "mid_q", 1},
        prefix + "Mid Q",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f, 0.4f),
        1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "high_gain", 1},
        prefix + "High Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "high_freq", 1},
        prefix + "High Freq",
        juce::NormalisableRange<float>(2000.0f, 20000.0f, 1.0f, 0.3f),
        8000.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    return params;
}

void ParametricEQ::updateFromAPVTS(juce::AudioProcessorValueTreeState& apvts,
                                     const juce::String& prefix) {
    lowGainParam = getParam(apvts, prefix, "low_gain");
    lowFreqParam = getParam(apvts, prefix, "low_freq");
    midGainParam = getParam(apvts, prefix, "mid_gain");
    midFreqParam = getParam(apvts, prefix, "mid_freq");
    midQParam = getParam(apvts, prefix, "mid_q");
    highGainParam = getParam(apvts, prefix, "high_gain");
    highFreqParam = getParam(apvts, prefix, "high_freq");
}

} // namespace vybecode
