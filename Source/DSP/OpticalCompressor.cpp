/*
    OpticalCompressor.cpp — Opto/LA-2A style compressor
*/

#include "OpticalCompressor.h"

namespace vybecode {

void OpticalCompressorModule::prepare(const ModuleSpec& spec) {
    sampleRate = spec.sampleRate;
    rmsLevel = 0.0f;
    gainReductionDb = 0.0f;
    timeAboveThreshold = 0.0f;
    gainSmoother.reset(sampleRate, 0.02);
    gainSmoother.setCurrentAndTargetValue(1.0f);
}

void OpticalCompressorModule::process(juce::AudioBuffer<float>& buffer) {
    const float threshold    = juce::jlimit(-40.0f, 0.0f,
        (thresholdParam ? thresholdParam->load() : -20.0f));
    const float peakReduction = peakReductionParam  ? peakReductionParam->load()  : 50.0f;
    const float gainDb       = juce::jlimit(0.0f, 24.0f,
        (gainParam ? gainParam->load() : 0.0f));
    const float mode         = modeParam           ? modeParam->load()           : 0.0f;

    gainSmoother.setTargetValue(juce::Decibels::decibelsToGain(gainDb));

    // Mode: 0 = compress (ratio ~3:1), 1 = limit (ratio ~10:1)
    const float ratio = (mode >= 0.5f) ? 10.0f : 3.0f;

    // Scale compression amount by peak_reduction knob (0-100%)
    const float compressionAmount = peakReduction / 100.0f;

    // RMS smoothing coefficient (~10ms window for opto character)
    const float rmsCoeff = std::exp(-1.0f / (float(sampleRate) * 0.01f));

    // Opto attack tracks mode: Limit grabs fast (~1ms), Compress is slow opto (~10ms)
    const float attackMs = (mode >= 0.5f) ? 1.0f : 10.0f;
    const float attackCoeff = std::exp(-1.0f / (float(sampleRate) * attackMs * 0.001f));

    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();
    const float invSampleRate = 1.0f / float(sampleRate);

    for (int sample = 0; sample < numSamples; ++sample) {
        // Compute sum of squares across channels
        float sumSq = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch) {
            float s = buffer.getReadPointer(ch)[sample];
            sumSq += s * s;
        }
        sumSq /= float(numChannels);

        // Smooth RMS
        rmsLevel = rmsCoeff * rmsLevel + (1.0f - rmsCoeff) * sumSq;
        // 10*log10(meanSquare) == 20*log10(rms) == amplitude dBFS of the RMS detector
        float rmsDb = (rmsLevel > 1e-20f)
            ? 10.0f * std::log10(rmsLevel)
            : -200.0f;

        // Opto "peak reduction" classically pulls the engage point down (up to 24 dB) as it
        // is raised, so more signal is grabbed -- this is what makes the knob audible on a
        // normal glue-bus level, not just GR depth.
        const float effThreshold = threshold - compressionAmount * 24.0f;
        const float knee = 6.0f; // soft knee, dB

        // Compute desired gain reduction (soft knee so peak_reduction is felt continuously)
        float overDb = rmsDb - effThreshold;
        float targetGrDb = 0.0f;
        if (overDb > -knee * 0.5f) {
            float kneeOver = (overDb < knee * 0.5f)
                ? (overDb + knee * 0.5f) * (overDb + knee * 0.5f) / (2.0f * knee)
                : overDb;
            targetGrDb = -(kneeOver - kneeOver / ratio) * (0.5f + 0.5f * compressionAmount);
            timeAboveThreshold += invSampleRate;
        } else {
            timeAboveThreshold = std::max(0.0f, timeAboveThreshold - invSampleRate);
        }

        // Program-dependent release: longer signal was compressed, slower release
        float releaseMs = 60.0f + timeAboveThreshold * 500.0f; // 60ms base, up to ~560ms
        releaseMs = std::min(releaseMs, 2000.0f);
        float releaseCoeff = std::exp(-1.0f / (float(sampleRate) * releaseMs * 0.001f));

        // Apply ballistics
        if (targetGrDb < gainReductionDb)
            gainReductionDb = attackCoeff * gainReductionDb + (1.0f - attackCoeff) * targetGrDb;
        else
            gainReductionDb = releaseCoeff * gainReductionDb + (1.0f - releaseCoeff) * targetGrDb;

        // Apply gain
        float grLinear = juce::Decibels::decibelsToGain(gainReductionDb);
        float makeup = gainSmoother.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch)
            buffer.getWritePointer(ch)[sample] *= grLinear * makeup;
    }
}

void OpticalCompressorModule::reset() {
    rmsLevel = 0.0f;
    gainReductionDb = 0.0f;
    timeAboveThreshold = 0.0f;
    gainSmoother.setCurrentAndTargetValue(1.0f);
}

std::vector<std::unique_ptr<juce::RangedAudioParameter>>
OpticalCompressorModule::createParameters(const juce::String& prefix) {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "threshold_db", 1},
        prefix + "Threshold",
        juce::NormalisableRange<float>(-40.0f, 0.0f, 0.1f),
        -20.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "peak_reduction", 1},
        prefix + "Peak Reduction",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{prefix + "gain", 1},
        prefix + "Gain",
        juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{prefix + "mode", 1},
        prefix + "Mode",
        juce::StringArray{"Compress", "Limit"},
        0));

    return params;
}

void OpticalCompressorModule::updateFromAPVTS(juce::AudioProcessorValueTreeState& apvts,
                                                const juce::String& prefix) {
    thresholdParam     = getParam(apvts, prefix, "threshold_db");
    peakReductionParam = getParam(apvts, prefix, "peak_reduction");
    gainParam          = getParam(apvts, prefix, "gain");
    modeParam          = getParam(apvts, prefix, "mode");
}

} // namespace vybecode
