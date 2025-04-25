#include "processor.h"
#if !HEADLESS
#include "editor.h"
#endif
#include <cmath>

// Static member initialization
juce::StringArray NeuralAmpProcessor::modelNames;
std::vector<juce::String> NeuralAmpProcessor::modelPathsByIndex;
bool NeuralAmpProcessor::modelPathsInitialized = false;

juce::StringArray NeuralAmpProcessor::getSortedNamModelNames(
    const juce::File& namFolder,
    std::vector<juce::String>& modelPaths) {
  juce::StringArray modelNames;
  modelNames.add("Select model...");
  modelPaths.push_back("");

  if (!namFolder.exists() || !namFolder.isDirectory()) {
    DBG("Error: NAM directory does not exist or is not accessible: "
        << namFolder.getFullPathName());
    return modelNames;
  }

  juce::Array<juce::File> files = namFolder.findChildFiles(juce::File::findFiles, false, "*.nam");
  if (files.isEmpty()) {
    DBG("Warning: No .nam files found in " << namFolder.getFullPathName());
  }

  std::sort(files.begin(), files.end(), [](const juce::File& a, const juce::File& b) {
    return a.getFileName().compareIgnoreCase(b.getFileName()) < 0;
  });

  for (int i = 0; i < files.size(); ++i) {
    const auto& file = files.getReference(i);
    auto name = file.getFileNameWithoutExtension();
    if (name.isNotEmpty()) {
      modelNames.add(name);
      modelPaths.push_back(file.getFullPathName());
      DBG("Added model: " << name << " at path: " << file.getFullPathName());
    } else {
      DBG("Skipped invalid model name for file: " << file.getFullPathName());
    }
  }
  return modelNames;
}

juce::StringArray NeuralAmpProcessor::irNames;
std::vector<juce::String> NeuralAmpProcessor::irPathsByIndex;
bool NeuralAmpProcessor::irPathsInitialized = false;

juce::StringArray NeuralAmpProcessor::getSortedIrNames(const juce::File& irFolder,
                                                       std::vector<juce::String>& irPaths) {
  juce::StringArray names;
  names.add("Select IR...");
  irPaths.push_back("");

  if (!irFolder.exists() || !irFolder.isDirectory()) {
    DBG("IR folder not accessible: " << irFolder.getFullPathName());
    return names;
  }

  juce::Array<juce::File> files = irFolder.findChildFiles(juce::File::findFiles, false, "*.wav");

  std::sort(files.begin(), files.end(), [](const juce::File& a, const juce::File& b) {
    return a.getFileName().compareIgnoreCase(b.getFileName()) < 0;
  });

  for (const auto& file : files) {
    auto name = file.getFileNameWithoutExtension();
    if (name.isNotEmpty()) {
      names.add(name);
      irPaths.push_back(file.getFullPathName());
      DBG("Added IR: " << name << " from path: " << file.getFullPathName());
    }
  }
  return names;
}

const juce::StringArray& NeuralAmpProcessor::getModelNames() const {
  return modelNames;
}

const std::vector<juce::String>& NeuralAmpProcessor::getModelPaths() const {
  return modelPathsByIndex;
}

const juce::StringArray& NeuralAmpProcessor::getIrNames() const {
  return irNames;
}

const std::vector<juce::String>& NeuralAmpProcessor::getIrPaths() const {
  return irPathsByIndex;
}

void NeuralAmpProcessor::initModelNamesAndPaths() {
  if (!modelPathsInitialized) {
    juce::File namDir("/home/rekz/Documents/NAM");
    modelNames = getSortedNamModelNames(namDir, modelPathsByIndex);
    modelPathsInitialized = true;
    DBG("Initialized with " << modelNames.size() << " Nam choices");
  }
}

void NeuralAmpProcessor::initIrNamesAndPaths() {
  if (!irPathsInitialized) {
    juce::File irDir("/home/rekz/Documents/IR");
    irNames = getSortedIrNames(irDir, irPathsByIndex);
    irPathsInitialized = true;
  }
}

NeuralAmpProcessor::NeuralAmpProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, juce::Identifier("PARAMETERS"), createParameterLayout()),
      bassFilter(juce::dsp::IIR::Coefficients<float>::makeLowShelf(48000, 100.0f, 1.0f, 1.0f)),
      midFilter(juce::dsp::IIR::Coefficients<float>::makePeakFilter(48000, 1000.0f, 1.0f, 1.0f)),
      trebleFilter(juce::dsp::IIR::Coefficients<float>::makeHighShelf(48000, 4000.0f, 1.0f, 1.0f)),
      oversampler(std::make_unique<juce::dsp::Oversampling<float>>(
          2,
          0,
          juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple)) {
  normalizationGainSmoother.reset(48000, 0.05f);
  DBG("NeuralAmpProcessor constructed");
}

