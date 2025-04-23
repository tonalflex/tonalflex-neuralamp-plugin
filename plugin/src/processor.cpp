#include "processor.h"
#include "editor.h"
#include <algorithm>
#include <cmath>

// Static member initialization
juce::StringArray NeuralAmpProcessor::modelNames;
std::vector<juce::String> NeuralAmpProcessor::modelPathsByIndex;
bool NeuralAmpProcessor::modelPathsInitialized = false;

juce::StringArray NeuralAmpProcessor::getSortedNamModelNames(
    const juce::File& namFolder,
    std::vector<juce::String>& modelPaths) {
  juce::StringArray modelNames;
  modelNames.add("No Model");
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

  DBG("Found " << modelNames.size() - 1 << " NAM models");
  return modelNames;
}

void NeuralAmpProcessor::initModelNamesAndPaths() {
  if (!modelPathsInitialized) {
    juce::File namDir("/Users/simonthorell/Code/tonalflex/tonalflex-neuralamp-plugin/NAM");
    modelNames = getSortedNamModelNames(namDir, modelPathsByIndex);
    modelPathsInitialized = true;
    DBG("Initialized with " << modelNames.size() << " model choices");
  }
}

NeuralAmpProcessor::NeuralAmpProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, juce::Identifier("PARAMETERS"), createParameterLayout()),
      bassFilter(juce::dsp::IIR::Coefficients<float>::makeLowShelf(44100, 100.0f, 1.0f, 1.0f)),
      midFilter(juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 1000.0f, 1.0f, 1.0f)),
      trebleFilter(juce::dsp::IIR::Coefficients<float>::makeHighShelf(44100, 4000.0f, 1.0f, 1.0f)) {
  DBG("NeuralAmpProcessor constructed");
}

const juce::StringArray& NeuralAmpProcessor::getModelNames() const {
  return modelNames;
}

const std::vector<juce::String>& NeuralAmpProcessor::getModelPaths() const {
  return modelPathsByIndex;
}

juce::AudioProcessorValueTreeState::ParameterLayout NeuralAmpProcessor::createParameterLayout() {
  initModelNamesAndPaths();
  juce::AudioProcessorValueTreeState::ParameterLayout layout;

  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "inputLevel", "Input Level", juce::NormalisableRange<float>(-20.0f, 20.0f, 0.1f), 0.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "toneBass", "Bass", juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f), 5.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "toneMid", "Middle", juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f), 5.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "toneTreble", "Treble", juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f), 5.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "outputLevel", "Output Level", juce::NormalisableRange<float>(-40.0f, 40.0f, 0.1f), 0.0f));
  layout.add(
      std::make_unique<juce::AudioParameterBool>("noiseGateActive", "Noise Gate Active", true));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "noiseGateThreshold", "Noise Gate Threshold",
      juce::NormalisableRange<float>(-100.0f, 0.0f, 0.1f), -80.0f));
  layout.add(std::make_unique<juce::AudioParameterBool>("eqActive", "EQ Active", true));
  layout.add(std::make_unique<juce::AudioParameterBool>("irToggle", "IR Toggle", true));
  layout.add(
      std::make_unique<juce::AudioParameterBool>("calibrateInput", "Calibrate Input", false));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "inputCalibrationLevel", "Input Calibration Level",
      juce::NormalisableRange<float>(-60.0f, 60.0f, 0.1f), 12.0f));
  layout.add(std::make_unique<juce::AudioParameterChoice>(
      "outputMode", "Output Mode", juce::StringArray{"Raw", "Normalized", "Calibrated"}, 1));

  juce::StringArray choices = modelNames.isEmpty() ? juce::StringArray("No Model") : modelNames;
  layout.add(
      std::make_unique<juce::AudioParameterChoice>("selectedNamModel", "NAM Model", choices, 0));

  DBG("Parameter layout created with " << choices.size() << " model choices");
  return layout;
}

NeuralAmpProcessor::~NeuralAmpProcessor() {
  DBG("NeuralAmpProcessor destroyed");
}

void NeuralAmpProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  DBG("Preparing to play: sampleRate=" << sampleRate << ", samplesPerBlock=" << samplesPerBlock);

  // auto localDsp = dsp.load();  // ✅ atomic-safe read
  std::shared_ptr<nam::DSP> localDsp;
  {
    std::lock_guard<std::mutex> lock(dspMutex);
    localDsp = dsp;
  }

  if (localDsp) {
    localDsp->Reset(sampleRate, samplesPerBlock);
    DBG("DSP reset successfully");
  } else {
    DBG("No DSP loaded; skipping DSP reset");
  }

  juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2};
  bassFilter.prepare(spec);
  midFilter.prepare(spec);
  trebleFilter.prepare(spec);
}

