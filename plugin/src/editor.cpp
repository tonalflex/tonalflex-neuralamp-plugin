#include "editor.h"

NeuralAmpEditor::NeuralAmpEditor(NeuralAmpProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(400, 800);

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
    modelDropdownAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(params, "selectedNamModel", modelDropdown);

    // Setup Dropdown for model selection
    modelDropdownLabel.setText("Model Selector", juce::dontSendNotification);
    addAndMakeVisible(modelDropdownLabel);
    addAndMakeVisible(modelDropdown);
    modelDropdown.onChange = [this] { modelSelectionChanged(); };
    updateModelDropdown();

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

NeuralAmpEditor::~NeuralAmpEditor()
{
}

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
    fileChooser = std::make_unique<juce::FileChooser>("Select a NAM Model File", juce::File(), "*.nam"); // 🔴 Store it persistently
    DBG("Launching file chooser");

    fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                             [this](const juce::FileChooser& fc) {
                                 DBG("File chooser callback triggered");
                                 juce::File selectedFile = fc.getResult();
                                 if (selectedFile.existsAsFile())
                                 {
                                     DBG("Selected file exists: " << selectedFile.getFullPathName());
                                     processor.loadNamFromFile(selectedFile.getFullPathName());
                                     juce::AlertWindow::showMessageBoxAsync(
                                         juce::AlertWindow::InfoIcon,
                                         "Model Loaded",
                                         "Loaded: " + selectedFile.getFullPathName());
                                 }
                                 else
                                 {
                                     DBG("No file selected or file doesn’t exist");
                                 }
                             });

    DBG("File chooser launched");
}

void NeuralAmpEditor::updateModelDropdown()
{
    modelDropdown.clear();
    const auto& modelNames = processor.getModelNames();

    if (modelNames.isEmpty())
    {
        modelDropdown.addItem("No Models Available", 1);
        modelDropdown.setSelectedId(1, juce::dontSendNotification);
        DBG("No model names available");
        return;
    }

    for (int i = 0; i < modelNames.size(); ++i)
    {
        if (modelNames[i].isNotEmpty())
        {
            modelDropdown.addItem(modelNames[i], i + 1);
            DBG("Added model to dropdown: " << modelNames[i]);
        }
        else
        {
            DBG("Skipped empty model name at index: " << i);
        }
    }

    int currentIndex = processor.getCurrentModelIndex();
    int dropdownId = currentIndex + 1; // Map -1 to 1, 0 to 2, etc.
    if (dropdownId >= 1 && dropdownId <= modelNames.size())
    {
        modelDropdown.setSelectedId(dropdownId, juce::dontSendNotification);
        DBG("Selected model index: " << currentIndex << ", dropdown ID: " << dropdownId);
    }
    else
    {
        modelDropdown.setSelectedId(1, juce::dontSendNotification);
        DBG("Selected 'No Model' (dropdown ID: 1)");
    }
}

void NeuralAmpEditor::modelSelectionChanged()
{
    int selectedId = modelDropdown.getSelectedId();
    DBG("modelSelectionChanged: selectedId = " << selectedId);

    if (selectedId < 1 || selectedId > processor.getModelNames().size())
    {
        DBG("Invalid dropdown ID: " << selectedId << "; resetting to 1");
        modelDropdown.setSelectedId(1, juce::dontSendNotification);
        return;
    }

    int processorIndex = selectedId - 2; // Map ID (1–27) to processor index (-1 to 25)
    DBG("Computed processorIndex: " << processorIndex << " for selectedId: " << selectedId);

    if (processorIndex == processor.getCurrentModelIndex())
    {
        DBG("Model index " << processorIndex << " already selected; ignoring");
        return;
    }

    // Check current parameter state
    auto* param = processor.getParameters().getParameter("selectedNamModel");
    float currentValue = param->getValue();
    int currentParamIndex = static_cast<int>(currentValue * (processor.getModelNames().size() - 1));
    int newParamIndex = selectedId - 1; // Map ID (1–27) to param index (0–26)
    DBG("Current param index: " << currentParamIndex << ", new param index: " << newParamIndex);

    DBG("Setting model index: " << processorIndex << " for dropdown ID: " << selectedId);
    processor.setCurrentModelIndex(processorIndex); // Triggers loadNamFromFile
}

void NeuralAmpEditor::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &modelDropdown)
    {
        modelSelectionChanged();
    }
}

void NeuralAmpEditor::timerCallback()
{
}