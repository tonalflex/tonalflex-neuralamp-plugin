#include "editor.h"
#include "processor.h"

NeuralAmpEditor::NeuralAmpEditor(NeuralAmpProcessor& p) : AudioProcessorEditor(&p), processor(p) {
  webView = std::make_unique<juce::WebBrowserComponent>(

      juce::WebBrowserComponent::Options{}
          .withNativeIntegrationEnabled()  // (C++ <=> JS bridge, events, etc.)

          // Explicitly use WebView2 backend on Windows for modern HTML/CSS/JS support
          // JUCE defaults to WebKit on macOS/Linux
          .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
          .withWinWebView2Options(
              juce::WebBrowserComponent::Options::WinWebView2{}.withUserDataFolder(
                  juce::File::getSpecialLocation(juce::File::tempDirectory)))

          // Provide WebView UI resources from JUCE BinaryData (HTML/CSS/JS, etc.)
          .withResourceProvider([this](const auto& url) { return getResource(url); },
                                juce::URL{"http://localhost:5173/"}.getOrigin())

          // Add support for control focus tracking in the WebView (parameter automation)
          .withOptionsFrom(controlParameterIndexReceiver)

          // Bind parameter relays for two-way communication (C++ <=> JS)
          .withOptionsFrom(inputLevelRelay)
          .withOptionsFrom(outputLevelRelay)
          .withOptionsFrom(bassRelay)
          .withOptionsFrom(midRelay)
          .withOptionsFrom(trebleRelay)
          .withOptionsFrom(noiseGateThresholdRelay)
          .withOptionsFrom(noiseGateToggleRelay)
          .withOptionsFrom(eqToggleRelay)
          .withOptionsFrom(irToggleRelay)
          .withOptionsFrom(normalizeNamOutputRelay)
          .withOptionsFrom(normalizeIrOutputRelay)
          .withOptionsFrom(modelDropdownRelay)
          .withOptionsFrom(irDropdownRelay)

          .withNativeFunction(
              "getModelChoices",
              [this](const juce::Array<juce::var>& args,
                     juce::WebBrowserComponent::NativeFunctionCompletion completion) {
                juce::StringArray names = processor.getModelNames();
                juce::Array<juce::var> result;
                for (const auto& name : names) {
                  result.add(juce::var(name));
                }
                // juce::Logger::writeToLog("getModelChoices returning: " +
                //                         names.joinIntoString(", "));
                completion(result);
              })
          .withNativeFunction(
              "getIRChoices",
              [this](const juce::Array<juce::var>& args,
                     juce::WebBrowserComponent::NativeFunctionCompletion completion) {
                juce::StringArray names = processor.getIrNames();
                juce::Array<juce::var> result;
                for (const auto& name : names) {
                  result.add(juce::var(name));
                }
                // juce::Logger::writeToLog("getIRChoices returning: " + names.joinIntoString(",
                // "));
                completion(result);
              })

          // Inject debug message into browser console on load
          .withUserScript(R"(console.log("JUCE C++ Backend is running!");)"));

  // Set size of desktop plugin window (pixels)
  setSize(500, 600);

  // Ensure WebView is added after full construction (avoids timing issues)
  juce::MessageManager::callAsync([this]() {
    addAndMakeVisible(*webView);
    webView->goToURL(juce::WebBrowserComponent::getResourceProviderRoot() + "index.html");
  });

  // !!!
  /*
  // Attach Sliders
  inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      processor.getParameters(), "inputLevel", inputGainSlider);
  outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      processor.getParameters(), "outputLevel", outputGainSlider);
  bassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      processor.getParameters(), "toneBass", bassSlider);
  midAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      processor.getParameters(), "toneMid", midSlider);
  trebleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
      processor.getParameters(), "toneTreble", trebleSlider);
  noiseGateThresholdAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processor.getParameters(), "noiseGateThreshold", noiseGateThresholdSlider);

  // Attach Toggles
  noiseGateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
      processor.getParameters(), "noiseGateToggle", noiseGateToggle);
  irToggleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
      processor.getParameters(), "irToggle", irToggle);
  eqToggleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
      processor.getParameters(), "eqToggle", eqToggle);
  normalizeNamOutputAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          processor.getParameters(), "normalizeNamOutput", normalizeNamOutput);
  normalizeIrOutputAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          processor.getParameters(), "normalizeIrOutput", normalizeIrOutput);

  // Attach ComboBoxes
  modelDropdownAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
          processor.getParameters(), "selectedNamModel", modelDropdown);
  irDropdownAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
      processor.getParameters(), "selectedIR", irDropdown);

  // Setup Model Dropdown
  modelDropdownLabel.setText("Model Selector", juce::dontSendNotification);
  addAndMakeVisible(modelDropdownLabel);
  addAndMakeVisible(modelDropdown);
  updateModelDropdown();

  // Setup IR Dropdown
  irDropdownLabel.setText("IR Selector", juce::dontSendNotification);
  addAndMakeVisible(irDropdownLabel);
  addAndMakeVisible(irDropdown);
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

  // Setup and Add Toggles with Labels
  noiseGateLabel.setText("Noise Gate", juce::dontSendNotification);
  addAndMakeVisible(noiseGateLabel);
  noiseGateToggle.setButtonText("");
  addAndMakeVisible(noiseGateToggle);

  irToggleLabel.setText("IR Toggle", juce::dontSendNotification);
  addAndMakeVisible(irToggleLabel);
  irToggle.setButtonText("");
  addAndMakeVisible(irToggle);

  eqToggleLabel.setText("EQ Active", juce::dontSendNotification);
  addAndMakeVisible(eqToggleLabel);
  eqToggle.setButtonText("");
  addAndMakeVisible(eqToggle);

  normalizeNamOutputLabel.setText("Nornamlize NAM", juce::dontSendNotification);
  addAndMakeVisible(normalizeNamOutputLabel);
  normalizeNamOutput.setButtonText("");
  addAndMakeVisible(normalizeNamOutput);

  normalizeIrOutputLabel.setText("Nornamlize IR", juce::dontSendNotification);
  addAndMakeVisible(normalizeIrOutputLabel);
  normalizeIrOutput.setButtonText("");
  addAndMakeVisible(normalizeIrOutput);
  */
}

