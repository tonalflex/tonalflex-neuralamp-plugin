#include "editor.h"

NeuralAmpEditor::NeuralAmpEditor(NeuralAmpProcessor& p) : AudioProcessorEditor(&p), processor(p) {
  setSize(400, 800);

  auto& params = processor.getParameters();

  // Attach Sliders
  inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      params, "inputLevel", inputGainSlider);
  outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      params, "outputLevel", outputGainSlider);
  bassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      params, "toneBass", bassSlider);
  midAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      params, "toneMid", midSlider);
  trebleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      params, "toneTreble", trebleSlider);
  noiseGateThresholdAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          params, "noiseGateThreshold", noiseGateThresholdSlider);
  inputCalibrationLevelAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          params, "inputCalibrationLevel", inputCalibrationLevelSlider);

  // Attach Toggles
  noiseGateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
      params, "noiseGateActive", noiseGateToggle);
  irAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
      params, "irToggle", irToggle);
  eqActiveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
      params, "eqActive", eqActiveToggle);
  normalizeActiveAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          params, "normalizeNamOutput", normalizeActiveToggle);
  normalizeIrActiveAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          params, "normalizeIrOutput", normalizeIrActiveToggle);

  // Attach ComboBoxes
  modelDropdownAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
          params, "selectedNamModel", modelDropdown);
  irDropdownAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
      params, "selectedIR", irDropdown);

  // Setup Model Dropdown
  modelDropdownLabel.setText("Model Selector", juce::dontSendNotification);
  addAndMakeVisible(modelDropdownLabel);
  addAndMakeVisible(modelDropdown);
  modelDropdown.onChange = [this] { modelSelectionChanged(); };
  updateModelDropdown();

  // Setup IR Dropdown
  irDropdownLabel.setText("IR Selector", juce::dontSendNotification);
  addAndMakeVisible(irDropdownLabel);
  addAndMakeVisible(irDropdown);
  irDropdown.onChange = [this] { irSelectionChanged(); };
  updateIrDropdown();

  // Configure Sliders
  auto setupSlider = [](juce::Slider& slider) {
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
  };
  setupSlider(inputGainSlider);
  setupSlider(outputGainSlider);
  setupSlider(bassSlider);
  setupSlider(midSlider);
  setupSlider(trebleSlider);
  setupSlider(noiseGateThresholdSlider);
  setupSlider(inputCalibrationLevelSlider);

  // Setup and Add Sliders with Labels
  inputGainLabel.setText("Input Gain", juce::dontSendNotification);
  addAndMakeVisible(inputGainLabel);
  addAndMakeVisible(inputGainSlider);

  outputGainLabel.setText("Output Gain", juce::dontSendNotification);
  addAndMakeVisible(outputGainLabel);
  addAndMakeVisible(outputGainSlider);

  bassLabel.setText("Bass", juce::dontSendNotification);
  addAndMakeVisible(bassLabel);
  addAndMakeVisible(bassSlider);

  midLabel.setText("Mid", juce::dontSendNotification);
  addAndMakeVisible(midLabel);
  addAndMakeVisible(midSlider);

  trebleLabel.setText("Treble", juce::dontSendNotification);
  addAndMakeVisible(trebleLabel);
  addAndMakeVisible(trebleSlider);

  noiseGateThresholdLabel.setText("Noise Gate Threshold", juce::dontSendNotification);
  addAndMakeVisible(noiseGateThresholdLabel);
  addAndMakeVisible(noiseGateThresholdSlider);

  inputCalibrationLevelLabel.setText("Input Calibration", juce::dontSendNotification);
  addAndMakeVisible(inputCalibrationLevelLabel);
  addAndMakeVisible(inputCalibrationLevelSlider);

  // Setup and Add Toggles with Labels
  noiseGateLabel.setText("Noise Gate", juce::dontSendNotification);
  addAndMakeVisible(noiseGateLabel);
  noiseGateToggle.setButtonText("");
  addAndMakeVisible(noiseGateToggle);

  irLabel.setText("IR Toggle", juce::dontSendNotification);
  addAndMakeVisible(irLabel);
  irToggle.setButtonText("");
  addAndMakeVisible(irToggle);

  eqActiveLabel.setText("EQ Active", juce::dontSendNotification);
  addAndMakeVisible(eqActiveLabel);
  eqActiveToggle.setButtonText("");
  addAndMakeVisible(eqActiveToggle);

  normalizeActiveLabel.setText("Nornamlize NAM", juce::dontSendNotification);
  addAndMakeVisible(normalizeActiveLabel);
  normalizeActiveToggle.setButtonText("");
  addAndMakeVisible(normalizeActiveToggle);

  normalizeIrActiveLabel.setText("Nornamlize IR", juce::dontSendNotification);
  addAndMakeVisible(normalizeIrActiveLabel);
  normalizeIrActiveToggle.setButtonText("");
  addAndMakeVisible(normalizeIrActiveToggle);

  calibrateInputLabel.setText("Calibrate Input", juce::dontSendNotification);
  addAndMakeVisible(calibrateInputLabel);
  calibrateInputToggle.setButtonText("");
  addAndMakeVisible(calibrateInputToggle);
}

NeuralAmpEditor::~NeuralAmpEditor() {}

void NeuralAmpEditor::paint(juce::Graphics& g) {
  g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
  g.setColour(juce::Colours::black);
  g.setFont(15.0f);
}

