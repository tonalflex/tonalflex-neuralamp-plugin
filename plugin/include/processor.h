#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <map>
#include <atomic>
#include <memory>
#include "NAM/dsp.h"
#include "NAM/get_dsp.h"
#include "NAM/activations.h"
#include "NAM/convnet.h"
#include "NAM/lstm.h"
#include "NAM/util.h"
#include "NAM/wavenet.h"

class NeuralAmpProcessor : public juce::AudioProcessor {
public:
  NeuralAmpProcessor();
  ~NeuralAmpProcessor() override;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  using AudioProcessor::processBlock;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override;

  const juce::String getName() const override;
  double getTailLengthSeconds() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int) override;
  const juce::String getProgramName(int) override;
  void changeProgramName(int, const juce::String&) override;

  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;

  juce::AudioProcessorValueTreeState parameters;

  juce::AudioProcessorValueTreeState& getParameters() { return parameters; }

  void loadNamFile(const juce::String& filePath);
  void loadIrFile(const juce::File& irFile);

  const juce::StringArray& getModelNames() const;
  const juce::StringArray& getIrNames() const;
  const std::vector<juce::String>& getModelPaths() const;
  const std::vector<juce::String>& getIrPaths() const;

  int getCurrentModelIndex() const { return currentModelIndex; }
  int getCurrentIrIndex() const { return currentIrIndex; }
  void setCurrentModelIndex(int index);
  void setCurrentIrIndex(int index);

  bool isModelLoaded() const { return modelLoaded; }
  bool isIrLoaded() const { return irLoaded; }

private:
  // Path to NAM and IR folder
  static constexpr const char* NamFolder = "/home/mind/NAM";
  static constexpr const char* IrFolder = "/home/mind/IR";

  juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

  static void initModelNamesAndPaths();
  static void initIrNamesAndPaths();
  static juce::StringArray getSortedNamModelNames(const juce::File& namFolder,
                                                  std::vector<juce::String>& modelPaths);
  static juce::StringArray getSortedIrNames(const juce::File& irFolder,
                                            std::vector<juce::String>& irPaths);
  std::shared_ptr<nam::DSP> dsp;
  std::mutex dspMutex;
  std::atomic<bool> modelLoaded{false};
  std::atomic<bool> irLoaded{false};
  std::atomic<bool> normalizeIr{true};

  // cache
  float cInputLevel;
  float cOutputLevel;
  float cToneBass;
  float cToneMid;
  float cToneTreble;
  bool cEqToggle;
  bool cNoiseGateToggle;
  float cNoiseGateThreshold;
  int cSelectedNamModel;
  int cSelectedIR;
  bool cIrToggle;
  bool cNormalizeNamOutput;
  bool cNormalizeIrOutput;
  float cTargetLoudness;

  void updateCachedParameters();

  juce::LinearSmoothedValue<float> normalizationGainSmoother;

  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>
      dcBlockerLeft;
  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>
      dcBlockerRight;

  juce::dsp::Convolution irConvolverLeft;
  juce::dsp::Convolution irConvolverRight;
  bool irEnabled = true;

  static juce::StringArray modelNames;
  static juce::StringArray irNames;
  static std::vector<juce::String> modelPathsByIndex;
  static std::vector<juce::String> irPathsByIndex;
  static bool modelPathsInitialized;
  static bool irPathsInitialized;
  int currentModelIndex = -1;  // -1 = "No Model"
  int currentIrIndex = -1;     // -1 = "No Model"
  juce::CriticalSection modelLoadLock;
  juce::CriticalSection irLoadLock;

  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>
      bassFilter;
  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>
      midFilter;
  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>
      trebleFilter;

  std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
  juce::AudioBuffer<float> oversampleBuffer;
  double modelSampleRate = 48000.0;  // Default, updated dynamically in prepareToPlay
  bool bypassResampling = true;      // Default to bypass unless model requires specific rate

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeuralAmpProcessor)
};