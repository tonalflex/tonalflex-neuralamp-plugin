#include "processor.h"
#if !HEADLESS
#include "editor.h"
#endif
#include <cmath>

// ##############################
// STORE NAM AND IR NAMES / PATHS
// ##############################
juce::StringArray NeuralAmpProcessor::modelNames;
std::vector<juce::String> NeuralAmpProcessor::modelPathsByIndex;
bool NeuralAmpProcessor::modelPathsInitialized = false;

juce::StringArray NeuralAmpProcessor::getSortedNamModelNames(
    const juce::File& namFolder,
    std::vector<juce::String>& modelPaths) {
  juce::StringArray modelNames;
  modelNames.add("Select NAM model...");
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
      // DBG("Added model: " << name << " at path: " << file.getFullPathName());
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
  names.add("Select IR model...");
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
      // DBG("Added IR: " << name << " from path: " << file.getFullPathName());
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
    juce::File namDir(NamFolder);
    modelNames = getSortedNamModelNames(namDir, modelPathsByIndex);
    modelPathsInitialized = true;
    DBG("Initialized with " << modelNames.size() << " Nam choices");
  }
}

void NeuralAmpProcessor::initIrNamesAndPaths() {
  if (!irPathsInitialized) {
    juce::File irDir(IrFolder);
    irNames = getSortedIrNames(irDir, irPathsByIndex);
    irPathsInitialized = true;
  }
}

