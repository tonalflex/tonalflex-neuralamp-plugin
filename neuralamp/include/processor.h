#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <map>
#include "NAM/dsp.h"
#include "NAM/get_dsp.h"
#include "NAM/activations.h"
#include "NAM/convnet.h"
#include "NAM/lstm.h"
#include "NAM/util.h"
#include "NAM/wavenet.h"

class NeuralAmpProcessor : public juce::AudioProcessor
{
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

private:
    std::unique_ptr<nam::DSP> dsp;
    juce::AudioProcessorValueTreeState parameters;

    std::map<int, juce::String> modelIndexToPath;
    int currentModelIndex = -1;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // EQ filters
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> bassFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> midFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> trebleFilter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeuralAmpProcessor)
};