void NeuralAmpEditor::resized() {
  auto area = getLocalBounds().reduced(10, 0);
  auto rowHeight = 40;  // Increased for better spacing
  auto labelWidth = 150;

  auto row = area.removeFromTop(rowHeight);
  inputGainLabel.setBounds(row.removeFromLeft(labelWidth));
  inputGainSlider.setBounds(row);

  row = area.removeFromTop(rowHeight);
  outputGainLabel.setBounds(row.removeFromLeft(labelWidth));
  outputGainSlider.setBounds(row);

  row = area.removeFromTop(rowHeight);
  bassLabel.setBounds(row.removeFromLeft(labelWidth));
  bassSlider.setBounds(row);

  row = area.removeFromTop(rowHeight);
  midLabel.setBounds(row.removeFromLeft(labelWidth));
  midSlider.setBounds(row);

  row = area.removeFromTop(rowHeight);
  trebleLabel.setBounds(row.removeFromLeft(labelWidth));
  trebleSlider.setBounds(row);

  row = area.removeFromTop(rowHeight);
  noiseGateThresholdLabel.setBounds(row.removeFromLeft(labelWidth));
  noiseGateThresholdSlider.setBounds(row);

  row = area.removeFromTop(rowHeight);
  inputCalibrationLevelLabel.setBounds(row.removeFromLeft(labelWidth));
  inputCalibrationLevelSlider.setBounds(row);

  row = area.removeFromTop(rowHeight);
  noiseGateLabel.setBounds(row.removeFromLeft(labelWidth));
  noiseGateToggle.setBounds(row);

  row = area.removeFromTop(rowHeight);
  irLabel.setBounds(row.removeFromLeft(labelWidth));
  irToggle.setBounds(row);

  row = area.removeFromTop(rowHeight);
  eqActiveLabel.setBounds(row.removeFromLeft(labelWidth));
  eqActiveToggle.setBounds(row);

  row = area.removeFromTop(rowHeight);
  normalizeActiveLabel.setBounds(row.removeFromLeft(labelWidth));
  normalizeActiveToggle.setBounds(row);

  row = area.removeFromTop(rowHeight);
  normalizeIrActiveLabel.setBounds(row.removeFromLeft(labelWidth));
  normalizeIrActiveToggle.setBounds(row);

  row = area.removeFromTop(rowHeight);
  calibrateInputLabel.setBounds(row.removeFromLeft(labelWidth));
  calibrateInputToggle.setBounds(row);

  row = area.removeFromTop(rowHeight);
  modelDropdownLabel.setBounds(row.removeFromLeft(labelWidth));
  modelDropdown.setBounds(row);

  row = area.removeFromTop(rowHeight);
  irDropdownLabel.setBounds(row.removeFromLeft(labelWidth));
  irDropdown.setBounds(row);
}

void NeuralAmpEditor::updateModelDropdown() {
  modelDropdown.clear();
  const auto& modelNames = processor.getModelNames();

  if (modelNames.isEmpty()) {
    modelDropdown.addItem("No Models Available", 1);
    modelDropdown.setSelectedId(1, juce::dontSendNotification);
    DBG("No model names available");
    return;
  }

  for (int i = 0; i < modelNames.size(); ++i) {
    if (modelNames[i].isNotEmpty()) {
      modelDropdown.addItem(modelNames[i], i + 1);
      DBG("Added model to dropdown: " << modelNames[i]);
    }
  }

  int currentIndex = processor.getCurrentModelIndex();
  int dropdownId = currentIndex + 1;
  if (dropdownId >= 1 && dropdownId <= modelNames.size()) {
    modelDropdown.setSelectedId(dropdownId, juce::dontSendNotification);
    DBG("Selected model index: " << currentIndex << ", dropdown ID: " << dropdownId);
  } else {
    modelDropdown.setSelectedId(1, juce::dontSendNotification);
    DBG("Selected 'No Model' (dropdown ID: 1)");
  }
}

void NeuralAmpEditor::updateIrDropdown() {
  irDropdown.clear();
  const auto& irNames = processor.getIrNames();

  if (irNames.isEmpty()) {
    irDropdown.addItem("No IRs Available", 1);
    irDropdown.setSelectedId(1, juce::dontSendNotification);
    DBG("No IR names available");
    return;
  }

  for (int i = 0; i < irNames.size(); ++i) {
    if (irNames[i].isNotEmpty()) {
      irDropdown.addItem(irNames[i], i + 1);
      DBG("Added IR to dropdown: " << irNames[i]);
    }
  }

  int currentIndex = processor.getCurrentIrIndex();
  int dropdownId = currentIndex + 1;
  if (dropdownId >= 1 && dropdownId <= irNames.size()) {
    irDropdown.setSelectedId(dropdownId, juce::dontSendNotification);
    DBG("Selected IR index: " << currentIndex << ", dropdown ID: " << dropdownId);
  } else {
    irDropdown.setSelectedId(1, juce::dontSendNotification);
    DBG("Selected 'No IR' (dropdown ID: 1)");
  }
}

void NeuralAmpEditor::modelSelectionChanged() {
  int selectedId = modelDropdown.getSelectedId();
  processor.setCurrentModelIndex(selectedId);  // ComboBox ID passed directly
}

void NeuralAmpEditor::irSelectionChanged() {
  int selectedId = irDropdown.getSelectedId();
  processor.setCurrentIrIndex(selectedId);  // ComboBox ID passed directly
}

void NeuralAmpEditor::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) {
  if (comboBoxThatHasChanged == &modelDropdown) {
    modelSelectionChanged();
  } else if (comboBoxThatHasChanged == &irDropdown) {
    irSelectionChanged();
  }
}