#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_basics/juce_gui_basics.h>
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
  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  void processBlock(juce::AudioBuffer<double>&, juce::MidiBuffer&) override;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override { return true; }

  const juce::String getName() const override { return "NeuralAmp"; }
  double getTailLengthSeconds() const override { return 0.0; }

  bool acceptsMidi() const override { return false; }
  bool producesMidi() const override { return false; }

  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return {}; }
  void changeProgramName(int, const juce::String&) override {}

  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;

  juce::AudioProcessorValueTreeState& getParameters() { return parameters; }
  void loadNamFromFile(const juce::String& filePath);
  void addModel(const juce::String& filePath);

  const juce::StringArray& getModelNames() const;
  const std::vector<juce::String>& getModelPaths() const;

  int getCurrentModelIndex() const { return currentModelIndex; }
  void setCurrentModelIndex(int index);

  bool isModelLoaded() const { return modelLoaded; }

private:
  juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

  static void initModelNamesAndPaths();
  static juce::StringArray getSortedNamModelNames(const juce::File& namFolder,
                                                  std::vector<juce::String>& modelPaths);

  std::shared_ptr<nam::DSP> dsp;
  std::mutex dspMutex;
  std::atomic<bool> modelLoaded{false};

  juce::AudioProcessorValueTreeState parameters;

  static juce::StringArray modelNames;
  static std::vector<juce::String> modelPathsByIndex;
  static bool modelPathsInitialized;
  int currentModelIndex = -1;  // -1 = "No Model"
  juce::CriticalSection modelLoadLock;

  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>
      bassFilter;
  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>
      midFilter;
  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>
      trebleFilter;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeuralAmpProcessor)
};