NeuralAmpEditor::~NeuralAmpEditor() {}

void NeuralAmpEditor::paint(juce::Graphics& g) {
  // g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
  // g.setColour(juce::Colours::black);
  // g.setFont(15.0f);
}

void NeuralAmpEditor::resized() {
  auto bounds = getLocalBounds();

  /*
  auto leftPanel = bounds.removeFromLeft(bounds.getWidth() / 2);

  auto rowHeight = 40;  // Increased for better spacing
  auto labelWidth = 150;

  auto row = leftPanel.removeFromTop(rowHeight);
  inputGainLabel.setBounds(row.removeFromLeft(labelWidth));
  inputGainSlider.setBounds(row);

  row = leftPanel.removeFromTop(rowHeight);
  outputGainLabel.setBounds(row.removeFromLeft(labelWidth));
  outputGainSlider.setBounds(row);

  row = leftPanel.removeFromTop(rowHeight);
  bassLabel.setBounds(row.removeFromLeft(labelWidth));
  bassSlider.setBounds(row);

  row = leftPanel.removeFromTop(rowHeight);
  midLabel.setBounds(row.removeFromLeft(labelWidth));
  midSlider.setBounds(row);

  row = leftPanel.removeFromTop(rowHeight);
  trebleLabel.setBounds(row.removeFromLeft(labelWidth));
  trebleSlider.setBounds(row);

  row = leftPanel.removeFromTop(rowHeight);
  noiseGateThresholdLabel.setBounds(row.removeFromLeft(labelWidth));
  noiseGateThresholdSlider.setBounds(row);

  row = leftPanel.removeFromTop(rowHeight);
  noiseGateLabel.setBounds(row.removeFromLeft(labelWidth));
  noiseGateToggle.setBounds(row);

  row = leftPanel.removeFromTop(rowHeight);
  irToggleLabel.setBounds(row.removeFromLeft(labelWidth));
  irToggle.setBounds(row);

  row = leftPanel.removeFromTop(rowHeight);
  eqToggleLabel.setBounds(row.removeFromLeft(labelWidth));
  eqToggle.setBounds(row);

  row = leftPanel.removeFromTop(rowHeight);
  normalizeNamOutputLabel.setBounds(row.removeFromLeft(labelWidth));
  normalizeNamOutput.setBounds(row);

  row = leftPanel.removeFromTop(rowHeight);
  normalizeIrOutputLabel.setBounds(row.removeFromLeft(labelWidth));
  normalizeIrOutput.setBounds(row);

  row = leftPanel.removeFromTop(rowHeight);
  modelDropdownLabel.setBounds(row.removeFromLeft(labelWidth));
  modelDropdown.setBounds(row);

  row = leftPanel.removeFromTop(rowHeight);
  irDropdownLabel.setBounds(row.removeFromLeft(labelWidth));
  irDropdown.setBounds(row);
  */

  webView->setBounds(bounds);
}

/*
// Add Nam items to dropdown
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

// Add IR items to dropdown
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
*/

// Get the WebView UI resources from BinaryData
std::optional<juce::WebBrowserComponent::Resource> NeuralAmpEditor::getResource(
    const juce::String& url) {
  juce::Logger::writeToLog("Requested URL: " + url);

  // Extract filename and normalize to match BinaryData naming
  juce::String filename = juce::URL(url).getFileName().trim();
  juce::String resourceName = filename.removeCharacters("-").replaceCharacter('.', '_');

  int size = 0;
  const char* data = BinaryData::getNamedResource(resourceName.toRawUTF8(), size);

  if (data == nullptr || size <= 0) {
    juce::Logger::writeToLog("Resource not found or empty: " + resourceName);
    return std::nullopt;
  }

  std::vector<std::byte> content(static_cast<size_t>(size));
  std::memcpy(content.data(), data, static_cast<size_t>(size));

  juce::String ext = filename.fromLastOccurrenceOf(".", false, false).toLowerCase();
  juce::String mime = getMimeForExtension(ext);
  if (mime.isEmpty())
    mime = "application/octet-stream";

  juce::Logger::writeToLog("Returning resource: " + resourceName + " (" + mime + ")");
  return juce::WebBrowserComponent::Resource{std::move(content), mime};
}

// Map file extensions to MIME types for serving embedded resources in the WebView UI
juce::String NeuralAmpEditor::getMimeForExtension(const juce::String& extension) {
  static const std::unordered_map<juce::String, juce::String> mimeMap = {
      {"htm", "text/html"},
      {"html", "text/html"},
      {"txt", "text/plain"},
      {"jpg", "image/jpeg"},
      {"jpeg", "image/jpeg"},
      {"svg", "image/svg+xml"},
      {"ico", "image/vnd.microsoft.icon"},
      {"json", "application/json"},
      {"png", "image/png"},
      {"css", "text/css"},
      {"map", "application/json"},
      {"js", "text/javascript"},
      {"woff2", "font/woff2"}};

  const auto lower = extension.toLowerCase();

  if (const auto it = mimeMap.find(lower); it != mimeMap.end())
    return it->second;

  jassertfalse;
  return "application/octet-stream";
}