// ##############
// MAIN PROCESSOR
// ##############
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
  namLoaderThread = std::make_unique<NamLoaderThread>(*this);
  irLoaderThread = std::make_unique<IrLoaderThread>(*this);
  DBG("NeuralAmpProcessor constructed");
}
juce::AudioProcessorValueTreeState::ParameterLayout NeuralAmpProcessor::createParameterLayout() {
  initModelNamesAndPaths();
  initIrNamesAndPaths();
  juce::AudioProcessorValueTreeState::ParameterLayout layout;
  layout.add(
      std::make_unique<juce::AudioParameterFloat>("inputLevel", "inputLevel", 0.0f, 1.0f, 0.5f));
  layout.add(
      std::make_unique<juce::AudioParameterFloat>("toneBass", "toneBass", 0.01f, 10.0f, 5.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>("toneMid", "toneMid", 0.01f, 10.0f, 5.0f));
  layout.add(
      std::make_unique<juce::AudioParameterFloat>("toneTreble", "toneTreble", 0.01f, 10.0f, 5.0f));
  layout.add(
      std::make_unique<juce::AudioParameterFloat>("outputLevel", "outputLevel", 0.0f, 1.0f, 0.5f));
  layout.add(
      std::make_unique<juce::AudioParameterBool>("noiseGateToggle", "noiseGateToggle", true));
  layout.add(std::make_unique<juce::AudioParameterFloat>("noiseGateThreshold", "noiseGateThreshold",
                                                         -100.0f, 0.0f, -80.0f));
  layout.add(std::make_unique<juce::AudioParameterBool>("eqToggle", "eqToggle", true));
  layout.add(std::make_unique<juce::AudioParameterBool>("irToggle", "irToggle", true));
  layout.add(
      std::make_unique<juce::AudioParameterBool>("normalizeNamOutput", "normalizeNamOutput", true));
  layout.add(std::make_unique<juce::AudioParameterFloat>("targetLoudness", "targetLoudness", -60.0f,
                                                         0.0f, -18.0f));
  layout.add(
      std::make_unique<juce::AudioParameterBool>("normalizeIrOutput", "normalizeIrOutput", true));

  juce::StringArray Namchoices = modelNames.isEmpty() ? juce::StringArray("No Model") : modelNames;
  layout.add(std::make_unique<juce::AudioParameterChoice>("selectedNamModel", "selectedNamModel",
                                                          Namchoices, 0));

  juce::StringArray irChoices = irNames.isEmpty() ? juce::StringArray("No IR") : irNames;
  layout.add(
      std::make_unique<juce::AudioParameterChoice>("selectedIR", "selectedIR", irChoices, 0));
  return layout;
}

NeuralAmpProcessor::~NeuralAmpProcessor() {
  releaseResources();
  juce::Logger::writeToLog("[Processor] Destructor called");
}

// #############
// JUCE SETTINGS
// #############
const juce::String NeuralAmpProcessor::getName() const {
  return "NeuralAmp";
}

bool NeuralAmpProcessor::acceptsMidi() const {
  return false;
}
bool NeuralAmpProcessor::producesMidi() const {
  return false;
}
bool NeuralAmpProcessor::isMidiEffect() const {
  return false;
}
double NeuralAmpProcessor::getTailLengthSeconds() const {
  return 0.0;
}

int NeuralAmpProcessor::getNumPrograms() {
  return 1;
}
int NeuralAmpProcessor::getCurrentProgram() {
  return 0;
}
void NeuralAmpProcessor::setCurrentProgram(int index) {
  juce::ignoreUnused(index);
}
const juce::String NeuralAmpProcessor::getProgramName(int index) {
  juce::ignoreUnused(index);
  return {};
}
void NeuralAmpProcessor::changeProgramName(int index, const juce::String& newName) {
  juce::ignoreUnused(index, newName);
}

void NeuralAmpProcessor::getStateInformation(juce::MemoryBlock& destData) {
  juce::ignoreUnused(destData);
}

void NeuralAmpProcessor::setStateInformation(const void* data, int sizeInBytes) {
  juce::ignoreUnused(data, sizeInBytes);
}

// #############################
// PREPARATIONS BEFORE RT THREAD
// #############################
void NeuralAmpProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  if (namLoaderThread)
    namLoaderThread->startThread();
  if (irLoaderThread)
    irLoaderThread->startThread();
  resampleInputBuffer.resize(samplesPerBlock + 16);
  resampleOutputBuffer.resize(samplesPerBlock + 16);
  modelSampleRate = sampleRate;  // Set to DAW's sample rate
  bypassResampling = false;      // Default to bypass unless model requires specific rate
  DBG("Preparing to play: sampleRate=" << sampleRate << ", samplesPerBlock=" << samplesPerBlock);
  DBG("Model sample rate set to: " << modelSampleRate);

  cInputLevel = parameters.getRawParameterValue("inputLevel")->load();
  cOutputLevel = parameters.getRawParameterValue("outputLevel")->load();
  cToneBass = parameters.getRawParameterValue("toneBass")->load();
  cToneMid = parameters.getRawParameterValue("toneMid")->load();
  cToneTreble = parameters.getRawParameterValue("toneTreble")->load();
  cEqToggle = parameters.getRawParameterValue("eqToggle")->load() > 0.5f;
  cNoiseGateToggle = parameters.getRawParameterValue("noiseGateToggle")->load() > 0.5f;
  cNoiseGateThreshold = parameters.getRawParameterValue("noiseGateThreshold")->load();
  cSelectedNamModel = static_cast<int>(*parameters.getRawParameterValue("selectedNamModel"));
  cSelectedIR = static_cast<int>(*parameters.getRawParameterValue("selectedIR"));
  cIrToggle = parameters.getRawParameterValue("irToggle")->load() > 0.5f;
  cNormalizeNamOutput = parameters.getRawParameterValue("normalizeNamOutput")->load() > 0.5f;
  cNormalizeIrOutput = parameters.getRawParameterValue("normalizeIrOutput")->load() > 0.5f;
  cTargetLoudness = parameters.getRawParameterValue("targetLoudness")->load();

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

// #################
// RELEASE RESOURCES
// #################
void NeuralAmpProcessor::releaseResources() {
  juce::Logger::writeToLog("[Processor] releaseResources() called");

  if (namLoaderThread) {
    namLoaderThread->signalThreadShouldExit();
    namLoaderThread->stopThread(500);
  }

  if (irLoaderThread) {
    irLoaderThread->signalThreadShouldExit();
    irLoaderThread->stopThread(500);
  }

  bassFilter.reset();
  midFilter.reset();
  trebleFilter.reset();
}

bool NeuralAmpProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
}

