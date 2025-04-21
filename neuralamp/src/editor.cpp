// === plugin_editor.cpp ===
#include "editor.h"

NeuralAmpEditor::NeuralAmpEditor(NeuralAmpProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(400, 800); // Default look, same size

    auto& params = processor.getParameters();

    // Attach Sliders
    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(params, "inputLevel", inputGainSlider);
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(params, "outputLevel", outputGainSlider);
    bassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(params, "toneBass", bassSlider);
    midAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(params, "toneMid", midSlider);
    trebleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(params, "toneTreble", trebleSlider);
    noiseGateThresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(params, "noiseGateThreshold", noiseGateThresholdSlider);
    inputCalibrationLevelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(params, "inputCalibrationLevel", inputCalibrationLevelSlider);

    // Attach Toggles
    noiseGateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(params, "noiseGateActive", noiseGateToggle);
    irAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(params, "irToggle", irToggle);
    eqActiveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(params, "eqActive", eqActiveToggle);
    calibrateInputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(params, "calibrateInput", calibrateInputToggle);

    // Attach ComboBox
    outputModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(params, "outputMode", outputModeSelector);

    // Setup Dropdown for model selection
    modelDropdownLabel.setText("Model Selector", juce::dontSendNotification);
    addAndMakeVisible(modelDropdownLabel);
    addAndMakeVisible(modelDropdown);
    updateModelDropdown();
    modelDropdown.onChange = [this] { modelSelectionChanged(); };

    // Configure Sliders (Default Look)
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

    calibrateInputLabel.setText("Calibrate Input", juce::dontSendNotification);
    addAndMakeVisible(calibrateInputLabel);
    calibrateInputToggle.setButtonText("");
    addAndMakeVisible(calibrateInputToggle);

    // Output Mode Dropdown with Label
    outputModeLabel.setText("Output Mode", juce::dontSendNotification);
    addAndMakeVisible(outputModeLabel);
    addAndMakeVisible(outputModeSelector);

    // Load Model Button with Label
    loadModelLabel.setText("Load Model", juce::dontSendNotification);
    addAndMakeVisible(loadModelLabel);
    loadModelButton.setButtonText("Browse Models");
    loadModelButton.onClick = [this] { loadModelButtonClicked(); };
    addAndMakeVisible(loadModelButton);
}

NeuralAmpEditor::~NeuralAmpEditor() {}

void NeuralAmpEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::black);
    g.setFont(15.0f);
}

void NeuralAmpEditor::resized()
{
    auto area = getLocalBounds().reduced(10, 0);
    auto rowHeight = 30;
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
    calibrateInputLabel.setBounds(row.removeFromLeft(labelWidth));
    calibrateInputToggle.setBounds(row);

    row = area.removeFromTop(rowHeight);
    outputModeLabel.setBounds(row.removeFromLeft(labelWidth));
    outputModeSelector.setBounds(row);

    row = area.removeFromTop(rowHeight);
    modelDropdownLabel.setBounds(row.removeFromLeft(labelWidth));
    modelDropdown.setBounds(row);

    row = area.removeFromTop(rowHeight);
    loadModelLabel.setBounds(row.removeFromLeft(labelWidth));
    loadModelButton.setBounds(row);
}

void NeuralAmpEditor::loadModelButtonClicked()
{
    fileChooser = std::make_unique<juce::FileChooser>("Select a NAM Model File", juce::File(), "*.nam");

    fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc) {
            juce::File selectedFile = fc.getResult();
            if (selectedFile.existsAsFile()) {
                processor.loadNamFromFile(selectedFile.getFullPathName());
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::InfoIcon,
                    "Model Loaded",
                    "Loaded: " + selectedFile.getFullPathName());
            }
        });
}

void NeuralAmpEditor::updateModelDropdown()
{
    modelDropdown.clear();

    auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(
        processor.getParameters().getParameter("selectedNamModel"));

    if (choiceParam != nullptr)
    {
        auto choices = choiceParam->choices;
        for (int i = 0; i < choices.size(); ++i)
        {
            modelDropdown.addItem(choices[i], i + 1);
        }

        modelDropdown.setSelectedId(choiceParam->getIndex() + 1);
    }
}

void NeuralAmpEditor::modelSelectionChanged()
{
    auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(
        processor.getParameters().getParameter("selectedNamModel"));

    if (choiceParam != nullptr)
    {
        int selectedId = modelDropdown.getSelectedId();
        if (selectedId > 0 && selectedId - 1 != choiceParam->getIndex())
        {
            choiceParam->setValueNotifyingHost(static_cast<float>(selectedId - 1));
        }
    }
}
