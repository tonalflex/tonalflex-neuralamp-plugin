#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "processor.h"

class NeuralAmpEditor : public juce::AudioProcessorEditor, public juce::ComboBox::Listener {
public:
  NeuralAmpEditor(NeuralAmpProcessor&);
  ~NeuralAmpEditor() override;

  void paint(juce::Graphics&) override;
  void resized() override;

private:
  void updateModelDropdown();
  void updateIrDropdown();
  void modelSelectionChanged();
  void irSelectionChanged();
  void comboBoxChanged(juce::ComboBox*) override;

  NeuralAmpProcessor& processor;

  // Sliders
  juce::Slider inputGainSlider, outputGainSlider, bassSlider, midSlider, trebleSlider;
  juce::Slider noiseGateThresholdSlider, inputCalibrationLevelSlider;

  // Slider Labels
  juce::Label inputGainLabel, outputGainLabel, bassLabel, midLabel, trebleLabel;
  juce::Label noiseGateThresholdLabel, inputCalibrationLevelLabel;

  // Toggles
  juce::ToggleButton noiseGateToggle, irToggle, eqActiveToggle, calibrateInputToggle,
      normalizeActiveToggle, normalizeIrActiveToggle;

  // Toggle Labels
  juce::Label noiseGateLabel, irLabel, eqActiveLabel, calibrateInputLabel, normalizeActiveLabel,
      normalizeIrActiveLabel;

  // Model Selection
  juce::ComboBox modelDropdown;
  juce::Label modelDropdownLabel;

  // IR Selection
  juce::ComboBox irDropdown;
  juce::Label irDropdownLabel;

  // Parameter Attachments
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bassAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> trebleAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      noiseGateThresholdAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      inputCalibrationLevelAttachment;

  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> noiseGateAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> irAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> eqActiveAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> calibrateInputAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> normalizeActiveAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> normalizeIrActiveAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modelDropdownAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> irDropdownAttachment;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeuralAmpEditor)
};