void NeuralAmpProcessor::releaseResources() {
  DBG("Releasing resources");
  bassFilter.reset();
  midFilter.reset();
  trebleFilter.reset();
}

void NeuralAmpProcessor::setCurrentModelIndex(int index) {
  const juce::ScopedLock lock(modelLoadLock);
  if (index == currentModelIndex) {
    DBG("Model index " << index << " already selected; ignoring");
    return;
  }

  // Clamp index to valid range: -1 (No Model) to modelNames.size() - 2 (last model)
  index = std::clamp(index, -1, static_cast<int>(modelNames.size()) - 2);
  DBG("Setting model index: " << index);

  currentModelIndex = index;

  if (index >= 0 && index < static_cast<int>(modelPathsByIndex.size()) &&
      modelPathsByIndex[static_cast<size_t>(index)].isNotEmpty()) {
    DBG("Loading NAM model from path: " << modelPathsByIndex[static_cast<size_t>(index)]);
    loadNamFromFile(modelPathsByIndex[static_cast<size_t>(index)]);
  }

  // Update parameter
  auto* param = parameters.getParameter("selectedNamModel");
  int paramIndex = index + 1;  // Map -1 to 0 ("No Model"), 0 to 1, etc.
  if (param && paramIndex >= 0 && paramIndex < modelNames.size()) {
    DBG("Updating selectedNamModel to paramIndex: " << paramIndex);
    param->beginChangeGesture();
    param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(paramIndex)));
    param->endChangeGesture();
  } else {
    DBG("Invalid parameter index: " << paramIndex << " (modelNames.size: " << modelNames.size()
                                    << ")");
  }
}

void NeuralAmpProcessor::addModel(const juce::String& filePath) {
  juce::File file(filePath);
  if (!file.existsAsFile() || !file.hasFileExtension(".nam")) {
    DBG("Invalid model file: " << filePath);
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Invalid File",
                                           "The selected file is not a valid .nam file.");
    return;
  }

  juce::String name = file.getFileNameWithoutExtension();
  if (name.isEmpty()) {
    DBG("Invalid model name for file: " << filePath);
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Invalid Name",
                                           "The selected file has an invalid name.");
    return;
  }

  const juce::ScopedLock lock(modelLoadLock);
  auto* param = parameters.getParameter("selectedNamModel");
  int maxModels =
      param ? dynamic_cast<juce::AudioParameterChoice*>(param)->choices.size() : modelNames.size();
  if (modelNames.size() >= maxModels) {
    DBG("Maximum model limit reached: " << maxModels);
    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::WarningIcon, "Model Limit Reached",
        "Cannot add more models. Maximum limit is " + juce::String(maxModels) + ".");
    return;
  }

  auto it = std::find(modelPathsByIndex.begin(), modelPathsByIndex.end(), filePath);
  if (it != modelPathsByIndex.end()) {
    int index = static_cast<int>(std::distance(modelPathsByIndex.begin(), it));
    DBG("Model already exists at index: " << index);
    setCurrentModelIndex(index - 1);
    return;
  }

  modelNames.add(name);
  modelPathsByIndex.push_back(filePath);
  DBG("Added new model: " << name << " at path: " << filePath);

  setCurrentModelIndex(static_cast<int>(modelPathsByIndex.size()) - 2);
}

void NeuralAmpProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
  juce::ScopedNoDenormals noDenormals;

  const int numSamples = buffer.getNumSamples();
  const int numChannels = buffer.getNumChannels();

  if (numSamples <= 0 || numChannels <= 0) {
    DBG("Invalid buffer: samples=" << numSamples << ", channels=" << numChannels);
    buffer.clear();
    return;
  }

  float inputGain =
      juce::Decibels::decibelsToGain(parameters.getRawParameterValue("inputLevel")->load());
  float outputGain =
      juce::Decibels::decibelsToGain(parameters.getRawParameterValue("outputLevel")->load());
  float bassGain = *parameters.getRawParameterValue("toneBass") / 5.0f;      // Scale 0-10 to 0-2
  float midGain = *parameters.getRawParameterValue("toneMid") / 5.0f;        // Scale 0-10 to 0-2
  float trebleGain = *parameters.getRawParameterValue("toneTreble") / 5.0f;  // Scale 0-10 to 0-2
  bool eqActive = *parameters.getRawParameterValue("eqActive") > 0.5f;
  bool noiseGateActive = *parameters.getRawParameterValue("noiseGateActive") > 0.5f;
  float noiseGateThreshold = *parameters.getRawParameterValue("noiseGateThreshold");

  if (!std::isfinite(inputGain) || !std::isfinite(outputGain) || !std::isfinite(bassGain) ||
      !std::isfinite(midGain) || !std::isfinite(trebleGain) || !std::isfinite(noiseGateThreshold)) {
    DBG("Invalid parameter values detected; clearing buffer");
    buffer.clear();
    return;
  }

  for (int channel = 0; channel < numChannels; ++channel) {
    auto* channelData = buffer.getWritePointer(channel);
    for (int i = 0; i < numSamples; ++i) {
      channelData[i] *= inputGain;
    }
  }

  if (noiseGateActive) {
    for (int channel = 0; channel < numChannels; ++channel) {
      auto* channelData = buffer.getWritePointer(channel);
      for (int i = 0; i < numSamples; ++i) {
        if (std::abs(channelData[i]) < juce::Decibels::decibelsToGain(noiseGateThreshold))
          channelData[i] = 0.0f;
      }
    }
  }

  // auto localDsp = dsp.load();  // atomic load
  std::shared_ptr<nam::DSP> localDsp;
  {
    std::lock_guard<std::mutex> lock(dspMutex);
    localDsp = dsp;
  }

  if (modelLoaded.load() && localDsp) {
    try {
      std::vector<double> input(numSamples);
      std::vector<double> output(numSamples);
      for (int channel = 0; channel < numChannels; ++channel) {
        auto* channelData = buffer.getWritePointer(channel);
        for (int i = 0; i < numSamples; ++i) {
          input[i] = static_cast<double>(channelData[i]);
        }

        // ✅ safe: using a stable localDsp reference
        localDsp->process(input.data(), output.data(), numSamples);

        for (int i = 0; i < numSamples; ++i) {
          channelData[i] = static_cast<float>(output[i]);
          if (!std::isfinite(channelData[i])) {
            DBG("Non-finite output detected at sample " << i);
            buffer.clear();
            return;
          }
        }
      }
    } catch (const std::exception& e) {
      DBG("Error in DSP processing: " << e.what());
      buffer.clear();
      return;
    }
  }

  if (eqActive) {
    auto bassCoeffs =
        juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), 100.0f, 1.0f, bassGain);
    auto midCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), 1000.0f,
                                                                         1.0f, midGain);
    auto trebleCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(getSampleRate(), 4000.0f,
                                                                           1.0f, trebleGain);

    if (bassCoeffs && midCoeffs && trebleCoeffs) {
      *bassFilter.state = *bassCoeffs;
      *midFilter.state = *midCoeffs;
      *trebleFilter.state = *trebleCoeffs;

      juce::dsp::AudioBlock<float> block(buffer);
      juce::dsp::ProcessContextReplacing<float> context(block);
      bassFilter.process(context);
      midFilter.process(context);
      trebleFilter.process(context);
    } else {
      DBG("Invalid EQ coefficients; skipping EQ");
    }
  }

  for (int channel = 0; channel < numChannels; ++channel) {
    auto* channelData = buffer.getWritePointer(channel);
    for (int i = 0; i < numSamples; ++i) {
      channelData[i] *= outputGain;
      if (!std::isfinite(channelData[i])) {
        DBG("Non-finite output after gain at sample " << i);
        buffer.clear();
        return;
      }
    }
  }
}

void NeuralAmpProcessor::processBlock(juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midi) {
  buffer.clear();
}

juce::AudioProcessorEditor* NeuralAmpProcessor::createEditor() {
  return new NeuralAmpEditor(*this);
}

void NeuralAmpProcessor::getStateInformation(juce::MemoryBlock& destData) {
  juce::ignoreUnused(destData);
}

void NeuralAmpProcessor::setStateInformation(const void* data, int sizeInBytes) {
  juce::ignoreUnused(data, sizeInBytes);
}

void NeuralAmpProcessor::loadNamFromFile(const juce::String& filePath) {
  juce::File file(filePath);
  if (!file.existsAsFile()) {
    DBG("Error: File does not exist: " << filePath);
    return;
  }
  DBG("Loading NAM model from: " << filePath);
  try {
    auto newDsp = nam::get_dsp(filePath.toStdString());
    std::unique_ptr<nam::DSP> rawDsp = nam::get_dsp(filePath.toStdString());

    if (rawDsp) {
      rawDsp->Reset(getSampleRate(), 512);

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
  }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new NeuralAmpProcessor();
}