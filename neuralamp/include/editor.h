#pragma once
#include "processor.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class NeuralAmpEditor : public juce::AudioProcessorEditor
{
public:
    NeuralAmpEditor(NeuralAmpProcessor&);
    ~NeuralAmpEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    NeuralAmpProcessor& processor;

    // Sliders
    juce::Slider inputGainSlider;
    juce::Slider outputGainSlider;
    juce::Slider bassSlider;
    juce::Slider midSlider;
    juce::Slider trebleSlider;
    juce::Slider noiseGateThresholdSlider;
    juce::Slider inputCalibrationLevelSlider;

    // Labels for Sliders
    juce::Label inputGainLabel;
    juce::Label outputGainLabel;
    juce::Label bassLabel;
    juce::Label midLabel;
    juce::Label trebleLabel;
    juce::Label noiseGateThresholdLabel;
    juce::Label inputCalibrationLevelLabel;

    // Toggles
    juce::ToggleButton noiseGateToggle;
    juce::ToggleButton irToggle;
    juce::ToggleButton eqActiveToggle;
    juce::ToggleButton calibrateInputToggle;

    // Labels for Toggles
    juce::Label noiseGateLabel;
    juce::Label irLabel;
    juce::Label eqActiveLabel;
    juce::Label calibrateInputLabel;

    // Dropdown
    juce::ComboBox outputModeSelector;
    juce::Label outputModeLabel;

    // NamModelsDropDown
    juce::ComboBox modelDropdown;
    juce::Label modelLabel;

    void updateModelDropdown();
    void modelSelectionChanged();

    // Button
    juce::TextButton loadModelButton;
    juce::Label loadModelLabel;

    // 🔴 Store FileChooser as a member variable
    std::unique_ptr<juce::FileChooser> fileChooser;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> trebleAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noiseGateThresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputCalibrationLevelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> noiseGateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> irAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> eqActiveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> calibrateInputAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> outputModeAttachment;

    void loadModelButtonClicked();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeuralAmpEditor)
};