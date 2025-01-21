#pragma once

#include <JuceHeader.h>
#include "SoundfontAudioSource.h"
//#include "MidiFilePlayer.h"

class SoundfontPlayerAudioProcessor : public AudioProcessor, public Timer, public Thread, public AudioProcessorValueTreeState::Listener
{
public:
    SoundfontPlayerAudioProcessor();
    ~SoundfontPlayerAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    bool hasEditor() const override;
    void loadSoundfontFile();
    void uploadMidiFile();
    juce::AudioProcessorEditor* createEditor() override;


    bool isMidiEffect() { return true; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const String getProgramName(int index) override;
    void changeProgramName(int index, const String& newName) override;

    void getStateInformation(MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void processBlock(AudioBuffer<float>&, MidiBuffer&) override;

public:
    AudioProcessorValueTreeState apvts;
    SoundfontAudioSource soundfontPlayer;
    MidiKeyboardState keyState;
    Array<File> soundfontFiles;
    StringArray soundfontPaths;
    StringArray soundfontNames;
    String currentSoundfontFile;
    String loadedSoundfontName = "Current Soundfont File is: ";
    String loadedMidiName = "Current MIDI File is";
    StringArray trackTitles;

    void timerCallback() override;
    void updateMidiDevices();
    //MidiKeyboardState keyboardState;
    MidiKeyboardComponent keyboardComponent /*= MidiKeyboardComponent(keyState, MidiKeyboardComponent::horizontalKeyboard)*/;


    //MidiFilePlayer midiFilePlayer;
    bool loadMidiFile(const String&);
    void run() override;

    void playFile();
    void playAll();
    bool playAllTracks = false;
    bool channelHasMessages(int trackIndex, int channel);
    void setTracks();
    //void playFile_IgnoreSustainPedal();

    void printAllTracks();
    // methods controlled by buttons
    void play();
    void stop();
    MidiFile theMidiFile;
    MidiMessageSequence* tracks;

    int numTracks;
    bool canPlay = false;

    // Add members for bank management
    //int currentBank = 0;
    //int currentProgram = 0;
    /** Returns the number of tracks in the MIDI file. */
    int getNumTracks();

    /** Sets the current track from the MIDI file that needs to be played. */
    void setCurrentTrack(int value);

    /** Returns the MIDI file track currently played. */
    int getCurrentTrack();
    /** Sets the current track from the MIDI file that needs to be played. */
    void setCurrentChannel(int value);

    /** Returns the MIDI file track currently played. */
    int getCurrentChannel();
    bool trackHasChanged = false;
    std::atomic<int> currentTrack;              // Current MIDI file track that is played
    //std::atomic<int> numTracks;                 // Current MIDI file number of tracks
    //std::atomic<int> currentChannel;
    void sendAllNotesOff(MidiBuffer& midiMessages);
    bool isPlayingSomething;

    //void updateProgressBar(double progress);

    juce::AudioProcessorValueTreeState::ParameterLayout createParams();
    double currentPositionInSeconds{ 0.0 };

    void parameterChanged(const String& parameterID, float newValue);
    //void loadPreset(int bank, int preset);
    double calculateTotalLengthInSeconds();
    double getCurrentProgress();
    double totalLengthInSeconds{ 0.0 };
    void setCurrentPlaybackPosition(double timeInSeconds);


    int bankParam = 0;
    int presetParam = 0;

private:
    std::atomic<double> hostTempo{ 140.0 };
    double originalMidiTempo{ 120.0 };
    double tempoScale{ 0.0 };


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundfontPlayerAudioProcessor)
};