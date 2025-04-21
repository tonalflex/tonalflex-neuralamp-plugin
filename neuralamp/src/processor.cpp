#include "processor.h"
#include "editor.h"
#include <algorithm>

static juce::StringArray getSortedNamModelNames(const juce::File& namFolder, std::map<int, juce::String>& modelPaths)
{
    juce::StringArray modelNames;

    if (!namFolder.isDirectory())
        return modelNames;

    juce::Array<juce::File> files = namFolder.findChildFiles(juce::File::findFiles, false, "*.nam");
    std::sort(files.begin(), files.end(), [](const juce::File& a, const juce::File& b) {
        return a.getFileName().compareIgnoreCase(b.getFileName()) < 0;
    });

    for (int i = 0; i < files.size(); ++i)
    {
        const auto& file = files.getReference(i);
        auto name = file.getFileNameWithoutExtension();
        modelNames.add(name);
        modelPaths[i] = file.getFullPathName();
    }

    return modelNames;
}

NeuralAmpProcessor::NeuralAmpProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout()),
      bassFilter(juce::dsp::IIR::Coefficients<float>::makeLowShelf(44100, 100.0f, 1.0f, 1.0f)),
      midFilter(juce::dsp::IIR::Coefficients<float>::makePeakFilter(44100, 1000.0f, 1.0f, 1.0f)),
      trebleFilter(juce::dsp::IIR::Coefficients<float>::makeHighShelf(44100, 4000.0f, 1.0f, 1.0f))
{
    if (modelIndexToPath.count(0)) {
        loadNamFromFile(modelIndexToPath[0]);
        currentModelIndex = 0;
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout NeuralAmpProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // path on device: /home/mind/NAM
    juce::File namDir("../NAM");
    auto modelNames = getSortedNamModelNames(namDir, modelIndexToPath);

    layout.add(std::make_unique<juce::AudioParameterFloat>("inputLevel", "Input Level", -20.0f, 20.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("toneBass", "Bass", 0.0f, 10.0f, 5.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("toneMid", "Middle", 0.0f, 10.0f, 5.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("toneTreble", "Treble", 0.0f, 10.0f, 5.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("outputLevel", "Output Level", -40.0f, 40.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>("noiseGateActive", "Noise Gate Active", true));
    layout.add(std::make_unique<juce::AudioParameterFloat>("noiseGateThreshold", "Noise Gate Threshold", -100.0f, 0.0f, -80.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>("eqActive", "EQ Active", true));
    layout.add(std::make_unique<juce::AudioParameterBool>("irToggle", "IR Toggle", true));
    layout.add(std::make_unique<juce::AudioParameterBool>("calibrateInput", "Calibrate Input", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>("inputCalibrationLevel", "Input Calibration Level", -60.0f, 60.0f, 12.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>("outputMode", "Output Mode", juce::StringArray{"Raw", "Normalized", "Calibrated"}, 1));

    layout.add(std::make_unique<juce::AudioParameterChoice>("selectedNamModel", "NAM Model", modelNames, 0));

    return layout;
}

NeuralAmpProcessor::~NeuralAmpProcessor()
{
}

void NeuralAmpProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    if (dsp)
    {
        dsp->Reset(sampleRate, samplesPerBlock);
    }

    // Prepare EQ filters
    juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2};
    bassFilter.prepare(spec);
    midFilter.prepare(spec);
    trebleFilter.prepare(spec);
}

void NeuralAmpProcessor::releaseResources()
{
    DBG("Releasing resources");
    bassFilter.reset();
    midFilter.reset();
    trebleFilter.reset();
}

void NeuralAmpProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    //DBG("Processing block, dsp=" << (void*)dsp.get() << ", samples=" << buffer.getNumSamples() << ", channels=" << buffer.getNumChannels());

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Get parameter values
    float inputGain = juce::Decibels::decibelsToGain(parameters.getRawParameterValue("inputLevel")->load());
    float outputGain = juce::Decibels::decibelsToGain(parameters.getRawParameterValue("outputLevel")->load());
    float bassGain = *parameters.getRawParameterValue("toneBass") / 5.0f;  // Scale 0-10 to 0-2
    float midGain = *parameters.getRawParameterValue("toneMid") / 5.0f;    // Scale 0-10 to 0-2
    float trebleGain = *parameters.getRawParameterValue("toneTreble") / 5.0f;  // Scale 0-10 to 0-2
    bool eqActive = *parameters.getRawParameterValue("eqActive") > 0.5f;
    bool noiseGateActive = *parameters.getRawParameterValue("noiseGateActive") > 0.5f;
    float noiseGateThreshold = *parameters.getRawParameterValue("noiseGateThreshold");

    // Apply input gain
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int i = 0; i < numSamples; ++i)
        {
            channelData[i] *= inputGain;
        }
    }

    // Noise gate (simple threshold)
    if (noiseGateActive)
    {
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            for (int i = 0; i < numSamples; ++i)
            {
                if (std::abs(channelData[i]) < juce::Decibels::decibelsToGain(noiseGateThreshold))
                    channelData[i] = 0.0f;
            }
        }
    }

    // DSP processing (if loaded)
    if (dsp)
    {
        std::vector<double> input(numSamples);
        std::vector<double> output(numSamples);

        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            for (int i = 0; i < numSamples; ++i)
            {
                input[i] = static_cast<double>(channelData[i]);
            }
            dsp->process(input.data(), output.data(), numSamples);
            for (int i = 0; i < numSamples; ++i)
            {
                channelData[i] = static_cast<float>(output[i]);
            }
        }
    }

    // Apply EQ if active
    if (eqActive)
    {
        // Create new coefficients and assign them to the filters
        auto bassCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), 100.0f, 1.0f, bassGain);
        auto midCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), 1000.0f, 1.0f, midGain);
        auto trebleCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(getSampleRate(), 4000.0f, 1.0f, trebleGain);

        *bassFilter.state = *bassCoeffs;
        *midFilter.state = *midCoeffs;
        *trebleFilter.state = *trebleCoeffs;

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        bassFilter.process(context);
        midFilter.process(context);
        trebleFilter.process(context);
    }

    // Apply output gain
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int i = 0; i < numSamples; ++i)
        {
            channelData[i] *= outputGain;
        }
    }

    //DBG("Block processed");
}

void NeuralAmpProcessor::processBlock(juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midi)
{
    buffer.clear();
}

juce::AudioProcessorEditor* NeuralAmpProcessor::createEditor()
{
    return new NeuralAmpEditor(*this);
}

void NeuralAmpProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::ignoreUnused(destData);
}

void NeuralAmpProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
}

void NeuralAmpProcessor::loadNamFromFile(const juce::String& filePath)
{
    juce::File file(filePath);
    if (!file.existsAsFile())
    {
        DBG("Error: File does not exist: " << filePath);
        return;
    }
    DBG("Loading NAM model from: " << filePath);
    try
    {
        dsp = nam::get_dsp(filePath.toStdString());
        if (dsp)
        {
            dsp->Reset(getSampleRate(), 512);
            DBG("Model loaded successfully: " << filePath);
        }
        else
        {
            DBG("Failed to load model: null DSP returned");
        }
    }
    catch (const std::exception& e)
    {
        DBG("Error loading model: " << e.what());
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NeuralAmpProcessor();
}