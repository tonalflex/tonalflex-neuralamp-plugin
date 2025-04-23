#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "processor.h"

class NeuralAmpEditor : public juce::AudioProcessorEditor, public juce::ComboBox::Listener, public juce::Timer
{
public:
    NeuralAmpEditor(NeuralAmpProcessor&);
    ~NeuralAmpEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void loadModelButtonClicked();
    void updateModelDropdown();
    void modelSelectionChanged();
    void comboBoxChanged(juce::ComboBox*) override;
    void timerCallback() override;

    NeuralAmpProcessor& processor;

    // Sliders
    juce::Slider inputGainSlider, outputGainSlider, bassSlider, midSlider, trebleSlider;
    juce::Slider noiseGateThresholdSlider, inputCalibrationLevelSlider;

    // Slider Labels
    juce::Label inputGainLabel, outputGainLabel, bassLabel, midLabel, trebleLabel;
    juce::Label noiseGateThresholdLabel, inputCalibrationLevelLabel;

    // Toggles
    juce::ToggleButton noiseGateToggle, irToggle, eqActiveToggle, calibrateInputToggle;

    // Toggle Labels
    juce::Label noiseGateLabel, irLabel, eqActiveLabel, calibrateInputLabel;

    // Output Mode
    juce::ComboBox outputModeSelector;
    juce::Label outputModeLabel;

    // Model Selection
    juce::ComboBox modelDropdown;
    juce::Label modelDropdownLabel;

    // Load Model Button
    juce::TextButton loadModelButton;
    juce::Label loadModelLabel;

    // File Chooser
    std::unique_ptr<juce::FileChooser> fileChooser;

    // Parameter Attachments
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
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modelDropdownAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeuralAmpEditor)
};