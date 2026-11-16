/*
    MidSideProcessor.cpp — M/S matrix tool (surgical width and center control)
*/

#include "MidSideProcessor.h"

namespace vybecode {

void MidSideProcessor::prepare(const ModuleSpec& spec) {
    currentSampleRate = spec.sampleRate;

    midGainSmoother.reset(spec.sampleRate, 0.02);
    midGainSmoother.setCurrentAndTargetValue(1.0f);
    sideGainSmoother.reset(spec.sampleRate, 0.02);
    sideGainSmoother.setCurrentAndTargetValue(1.0f);

    cachedTilt = 1e9f;
    cachedSideHp = 1e9f;
    midTiltLow.clear();
    midTiltHigh.clear();
    sideHighpass.clear();
}

void MidSideProcessor::process(juce::AudioBuffer<float>& buffer) {
    const float midGainDb  = midGainParam      ? midGainParam->load()      : 0.0f;
    const float sideGainDb = sideGainParam     ? sideGainParam->load()     : 0.0f;
    const float tilt       = midTiltParam      ? midTiltParam->load()      : 0.0f;
    const float sideHpHz   = sideHighpassParam ? sideHighpassParam->load() : 20.0f;
    const int   solo       = soloParam ? juce::jlimit(0, 2,
                                 static_cast<int>(soloParam->load())) : 0;

    midGainSmoother.setTargetValue(juce::Decibels::decibelsToGain(midGainDb));
    sideGainSmoother.setTargetValue(juce::Decibels::decibelsToGain(sideGainDb));

    const float fs = static_cast<float>(currentSampleRate);

    // Refresh coefficients only when the knobs actually moved (no allocation)
    if (tilt != cachedTilt) {
        midTiltLow.setLowShelf(fs, 700.0f, -tilt * 0.5f);
        midTiltHigh.setHighShelf(fs, 700.0f, tilt * 0.5f);
        cachedTilt = tilt;
    }
    if (sideHpHz != cachedSideHp) {
        sideHighpass.setHighpass(fs, sideHpHz);
        cachedSideHp = sideHpHz;
    }

    // The tilt/HP sections do real work only when engaged; at defaults the
    // matrix is encode -> unity -> decode, which folds back bit-transparent
    // thanks to the energy-preserving 1/sqrt2 normalization.
    const bool tiltActive = std::abs(tilt) > 0.001f;
    const bool hpActive = sideHpHz > 20.5f;

    constexpr float invSqrt2 = 0.70710678f;

    const int numSamples = buffer.getNumSamples();
    if (buffer.getNumChannels() < 2)
        return; // M/S needs a stereo bus; mono passes through untouched

    float* left  = buffer.getWritePointer(0);
    float* right = buffer.getWritePointer(1);

    for (int sample = 0; sample < numSamples; ++sample) {
        float midGain  = midGainSmoother.getNextValue();
        float sideGain = sideGainSmoother.getNextValue();

        // Encode
        float mid  = (left[sample] + right[sample]) * invSqrt2;
        float side = (left[sample] - right[sample]) * invSqrt2;

        // Mid: gain + tilt (shelving pair pivoting at 700 Hz)
        mid *= midGain;
        if (tiltActive) {
            mid = midTiltLow.processSample(mid);
            mid = midTiltHigh.processSample(mid);
        }

        // Side: gain + 2-pole low cut — moves the bass into the mono center
        side *= sideGain;
        if (hpActive)
            side = sideHighpass.processSample(side);

        // Solo monitoring
        if (solo == 1)      side = 0.0f; // Mid Only
        else if (solo == 2) mid  = 0.0f; // Side Only

        // Decode
        left[sample]  = (mid + side) * invSqrt2;
        right[sample] = (mid - side) * invSqrt2;
    }
}

void MidSideProcessor::reset() {
    midTiltLow.clear();
    midTiltHigh.clear();
    sideHighpass.clear();
    midGainSmoother.setCurrentAndTargetValue(1.0f);
    sideGainSmoother.setCurrentAndTargetValue(1.0f);
}

std::vector<std::unique_ptr<juce::RangedAudioParameter>>
MidSideProcessor::createParameters(const juce::String& prefix) {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "mid_gain", 1},
        prefix + "Mid Gain",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "side_gain", 1},
        prefix + "Side Gain",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "mid_tilt", 1},
        prefix + "Mid Tilt",
        juce::NormalisableRange<float>(-6.0f, 6.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "side_highpass", 1},
        prefix + "Side Low Cut",
        juce::NormalisableRange<float>(20.0f, 1000.0f, 1.0f, 0.4f),
        20.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{prefix + "solo", 1},
        prefix + "Monitor",
        juce::StringArray{"Stereo", "Mid Only", "Side Only"},
        0));

    return params;
}

void MidSideProcessor::updateFromAPVTS(juce::AudioProcessorValueTreeState& apvts,
                                         const juce::String& prefix) {
    midGainParam      = getParam(apvts, prefix, "mid_gain");
    sideGainParam     = getParam(apvts, prefix, "side_gain");
    midTiltParam      = getParam(apvts, prefix, "mid_tilt");
    sideHighpassParam = getParam(apvts, prefix, "side_highpass");
    soloParam         = getParam(apvts, prefix, "solo");
}

} // namespace vybecode