// ####################
// UPDATE CACHED PARAMS
// ####################
void NeuralAmpProcessor::updateCachedParameters() {
  constexpr float epsilon = 1e-5f;

  auto input = parameters.getRawParameterValue("inputLevel")->load();
  if (std::abs(input - cInputLevel) > epsilon)
    cInputLevel = input;

  auto output = parameters.getRawParameterValue("outputLevel")->load();
  if (std::abs(output - cOutputLevel) > epsilon)
    cOutputLevel = output;

  auto bass = parameters.getRawParameterValue("toneBass")->load();
  if (std::abs(bass - cToneBass) > epsilon)
    cToneBass = bass;

  auto mid = parameters.getRawParameterValue("toneMid")->load();
  if (std::abs(mid - cToneMid) > epsilon)
    cToneMid = mid;

  auto treble = parameters.getRawParameterValue("toneTreble")->load();
  if (std::abs(treble - cToneTreble) > epsilon)
    cToneTreble = treble;

  bool eq = parameters.getRawParameterValue("eqToggle")->load() > 0.5f;
  if (eq != cEqToggle)
    cEqToggle = eq;

  bool ng = parameters.getRawParameterValue("noiseGateToggle")->load() > 0.5f;
  if (ng != cNoiseGateToggle)
    cNoiseGateToggle = ng;

  auto ngThresh = parameters.getRawParameterValue("noiseGateThreshold")->load();
  if (std::abs(ngThresh - cNoiseGateThreshold) > epsilon)
    cNoiseGateThreshold = ngThresh;

  int modelIndex = static_cast<int>(*parameters.getRawParameterValue("selectedNamModel"));
  if (modelIndex != cSelectedNamModel) {
    cSelectedNamModel = modelIndex;
    juce::String modelPath;
    if (modelIndex == 0) {
      modelPath = "";
    } else {
      modelPath = modelPathsByIndex[cSelectedNamModel];
    }

    namLoaderThread->requestLoadModel(modelPath);
  }

  int irIndex = static_cast<int>(*parameters.getRawParameterValue("selectedIR"));
  if (irIndex != cSelectedIR) {
    cSelectedIR = irIndex;
    juce::String irPath;
    if (irIndex == 0) {
      irPath = "";
    } else {
      irPath = irPathsByIndex[cSelectedIR];
    }

    irLoaderThread->requestLoadIr(irPath);
  }

  bool irToggle = parameters.getRawParameterValue("irToggle")->load() > 0.5f;
  if (irToggle != cIrToggle)
    cIrToggle = irToggle;

  bool normalizeNam = parameters.getRawParameterValue("normalizeNamOutput")->load() > 0.5f;
  if (normalizeNam != cNormalizeNamOutput)
    cNormalizeNamOutput = normalizeNam;

  bool normalizeIr = parameters.getRawParameterValue("normalizeIrOutput")->load() > 0.5f;
  if (normalizeIr != cNormalizeNamOutput)
    cNormalizeIrOutput = normalizeIr;

  auto tgtLoud = parameters.getRawParameterValue("targetLoudness")->load();
  if (std::abs(tgtLoud - cTargetLoudness) > epsilon)
    cTargetLoudness = tgtLoud;
}

