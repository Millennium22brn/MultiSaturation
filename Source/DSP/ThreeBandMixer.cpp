/*
    ThreeBandMixer.cpp — 3-band Linkwitz-Riley split with per-band gain and solo
*/

#include "ThreeBandMixer.h"

namespace vybecode {

void ThreeBandMixer::prepare(const ModuleSpec& spec) {
    currentSampleRate = spec.sampleRate;
    numChannels = spec.numChannels;

    juce::dsp::ProcessSpec jSpec;
    jSpec.sampleRate = spec.sampleRate;
    jSpec.maximumBlockSize = static_cast<juce::uint32>(spec.maxBlockSize);
    jSpec.numChannels = static_cast<juce::uint32>(spec.numChannels);

    lowLP.prepare(jSpec);  lowLP.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    lowHP.prepare(jSpec);  lowHP.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    highLP.prepare(jSpec); highLP.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    highHP.prepare(jSpec); highHP.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    lowAP.prepare(jSpec);  lowAP.setType(juce::dsp::LinkwitzRileyFilterType::allpass);

    lowGainSmoother.reset(spec.sampleRate, 0.02);  // 20ms smoothing
    lowGainSmoother.setCurrentAndTargetValue(1.0f);
    midGainSmoother.reset(spec.sampleRate, 0.02);
    midGainSmoother.setCurrentAndTargetValue(1.0f);
    highGainSmoother.reset(spec.sampleRate, 0.02);
    highGainSmoother.setCurrentAndTargetValue(1.0f);
}

void ThreeBandMixer::process(juce::AudioBuffer<float>& buffer) {
    const int channels = juce::jmin(buffer.getNumChannels(), numChannels);
    const int numSamples = buffer.getNumSamples();

    float lowX  = (lowXoverParam != nullptr) ? lowXoverParam->load() : 200.0f;
    float highX = (highXoverParam != nullptr) ? highXoverParam->load() : 3000.0f;
    const int solo = (soloParam != nullptr) ? static_cast<int>(soloParam->load()) : 0;

    // Keep the crossovers ordered so the bands never invert
    highX = juce::jmax(highX, lowX * 1.5f);
    lowLP.setCutoffFrequency(lowX);
    lowHP.setCutoffFrequency(lowX);
    highLP.setCutoffFrequency(highX);
    highHP.setCutoffFrequency(highX);
    lowAP.setCutoffFrequency(highX);

    // Per-band gains, smoothed; solo mutes the other bands
    auto bandGain = [solo](std::atomic<float>* p, int band) {
        const float db = (p != nullptr) ? p->load() : 0.0f;
        const float g = juce::Decibels::decibelsToGain(db);
        return (solo == 0 || solo == band) ? g : 0.0f;
    };
    lowGainSmoother.setTargetValue(juce::jmax(1.0e-5f, bandGain(lowGainParam, 1)));
    midGainSmoother.setTargetValue(juce::jmax(1.0e-5f, bandGain(midGainParam, 2)));
    highGainSmoother.setTargetValue(juce::jmax(1.0e-5f, bandGain(highGainParam, 3)));

    // Split: low | (mid+high), then mid | high; allpass the low band at the
    // high crossover so all three bands stay phase-coherent and sum flat.
    for (int sample = 0; sample < numSamples; ++sample) {
        const float lg = lowGainSmoother.getNextValue();
        const float mg = midGainSmoother.getNextValue();
        const float hg = highGainSmoother.getNextValue();

        for (int ch = 0; ch < channels; ++ch) {
            const float input = buffer.getReadPointer(ch)[sample];

            float low  = lowLP.processSample(ch, input);
            low        = lowAP.processSample(ch, low);
            const float rest = lowHP.processSample(ch, input);
            const float mid  = highLP.processSample(ch, rest);
            const float high = highHP.processSample(ch, rest);

            buffer.getWritePointer(ch)[sample] = low * lg + mid * mg + high * hg;
        }
    }
}

void ThreeBandMixer::reset() {
    lowLP.reset();
    lowHP.reset();
    highLP.reset();
    highHP.reset();
    lowAP.reset();
    lowGainSmoother.setCurrentAndTargetValue(1.0f);
    midGainSmoother.setCurrentAndTargetValue(1.0f);
    highGainSmoother.setCurrentAndTargetValue(1.0f);
}

std::vector<std::unique_ptr<juce::RangedAudioParameter>>
ThreeBandMixer::createParameters(const juce::String& prefix) {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "low_xover", 1},
        prefix + "Low Crossover",
        juce::NormalisableRange<float>(40.0f, 1000.0f, 0.1f, 0.35f),
        200.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "high_xover", 1},
        prefix + "High Crossover",
        juce::NormalisableRange<float>(1000.0f, 12000.0f, 0.1f, 0.35f),
        3000.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "low_gain", 1},
        prefix + "Low Gain",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "mid_gain", 1},
        prefix + "Mid Gain",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "high_gain", 1},
        prefix + "High Gain",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{prefix + "solo", 1},
        prefix + "Solo",
        juce::StringArray{"Off", "Low", "Mid", "High"},
        0));

    return params;
}

void ThreeBandMixer::updateFromAPVTS(juce::AudioProcessorValueTreeState& apvts,
                                       const juce::String& prefix) {
    lowXoverParam  = getParam(apvts, prefix, "low_xover");
    highXoverParam = getParam(apvts, prefix, "high_xover");
    lowGainParam   = getParam(apvts, prefix, "low_gain");
    midGainParam   = getParam(apvts, prefix, "mid_gain");
    highGainParam  = getParam(apvts, prefix, "high_gain");
    soloParam      = getParam(apvts, prefix, "solo");
}

} // namespace vybecode
