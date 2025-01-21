#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Fluidlite/include/fluidlite.h"

struct PresetInfo
{
    juce::String name;
    int bankNum;
    int presetNum;
};

class SoundfontAudioSource : public AudioSource, public AudioProcessorValueTreeState::Listener, public juce::ValueTree::Listener
{
public:
    
    /** Initializes fluidsynth. */
    SoundfontAudioSource(AudioProcessorValueTreeState& apvts, int numberOfVoices = 256);
    
    /** Destructor */
    ~SoundfontAudioSource();
    
    /** AudioSource Methods */
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    
    /** Load a .sf2 file. Will not reload a file if it is already loaded.
        If another file is loaded, it will unload that first. */
    bool loadSoundfont (const juce::String path);
    
    /** Sends an incoming midi message to fluidsynth */
    //void processMidi (const MidiMessage& message);
    
    /** Send a noteon message. */
    void noteOn (int note, float velocity, int channel = 1);
    
    /** Send a noteoff message. */
    void noteOff (int note, int channel = 1);
    
    /** Send a continuous controller message. */
    void cc (int control, int value, int channel = 1);
    
    /** Get a continuous controller value. */
    int getCc (int control, int channel = 1);
    
    /** Send a pitch bend message. */
    void pitchBend (int value, int channel = 1);
    
    /** Get the current pitch bend value. */
    int getPitchBend (int channel = 1);
    
    /** Set the pitch wheel sensitivity. */
    void setPitchBendRange (int value, int channel = 1);
    
    /** Get the pitch wheel sensitivity. */
    int getPitchBendRange (int channel = 1);
    
    /** Send a channel pressure message. */
    void channelPressure(int value, int channel = 1);
    
    /** Set the fluidsynth gain */
    void setGain (float gain);
    
    /** Get the fluidsynth gain */
    float getGain();
    
    /** Send a reset. A reset turns all the notes off and resets the
        controller values. */
    void systemReset();
    
    /** Returns the raw fluid_synth_t object for direct use with the Fluidsynth API. */
    fluid_synth_t* getSynth()       { return synth; }
    
    /** Returns the raw settings for use with the Fluidsynth API. */
    fluid_settings_t* getSettings() { return settings; }

    void iterate_presets();
    void loadPreset(int bankNum, int presetNum);
    const std::vector<PresetInfo>& SoundfontAudioSource::getPresets() const
    {
        return PresetInfos;
    }

    void updateAdsrParams(float a, float d, float s, float r);
    void parameterChanged(const String& parameterID, float newValue) override;
    void valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged, const Identifier& property) override;
    void refreshBanks();
    void enableReverb(bool isOn);
    void setReverbParameters(float size, float damp, float width, float level);
    int currentBank = 0;
    int currentProgram = 0;
    std::atomic<int> currentChannel;
private:
    const StringArray programChangeParams{ "bank", "preset" };
    AudioProcessorValueTreeState& valueTreeState;
    ADSR adsr;
    ADSR::Parameters adsrParams;
    int startSample = 0;
    CriticalSection lock;
    fluid_settings_t* settings;
    fluid_synth_t* synth;
    int sfontID;
    juce::String loadedSoundfont;
    std::vector<PresetInfo> PresetInfos;

};