// ################
// RT PROCESS BLOCK
// ################
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

  updateCachedParameters();

  float bassGain = cToneBass / 5.0f;
  float midGain = cToneMid / 5.0f;
  float trebleGain = cToneTreble / 5.0f;

  // ################
  // Apply input gain
  // ################
  for (size_t channel = 0; channel < numChannels; ++channel) {
    auto* channelData = buffer.getWritePointer(static_cast<int>(channel));
    for (int i = 0; i < numSamples; ++i) {
      channelData[i] *= cInputLevel;
    }
  }

  // ##########
  // Noise gate
  // ##########
  if (cNoiseGateToggle) {
    for (size_t channel = 0; channel < numChannels; ++channel) {
      auto* channelData = buffer.getWritePointer(static_cast<int>(channel));
      for (int i = 0; i < numSamples; ++i) {
        if (std::abs(channelData[i]) < juce::Decibels::decibelsToGain(cNoiseGateThreshold))
          channelData[i] = 0.0f;
      }
    }
  }

  // ##############
  // NAM processing
  // ##############
  std::shared_ptr<nam::DSP> localDsp;
  {
    std::lock_guard<std::mutex> lock(dspMutex);
    localDsp = dsp;
  }

  if (modelLoaded.load() && localDsp) {
    try {
      std::vector<double>* inputData = nullptr;
      std::vector<double>* outputData = nullptr;
      size_t inputFrames = static_cast<size_t>(numSamples);

      // Prepare input
      for (size_t i = 0; i < inputFrames; ++i) {
        float l = buffer.getSample(0, static_cast<int>(i));
        float r = (numChannels > 1) ? buffer.getSample(1, static_cast<int>(i)) : l;
        resampleInputBuffer[i] = 0.5f * (l + r);
      }

      // Resample if needed
      if (!bypassResampling) {
        const double ratio = modelSampleRate / getSampleRate();
        const int upsampled = upsampler.process(ratio, resampleInputBuffer.data(),
                                                resampleInputBuffer.data(), numSamples);
        inputData = new std::vector<double>(upsampled);
        for (int i = 0; i < upsampled; ++i)
          (*inputData)[i] = resampleInputBuffer[i];
        outputData = new std::vector<double>(upsampled);
      } else {
        inputData = new std::vector<double>(inputFrames);
        outputData = new std::vector<double>(inputFrames);
        for (size_t i = 0; i < inputFrames; ++i)
          (*inputData)[i] = resampleInputBuffer[i];
      }

      // Process
      localDsp->process(inputData->data(), outputData->data(), outputData->size());

      // Downsample if needed
      if (!bypassResampling) {
        const double ratio = getSampleRate() / modelSampleRate;
        const int downsampled =
            downsampler.process(ratio, reinterpret_cast<const float*>(outputData->data()),
                                resampleOutputBuffer.data(), outputData->size());
        for (int i = 0; i < downsampled; ++i) {
          float s = resampleOutputBuffer[i];
          buffer.setSample(0, i, s);
          if (numChannels > 1)
            buffer.setSample(1, i, s);
        }
      } else {
        for (int i = 0; i < numSamples; ++i) {
          float s = static_cast<float>((*outputData)[i]);
          buffer.setSample(0, i, s);
          if (numChannels > 1)
            buffer.setSample(1, i, s);
        }
      }

      delete inputData;
      delete outputData;
    } catch (const std::exception& e) {
      DBG("Error in DSP processing: " << e.what());
      buffer.clear();
    }
  }

  // #############
  // IR processing
  // #############
  if (cIrToggle && irLoaded) {
    juce::dsp::AudioBlock<float> irBlock(buffer);
    auto leftBlock = irBlock.getSingleChannelBlock(0);
    auto rightBlock = numChannels > 1 ? irBlock.getSingleChannelBlock(1) : leftBlock;
    irConvolverLeft.process(juce::dsp::ProcessContextReplacing<float>(leftBlock));
    if (numChannels > 1) {
      irConvolverRight.process(juce::dsp::ProcessContextReplacing<float>(rightBlock));
    }
  }

  // ##########
  // DC blocker
  // ##########
  juce::dsp::AudioBlock<float> block(buffer);
  juce::dsp::ProcessContextReplacing<float> context(block);
  dcBlockerLeft.process(context);
  if (numChannels > 1)
    dcBlockerRight.process(context);

  // Normalizer (in dB domain)
  if (cNormalizeNamOutput == 1 && localDsp) {
    float modelLoudnessDb = static_cast<float>(localDsp->GetLoudness());
    float targetLoudnessDb = *parameters.getRawParameterValue("targetLoudness");

    // Validate loudness range
    if (!std::isfinite(modelLoudnessDb) || modelLoudnessDb < -100.0f || modelLoudnessDb > 0.0f) {
      DBG("Invalid model loudness: " << modelLoudnessDb);
      modelLoudnessDb = targetLoudnessDb;
    }

    float gainAdjustmentDb = targetLoudnessDb - modelLoudnessDb;

    // Clamp to sane range (optional but safe)
    gainAdjustmentDb = juce::jlimit(-12.0f, 6.0f, gainAdjustmentDb);

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

  // #############
  // EQ Processing
  // #############
  if (cEqToggle) {
    auto bassCoeffs =
        juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), 250.0f, 1.0f, bassGain);
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

  // #################
  // Apply output gain
  // #################
  for (size_t channel = 0; channel < numChannels; ++channel) {
    auto* channelData = buffer.getWritePointer(static_cast<int>(channel));
    for (int i = 0; i < numSamples; ++i) {
      channelData[i] *= cOutputLevel;
    }
  }
}

