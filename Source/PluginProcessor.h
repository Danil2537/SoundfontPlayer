#pragma once

#include <JuceHeader.h>
#include "SoundfontAudioSource.h"

class SoundfontPlayerAudioProcessor : public AudioProcessor, public Timer, public Thread, public AudioProcessorValueTreeState::Listener
{
public:
    // Constructor and Destructor
    SoundfontPlayerAudioProcessor();
    ~SoundfontPlayerAudioProcessor() override;

    // AudioProcessor Methods
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(AudioBuffer<float>&, MidiBuffer&) override;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    bool hasEditor() const override;
    double getTailLengthSeconds() const override { return 0.0; }


    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const String getProgramName(int index) override;
    void changeProgramName(int index, const String& newName) override;
    void getStateInformation(MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //overrides
    juce::AudioProcessorEditor* createEditor() override;
    void timerCallback() override;
    void run() override;

    //midi playback
    void uploadMidiFile();
    void updateMidiDevices();
    bool loadMidiFile(const String&);
    void playFile();
    void playAll();
    bool channelHasMessages(int trackIndex, int channel);
    void setTracks();
    void printAllTracks();
    void play();
    void stop();
    void sendAllNotesOff(MidiBuffer& midiMessages);
    double calculateTotalLengthInSeconds();
    double getCurrentProgress();
    void setCurrentPlaybackPosition(double timeInSeconds);

    //other
    void loadSoundfontFile();
    juce::AudioProcessorValueTreeState::ParameterLayout createParams();
    void parameterChanged(const String& parameterID, float newValue);

    // Getters and Setters
    int getNumTracks();
    void setCurrentTrack(int value);
    int getCurrentTrack();
    void setCurrentChannel(int value);
    int getCurrentChannel();


public:
    AudioProcessorValueTreeState apvts;
    SoundfontAudioSource soundfontPlayer;
    MidiKeyboardState keyState;
    MidiKeyboardComponent keyboardComponent;
    Array<File> soundfontFiles;
    StringArray soundfontPaths;
    StringArray soundfontNames;
    String currentSoundfontFile;
    String loadedSoundfontName = "Current Soundfont File is: ";
    String loadedMidiName = "Current MIDI File is";
    StringArray trackTitles;
    MidiFile theMidiFile;
    MidiMessageSequence* tracks;
    int numTracks;
    bool canPlay = false;
    bool playAllTracks = false;
    bool trackHasChanged = false;
    bool isPlayingSomething;
    std::atomic<int> currentTrack;
    std::atomic<double> hostTempo{ 140.0 };
    double originalMidiTempo{ 120.0 };
    double tempoScale{ 0.0 };
    double currentPositionInSeconds{ 0.0 };
    double totalLengthInSeconds{ 0.0 };
    int bankParam = 0;
    int presetParam = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundfontPlayerAudioProcessor)
};