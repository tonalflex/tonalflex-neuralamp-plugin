#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "processor.h"
#include "BinaryData.h"  // Contains embedded Web UI assets (HTML/CSS/JS)

// Restricts WebView loading to internal resources only
struct SinglePageBrowser : juce::WebBrowserComponent {
  using WebBrowserComponent::WebBrowserComponent;
  bool pageAboutToLoad(const juce::String& newURL) override {
    return newURL == juce::String("http://localhost:5173/") || newURL == getResourceProviderRoot();
  }
};

class NeuralAmpEditor : public juce::AudioProcessorEditor {
public:
  NeuralAmpEditor(NeuralAmpProcessor&);
  ~NeuralAmpEditor() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  int getControlParameterIndex(juce::Component&) override {
    return controlParameterIndexReceiver.getControlParameterIndex();
  }

private:
  void updateModelDropdown();
  void updateIrDropdown();

  NeuralAmpProcessor& processor;
  //==============================================================================
  // WebView UI
  //==============================================================================
  std::unique_ptr<juce::WebBrowserComponent> webView;

  juce::WebControlParameterIndexReceiver controlParameterIndexReceiver;

  juce::WebSliderRelay inputLevelRelay{"inputLevel"};
  juce::WebSliderRelay outputLevelRelay{"outputLevel"};
  juce::WebSliderRelay bassRelay{"toneBass"};
  juce::WebSliderRelay midRelay{"toneMid"};
  juce::WebSliderRelay trebleRelay{"toneTreble"};
  juce::WebSliderRelay noiseGateThresholdRelay{"noiseGateThreshold"};

  juce::WebToggleButtonRelay noiseGateToggleRelay{"noiseGateToggle"};
  juce::WebToggleButtonRelay eqToggleRelay{"eqToggle"};
  juce::WebToggleButtonRelay irToggleRelay{"irToggle"};
  juce::WebToggleButtonRelay normalizeNamOutputRelay{"normalizeNamOutput"};
  juce::WebToggleButtonRelay normalizeIrOutputRelay{"normalizeIrOutput"};

  juce::WebComboBoxRelay modelDropdownRelay{"selectedNamModel"};
  juce::WebComboBoxRelay irDropdownRelay{"selectedIR"};

  // Attachments
  juce::WebSliderParameterAttachment inputLevelWebAttachment{
      *processor.parameters.getParameter("inputLevel"), inputLevelRelay, nullptr};
  juce::WebSliderParameterAttachment outputLevelWebAttachment{
      *processor.parameters.getParameter("outputLevel"), outputLevelRelay, nullptr};
  juce::WebSliderParameterAttachment bassWebAttachment{
      *processor.parameters.getParameter("toneBass"), bassRelay, nullptr};
  juce::WebSliderParameterAttachment midWebAttachment{*processor.parameters.getParameter("toneMid"),
                                                      midRelay, nullptr};
  juce::WebSliderParameterAttachment trebleWebAttachment{
      *processor.parameters.getParameter("toneTreble"), trebleRelay, nullptr};
  juce::WebSliderParameterAttachment noiseGateThresholdWebAttachment{
      *processor.parameters.getParameter("noiseGateThreshold"), noiseGateThresholdRelay, nullptr};
  juce::WebToggleButtonParameterAttachment noiseGateToggleWebAttachment{
      *processor.parameters.getParameter("noiseGateToggle"), noiseGateToggleRelay};
  juce::WebToggleButtonParameterAttachment eqActiveWebAttachment{
      *processor.parameters.getParameter("eqToggle"), eqToggleRelay};
  juce::WebToggleButtonParameterAttachment irToggleWebAttachment{
      *processor.parameters.getParameter("irToggle"), irToggleRelay};
  juce::WebToggleButtonParameterAttachment normalizeNamOutputWebAttachment{
      *processor.parameters.getParameter("normalizeNamOutput"), normalizeNamOutputRelay};
  juce::WebToggleButtonParameterAttachment normalizeIrOutputWebAttachment{
      *processor.parameters.getParameter("normalizeIrOutput"), normalizeIrOutputRelay};

  juce::WebComboBoxParameterAttachment modelDropdownWebAttachment{
      *processor.parameters.getParameter("selectedNamModel"), modelDropdownRelay};
  juce::WebComboBoxParameterAttachment irDropdownWebAttachment{
      *processor.parameters.getParameter("selectedIR"), irDropdownRelay};

  std::optional<juce::WebBrowserComponent::Resource> getResource(const juce::String& url);
  juce::String getMimeForExtension(const juce::String& extension);

  //==============================================================================
  // Native JUCE UI
  //==============================================================================

  // Sliders
  juce::Slider inputGainSlider, outputGainSlider, bassSlider, midSlider, trebleSlider,
      noiseGateThresholdSlider;

  // Slider Labels
  juce::Label inputGainLabel, outputGainLabel, bassLabel, midLabel, trebleLabel,
      noiseGateThresholdLabel, noiseGateLabel, irToggleLabel, eqToggleLabel,
      normalizeNamOutputLabel, normalizeIrOutputLabel, modelDropdownLabel, irDropdownLabel;

  // Toggles
  juce::ToggleButton noiseGateToggle, irToggle, eqToggle, normalizeNamOutput, normalizeIrOutput;

  // ComboBoxes
  juce::ComboBox modelDropdown, irDropdown;

  // Parameter Attachments
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment,
      outputGainAttachment, bassAttachment, midAttachment, trebleAttachment,
      noiseGateThresholdAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> noiseGateAttachment,
      irToggleAttachment, eqToggleAttachment, normalizeNamOutputAttachment,
      normalizeIrOutputAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modelDropdownAttachment,
      irDropdownAttachment;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeuralAmpEditor)
};