juce::AudioProcessorValueTreeState::ParameterLayout NeuralAmpProcessor::createParameterLayout() {
  initModelNamesAndPaths();
  initIrNamesAndPaths();
  juce::AudioProcessorValueTreeState::ParameterLayout layout;

  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "inputLevel", "Input Level", juce::NormalisableRange<float>(-20.0f, 20.0f, 0.1f), -14.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "toneBass", "Bass", juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f), 5.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "toneMid", "Middle", juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f), 5.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "toneTreble", "Treble", juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f), 5.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "outputLevel", "Output Level", juce::NormalisableRange<float>(-40.0f, 40.0f, 0.1f), -4.0f));
  layout.add(
      std::make_unique<juce::AudioParameterBool>("noiseGateActive", "Noise Gate Active", true));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "noiseGateThreshold", "Noise Gate Threshold",
      juce::NormalisableRange<float>(-100.0f, 0.0f, 0.1f), -80.0f));
  layout.add(std::make_unique<juce::AudioParameterBool>("eqActive", "EQ Active", true));
  layout.add(std::make_unique<juce::AudioParameterBool>("irToggle", "IR Toggle", true));
  layout.add(std::make_unique<juce::AudioParameterBool>("normalizeNamOutput",
                                                        "Normalize NAM Output", true));
  layout.add(
      std::make_unique<juce::AudioParameterBool>("calibrateInput", "Calibrate Input", false));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "inputCalibrationLevel", "Input Calibration Level",
      juce::NormalisableRange<float>(-60.0f, 60.0f, 0.1f), 12.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "targetLoudness", "Target Loudness", juce::NormalisableRange<float>(-30.0f, -6.0f, 0.1f),
      -18.0f));
  layout.add(
      std::make_unique<juce::AudioParameterBool>("normalizeIrOutput", "Normalize IR Output", true));
  juce::StringArray Namchoices = modelNames.isEmpty() ? juce::StringArray("No Model") : modelNames;
  layout.add(
      std::make_unique<juce::AudioParameterChoice>("selectedNamModel", "NAM Model", Namchoices, 0));
  juce::StringArray irChoices = irNames.isEmpty() ? juce::StringArray("No IR") : irNames;
  layout.add(
      std::make_unique<juce::AudioParameterChoice>("selectedIR", "Selected IR", irChoices, 0));

  DBG("Parameter layout created with " << Namchoices.size() << " model Namchoices");
  return layout;
}

NeuralAmpProcessor::~NeuralAmpProcessor() {
  DBG("NeuralAmpProcessor destroyed");
}

void NeuralAmpProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  modelSampleRate = sampleRate;  // Set to DAW's sample rate
  bypassResampling = true;       // Default to bypass unless model requires specific rate
  DBG("Preparing to play: sampleRate=" << sampleRate << ", samplesPerBlock=" << samplesPerBlock);
  DBG("Model sample rate set to: " << modelSampleRate);

  std::shared_ptr<nam::DSP> localDsp;
  {
    std::lock_guard<std::mutex> lock(dspMutex);
    localDsp = dsp;
  }

  if (localDsp) {
    localDsp->Reset(modelSampleRate, samplesPerBlock);
    DBG("DSP reset successfully");
  }

  juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2};
  bassFilter.prepare(spec);
  midFilter.prepare(spec);
  trebleFilter.prepare(spec);
  dcBlockerLeft.prepare(spec);
  dcBlockerRight.prepare(spec);

  *bassFilter.state =
      *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 100.0f, 1.0f, 1.0f);
  *midFilter.state =
      *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 1000.0f, 1.0f, 1.0f);
  *trebleFilter.state =
      *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 4000.0f, 1.0f, 1.0f);
  *dcBlockerLeft.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 20.0f);
  *dcBlockerRight.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 20.0f);

  // Reset filters to clear state
  bassFilter.reset();
  midFilter.reset();
  trebleFilter.reset();
  dcBlockerLeft.reset();
  dcBlockerRight.reset();

  // Initialize oversampler (used only if model requires specific rate)
  oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
      2, 0, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple);
  oversampler->initProcessing(static_cast<size_t>(samplesPerBlock));
  oversampler->reset();

  // Pre-allocate oversample buffer (conservative size for potential resampling)
  int maxOversampledFrames = samplesPerBlock + 8;  // Minimal padding
  oversampleBuffer.setSize(2, maxOversampledFrames, false, false, true);
  oversampleBuffer.clear();

  irConvolverLeft.prepare(spec);
  irConvolverRight.prepare(spec);

  normalizationGainSmoother.reset(sampleRate, 0.05f);  // Update smoother for current sample rate
  setLatencySamples(bypassResampling ? 0 : static_cast<int>(oversampler->getLatencyInSamples()));
}

