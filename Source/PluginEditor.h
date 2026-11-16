#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class MultiSaturationEditor : public juce::AudioProcessorEditor
{
public:
    explicit MultiSaturationEditor(MultiSaturationProcessor&);
    ~MultiSaturationEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    MultiSaturationProcessor& processorRef;

    // ── Controls ─────────────────────────────────────────
    juce::Label sectionLabel0;
    juce::Label sectionLabel1;
    juce::Label sectionLabel2;
    juce::Label sectionLabel3;
    juce::Label sectionLabel4;
    juce::Label sectionLabel5;
    juce::Label sectionLabel6;
    juce::Label sectionLabel7;
    juce::Label sectionLabel8;
    juce::Label sectionLabel9;
    juce::Label sectionLabel10;
    juce::Slider ingain_gain_dbSlider;
    juce::Slider api560_low_gainSlider;
    juce::Slider api560_mid_gainSlider;
    juce::Slider api560_high_gainSlider;
    juce::Slider xover_low_gainSlider;
    juce::Slider xover_mid_gainSlider;
    juce::Slider xover_high_gainSlider;
    juce::Slider band1_sat_drive_dbSlider;
    juce::Slider band1_sat_output_dbSlider;
    juce::Slider band1_comp_peak_reductionSlider;
    juce::Slider band1_comp_gainSlider;
    juce::Slider band1_phase_frequencySlider;
    juce::Slider band2_sat_drive_dbSlider;
    juce::Slider band2_sat_output_dbSlider;
    juce::Slider band2_comp_peak_reductionSlider;
    juce::Slider band2_comp_gainSlider;
    juce::Slider band2_phase_frequencySlider;
    juce::Slider band3_sat_drive_dbSlider;
    juce::Slider band3_sat_output_dbSlider;
    juce::Slider band3_comp_peak_reductionSlider;
    juce::Slider band3_comp_gainSlider;
    juce::Slider band3_phase_frequencySlider;
    juce::Slider band4_sat_drive_dbSlider;
    juce::Slider band4_sat_output_dbSlider;
    juce::Slider band4_comp_peak_reductionSlider;
    juce::Slider band4_comp_gainSlider;
    juce::Slider band4_phase_frequencySlider;
    juce::Slider band4_hf_amountSlider;
    juce::Slider band4_hf_mixSlider;
    juce::Slider crosstalk_mid_gainSlider;
    juce::Slider crosstalk_side_gainSlider;
    juce::Slider outgain_gain_dbSlider;
    juce::Slider lfo_rateSlider;
    juce::Slider lfo_depthSlider;
    juce::ComboBox lfo_shapeBox;
    juce::Slider envfoll_attack_msSlider;
    juce::Slider envfoll_release_msSlider;
    juce::Slider api560_low_freqSlider;
    juce::Slider api560_mid_freqSlider;
    juce::Slider api560_mid_qSlider;
    juce::Slider api560_high_freqSlider;
    juce::Slider xover_low_xoverSlider;
    juce::Slider xover_high_xoverSlider;
    juce::ComboBox xover_soloBox;
    juce::Slider band1_sat_asymmetrySlider;
    juce::Slider band1_comp_threshold_dbSlider;
    juce::ComboBox band1_comp_modeBox;
    juce::ComboBox band1_phase_stagesBox;
    juce::Slider band1_phase_spreadSlider;
    juce::Slider band2_sat_asymmetrySlider;
    juce::Slider band2_comp_threshold_dbSlider;
    juce::ComboBox band2_comp_modeBox;
    juce::ComboBox band2_phase_stagesBox;
    juce::Slider band2_phase_spreadSlider;
    juce::Slider band3_sat_asymmetrySlider;
    juce::Slider band3_comp_threshold_dbSlider;
    juce::ComboBox band3_comp_modeBox;
    juce::ComboBox band3_phase_stagesBox;
    juce::Slider band3_phase_spreadSlider;
    juce::Slider band4_sat_asymmetrySlider;
    juce::Slider band4_comp_threshold_dbSlider;
    juce::ComboBox band4_comp_modeBox;
    juce::ComboBox band4_phase_stagesBox;
    juce::Slider band4_phase_spreadSlider;
    juce::Slider band4_hf_frequencySlider;
    juce::Slider crosstalk_mid_tiltSlider;
    juce::Slider crosstalk_side_highpassSlider;
    juce::ComboBox crosstalk_soloBox;
    juce::Slider envfoll_sensitivitySlider;


    // ── Attachments (declared AFTER sliders for correct destruction order) ──
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ingain_gain_dbAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> api560_low_gainAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> api560_mid_gainAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> api560_high_gainAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> xover_low_gainAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> xover_mid_gainAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> xover_high_gainAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band1_sat_drive_dbAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band1_sat_output_dbAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band1_comp_peak_reductionAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band1_comp_gainAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band1_phase_frequencyAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band2_sat_drive_dbAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band2_sat_output_dbAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band2_comp_peak_reductionAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band2_comp_gainAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band2_phase_frequencyAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band3_sat_drive_dbAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band3_sat_output_dbAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band3_comp_peak_reductionAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band3_comp_gainAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band3_phase_frequencyAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band4_sat_drive_dbAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band4_sat_output_dbAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band4_comp_peak_reductionAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band4_comp_gainAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band4_phase_frequencyAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band4_hf_amountAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band4_hf_mixAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> crosstalk_mid_gainAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> crosstalk_side_gainAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outgain_gain_dbAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfo_rateAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfo_depthAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfo_shapeAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> envfoll_attack_msAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> envfoll_release_msAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> api560_low_freqAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> api560_mid_freqAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> api560_mid_qAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> api560_high_freqAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> xover_low_xoverAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> xover_high_xoverAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> xover_soloAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band1_sat_asymmetryAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band1_comp_threshold_dbAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> band1_comp_modeAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> band1_phase_stagesAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band1_phase_spreadAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band2_sat_asymmetryAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band2_comp_threshold_dbAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> band2_comp_modeAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> band2_phase_stagesAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band2_phase_spreadAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band3_sat_asymmetryAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band3_comp_threshold_dbAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> band3_comp_modeAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> band3_phase_stagesAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band3_phase_spreadAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band4_sat_asymmetryAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band4_comp_threshold_dbAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> band4_comp_modeAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> band4_phase_stagesAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band4_phase_spreadAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> band4_hf_frequencyAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> crosstalk_mid_tiltAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> crosstalk_side_highpassAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> crosstalk_soloAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> envfoll_sensitivityAttach;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiSaturationEditor)
};