// ##################
// ENABLE EDITOR / UI
// ##################
bool NeuralAmpProcessor::hasEditor() const {
  return true;
}

// ##############
// CREATE EDITOR
// ##############
juce::AudioProcessorEditor* NeuralAmpProcessor::createEditor() {
#if !HEADLESS
  return new NeuralAmpEditor(*this);
#else
  return nullptr;
#endif
}

// ##############
// LOAD NAM MODEL
// ##############
void NeuralAmpProcessor::loadNamFile(const juce::String& filePath) {
  DBG("NAM file path: " << filePath);
  if (filePath.trim().isEmpty()) {
    DBG("No NAM model selected — clearing DSP.");
    std::lock_guard<std::mutex> lock(dspMutex);
    dsp = nullptr;
    modelLoaded.store(false);
    return;
  }

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
      const double modelRate = rawDsp->GetExpectedSampleRate();
      const double hostRate = getSampleRate();

      if (std::abs(modelRate - hostRate) > 1.0) {
        DBG("Resampling required: Model SR = " << modelRate << ", Host SR = " << hostRate);
        bypassResampling = false;
      } else {
        DBG("No resampling needed.");
        bypassResampling = true;
      }

      modelSampleRate = modelRate;
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

// #############
// LOAD IR MODEL
// #############
void NeuralAmpProcessor::loadIrFile(const juce::String& filePath) {
  DBG("IR file path: " << filePath);
  if (filePath.trim().isEmpty()) {
    DBG("IR selection is 'None' – clearing IR.");
    const juce::ScopedLock lock(irLoadLock);
    irConvolverLeft.reset();
    irConvolverRight.reset();
    irLoaded = false;
    return;
  }

  juce::File irFile(filePath);
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

  const juce::ScopedLock lock(irLoadLock);
  DBG("Loading IR file: " << irFile.getFullPathName());

  try {
    bool normalize = cNormalizeIrOutput;
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

// #################################
// BACKGROUND THREAD FOR NAM LOADING
// #################################
void NamLoaderThread::run() {
  while (!threadShouldExit()) {
    wait(-1);  // wait indefinitely for notify()

    juce::String modelToLoad;
    {
      juce::ScopedLock lock(mutex);
      if (!modelRequested)
        continue;
      modelToLoad = requestedModelPath;
      modelRequested = false;
    }

    DBG("[Thread] Loading NAM model: " << modelToLoad);
    this->processor.loadNamFile(modelToLoad);
    DBG("[Thread] NAM Model load complete");
  }
}

// ################################
// BACKGROUND THREAD FOR IR LOADING
// ################################
void IrLoaderThread::run() {
  while (!threadShouldExit()) {
    wait(-1);  // wait indefinitely for notify()

    juce::String modelToLoad;
    {
      juce::ScopedLock lock(mutex);
      if (!modelRequested)
        continue;
      modelToLoad = requestedModelPath;
      modelRequested = false;
    }

    DBG("[Thread] Loading IR model: " << modelToLoad);
    this->processor.loadIrFile(modelToLoad);
    DBG("[Thread] IR Model load complete");
  }
}