void NeuralAmpProcessor::releaseResources() {
  DBG("Releasing resources");
  bassFilter.reset();
  midFilter.reset();
  trebleFilter.reset();
}

void NeuralAmpProcessor::setCurrentModelIndex(int comboBoxId) {
  const juce::ScopedLock lock(modelLoadLock);

  int index = comboBoxId - 1;

  if (index == currentModelIndex) {
    DBG("Model index " << index << " already selected; ignoring");
    return;
  }

  index = std::clamp(index, 0, static_cast<int>(modelNames.size()) - 1);
  DBG("Setting model index: " << index);

  currentModelIndex = index;

  if (index > 0 && index < static_cast<int>(modelPathsByIndex.size()) &&
      modelPathsByIndex[static_cast<size_t>(index)].isNotEmpty()) {
    DBG("Loading NAM model from path: " << modelPathsByIndex[static_cast<size_t>(index)]);
    loadNamFile(modelPathsByIndex[static_cast<size_t>(index)]);
  } else {
    DBG("Model index points to placeholder or invalid path");
  }

  auto* param = parameters.getParameter("selectedNamModel");
  if (param && index >= 0 && index < modelNames.size()) {
    DBG("Updating selectedNamModel to paramIndex: " << index);
    param->beginChangeGesture();
    param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(index)));
    param->endChangeGesture();
  } else {
    DBG("Invalid parameter index: " << index << " (modelNames.size: " << modelNames.size() << ")");
  }
}

void NeuralAmpProcessor::setCurrentIrIndex(int comboBoxId) {
  const juce::ScopedLock lock(irLoadLock);

  int index = comboBoxId - 1;

  if (index == currentIrIndex) {
    DBG("IR index " << index << " already selected; ignoring");
    return;
  }

  index = std::clamp(index, 0, static_cast<int>(irPathsByIndex.size()) - 1);
  currentIrIndex = index;

  if (index > 0 && index < static_cast<int>(irPathsByIndex.size()) &&
      irPathsByIndex[(size_t)index].isNotEmpty()) {
    DBG("Loading IR file: " << irPathsByIndex[(size_t)index]);
    loadIrFile(irPathsByIndex[(size_t)index]);
  } else {
    DBG("IR index points to placeholder or invalid path");
  }

  auto* param = parameters.getParameter("selectedIR");
  if (param && index >= 0 && index < irNames.size()) {
    DBG("Updating selectedIR to paramIndex: " << index);
    param->beginChangeGesture();
    param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(index)));
    param->endChangeGesture();
  } else {
    DBG("Invalid IR param index: " << index << " (irNames.size: " << irNames.size() << ")");
  }
}

void NeuralAmpProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
  juce::ScopedNoDenormals noDenormals;
  juce::ignoreUnused(midi);

  const int numSamples = buffer.getNumSamples();
  const size_t numChannels = static_cast<size_t>(buffer.getNumChannels());

  if (numSamples <= 0 || numChannels <= 0) {
    DBG("Invalid buffer: samples=" << numSamples << ", channels=" << numChannels);
    buffer.clear();
    return;
  }

  // Get parameter values
  float inputGain =
      juce::Decibels::decibelsToGain(parameters.getRawParameterValue("inputLevel")->load());
  float outputGain =
      juce::Decibels::decibelsToGain(parameters.getRawParameterValue("outputLevel")->load());
  float bassGain = *parameters.getRawParameterValue("toneBass") / 5.0f;
  float midGain = *parameters.getRawParameterValue("toneMid") / 5.0f;
  float trebleGain = *parameters.getRawParameterValue("toneTreble") / 5.0f;
  bool eqActive = *parameters.getRawParameterValue("eqActive") > 0.5f;
  bool noiseGateActive = *parameters.getRawParameterValue("noiseGateActive") > 0.5f;
  float noiseGateThreshold = *parameters.getRawParameterValue("noiseGateThreshold");

  // Apply input gain
  for (size_t channel = 0; channel < numChannels; ++channel) {
    auto* channelData = buffer.getWritePointer(static_cast<int>(channel));
    for (int i = 0; i < numSamples; ++i) {
      channelData[i] *= inputGain;
    }
  }

  // Noise gate
  if (noiseGateActive) {
    for (size_t channel = 0; channel < numChannels; ++channel) {
      auto* channelData = buffer.getWritePointer(static_cast<int>(channel));
      for (int i = 0; i < numSamples; ++i) {
        if (std::abs(channelData[i]) < juce::Decibels::decibelsToGain(noiseGateThreshold))
          channelData[i] = 0.0f;
      }
    }
  }

  std::shared_ptr<nam::DSP> localDsp;
  {
    std::lock_guard<std::mutex> lock(dspMutex);
    localDsp = dsp;
  }

  if (modelLoaded.load() && localDsp) {
    try {
      // Process directly at DAW's sample rate
      std::vector<double> input(static_cast<size_t>(numSamples));
      for (size_t i = 0; i < static_cast<size_t>(numSamples); ++i) {
        float l = buffer.getSample(0, static_cast<int>(i));
        float r = (numChannels > 1) ? buffer.getSample(1, static_cast<int>(i)) : l;
        input[i] = 0.5 * (l + r);
      }

      std::vector<double> output(static_cast<size_t>(numSamples));
      localDsp->process(input.data(), output.data(), static_cast<size_t>(numSamples));

      for (size_t i = 0; i < static_cast<size_t>(numSamples); ++i) {
        float s = static_cast<float>(output[i]);
        buffer.setSample(0, static_cast<int>(i), s);
        if (numChannels > 1)
          buffer.setSample(1, static_cast<int>(i), s);
      }
    } catch (const std::exception& e) {
      DBG("Error in DSP processing: " << e.what());
      buffer.clear();
      return;
    }
  }

  // DC blocker
  juce::dsp::AudioBlock<float> block(buffer);
  juce::dsp::ProcessContextReplacing<float> context(block);
  dcBlockerLeft.process(context);
  if (numChannels > 1)
    dcBlockerRight.process(context);

  // Normalizer
  int outputMode = static_cast<int>(parameters.getRawParameterValue("normalizeNamOutput")->load());
  if (outputMode == 1 && localDsp) {
    float modelLoudness = static_cast<float>(localDsp->GetLoudness());
    float targetLoudness = *parameters.getRawParameterValue("targetLoudness");

    if (!std::isfinite(modelLoudness) || modelLoudness < -120.0f || modelLoudness > 0.0f) {
      DBG("Invalid model loudness: " << modelLoudness);
      modelLoudness = targetLoudness;
    }

    float gainAdjustmentDb = targetLoudness - modelLoudness;
    float targetGain = juce::Decibels::decibelsToGain(gainAdjustmentDb);
    normalizationGainSmoother.setTargetValue(targetGain);

    for (int i = 0; i < numSamples; ++i) {
      float currentGain = normalizationGainSmoother.getNextValue();
      for (size_t channel = 0; channel < numChannels; ++channel) {
        auto* channelData = buffer.getWritePointer(static_cast<int>(channel));
        channelData[i] *= currentGain;
      }
    }
  }

  // IR processing
  bool irToggleOn = *parameters.getRawParameterValue("irToggle") > 0.5f;
  if (irToggleOn && irLoaded) {
    juce::dsp::AudioBlock<float> irBlock(buffer);
    auto leftBlock = irBlock.getSingleChannelBlock(0);
    auto rightBlock = numChannels > 1 ? irBlock.getSingleChannelBlock(1) : leftBlock;
    irConvolverLeft.process(juce::dsp::ProcessContextReplacing<float>(leftBlock));
    if (numChannels > 1) {
      irConvolverRight.process(juce::dsp::ProcessContextReplacing<float>(rightBlock));
    }
  }

  // EQ
  if (eqActive) {
    auto bassCoeffs =
        juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), 100.0f, 1.0f, bassGain);
    auto midCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), 1000.0f,
                                                                         1.0f, midGain);
    auto trebleCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(getSampleRate(), 4000.0f,
                                                                           1.0f, trebleGain);

    *bassFilter.state = *bassCoeffs;
    *midFilter.state = *midCoeffs;
    *trebleFilter.state = *trebleCoeffs;

    juce::dsp::AudioBlock<float> eqBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> eqContext(eqBlock);
    bassFilter.process(eqContext);
    midFilter.process(eqContext);
    trebleFilter.process(eqContext);
  }

  // Apply output gain
  for (size_t channel = 0; channel < numChannels; ++channel) {
    auto* channelData = buffer.getWritePointer(static_cast<int>(channel));
    for (int i = 0; i < numSamples; ++i) {
      channelData[i] *= outputGain;
    }
  }
}

void NeuralAmpProcessor::processBlock(juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midi) {
  juce::ignoreUnused(midi);  // Silence unused parameter warning
  buffer.clear();
}

juce::AudioProcessorEditor* NeuralAmpProcessor::createEditor() {
  return new NeuralAmpEditor(*this);
}

void NeuralAmpProcessor::getStateInformation(juce::MemoryBlock& destData) {
  auto state = parameters.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  xml->setAttribute("currentModelIndex", currentModelIndex);
  xml->setAttribute("currentIrIndex", currentIrIndex);
  copyXmlToBinary(*xml, destData);
}

void NeuralAmpProcessor::setStateInformation(const void* data, int sizeInBytes) {
  std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
  if (xml) {
    juce::ValueTree newState = juce::ValueTree::fromXml(*xml);
    parameters.replaceState(newState);
    setCurrentModelIndex(xml->getIntAttribute("currentModelIndex", -1));
    setCurrentIrIndex(xml->getIntAttribute("currentIrIndex", -1));
  }
}

void NeuralAmpProcessor::loadNamFile(const juce::String& filePath) {
  juce::File file(filePath);
  if (!file.existsAsFile()) {
    DBG("Error: File does not exist: " << filePath);
    modelLoaded.store(false);
    return;
  }
  DBG("Loading NAM model from: " << filePath);
  try {
    std::unique_ptr<nam::DSP> rawDsp = nam::get_dsp(filePath.toStdString());
    if (rawDsp) {
      rawDsp->Reset(modelSampleRate, getBlockSize());
      {
        std::lock_guard<std::mutex> lock(dspMutex);
        dsp = std::move(rawDsp);
      }
      modelLoaded.store(true);
      DBG("Model loaded successfully: " << filePath);
    } else {
      {
        std::lock_guard<std::mutex> lock(dspMutex);
        dsp = nullptr;
      }
      modelLoaded.store(false);
      DBG("Failed to load model: null DSP returned");
    }
  } catch (const std::exception& e) {
    DBG("Error loading model: " << e.what());
    modelLoaded.store(false);
  }
}

void NeuralAmpProcessor::loadIrFile(const juce::File& irFile) {
  if (!irFile.existsAsFile() || !irFile.hasFileExtension(".wav")) {
    DBG("Invalid IR file: " << irFile.getFullPathName());
    irLoaded = false;
    return;
  }

  juce::AudioFormatManager formatManager;
  formatManager.registerBasicFormats();
  std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(irFile));

  if (!reader) {
    DBG("Failed to read IR file: " << irFile.getFullPathName());
    irLoaded = false;
    return;
  }

  if (std::abs(reader->sampleRate - getSampleRate()) > 0.1) {
    DBG("IR sample rate (" << reader->sampleRate << ") does not match plugin sample rate ("
                           << getSampleRate() << ")");
    irLoaded = false;
    return;
  }

  const juce::ScopedLock lock(irLoadLock);
  DBG("Loading IR file: " << irFile.getFullPathName());

  try {
    bool normalize = *parameters.getRawParameterValue("normalizeIrOutput") > 0.5f;
    const size_t maxIrLength = 32768;  // Reduced for RPi4 memory efficiency

    irConvolverLeft.loadImpulseResponse(
        irFile, juce::dsp::Convolution::Stereo::no, juce::dsp::Convolution::Trim::yes, maxIrLength,
        normalize ? juce::dsp::Convolution::Normalise::yes : juce::dsp::Convolution::Normalise::no);

    irConvolverRight.loadImpulseResponse(
        irFile, juce::dsp::Convolution::Stereo::no, juce::dsp::Convolution::Trim::yes, maxIrLength,
        normalize ? juce::dsp::Convolution::Normalise::yes : juce::dsp::Convolution::Normalise::no);

    irLoaded = true;
    DBG("IR loaded successfully");
  } catch (const std::exception& e) {
    DBG("Error loading IR: " << e.what());
    irLoaded = false;
  }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new NeuralAmpProcessor();
}