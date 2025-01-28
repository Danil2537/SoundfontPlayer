#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "FilePicker.h"
#include "MidiFilePicker.h"
#include "TablesComponent.h"

class SoundfontPlayerAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::ComboBox::Listener,
    public juce::MidiKeyboardStateListener,
    private juce::Value::Listener,
    public juce::Timer,
    public juce::Slider::Listener
{
public:
    SoundfontPlayerAudioProcessorEditor(SoundfontPlayerAudioProcessor&);
    ~SoundfontPlayerAudioProcessorEditor() override;

    void paint(Graphics&) override;
    void resized() override;

    void handleNoteOn(MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    void comboBoxChanged(ComboBox* comboBoxThatWasChanged) override;
    void updatePresetComboBox(int bankNum);
    void updateSynthChannelComboBox(int channelNum);
    void populateBankComboBox();
    void updateUIAfterSoundfontLoad();
    void updateTrackComboBox();
    void updateChannelComboBox();
    void populateSynthChannelComboBox();
    //ProgressBar progressBar;
    //void updateProgressBar(double progress);

    void timerCallback() override;

    // Slider listener
    void sliderValueChanged(juce::Slider* slider) override;
    bool keyPressed(const KeyPress& key) override;
    bool keyStateChanged(bool isKeyDown) override;
    TablesComponent tablesComponent;

private:
    SoundfontPlayerAudioProcessor& audioProcessor;

    ScopedPointer<ComboBox> soundfontSelector;
    TextButton loadSoundfontButton;
    TextButton loadMidiFileButton;
    TextButton playButton;
    TextButton stopButton;
    TextButton setSynthChannelButton;
    TextButton renderButton;

    Label soundfontTitleLabel;
    Label midiFileTitleLabel;

    juce::Label loadSoundfontLabel;
    juce::Label loadMidiFileLabel;
    juce::Label playButtonLabel;
    juce::Label stopButtonLabel;
    juce::Label synthChannelLabel;
    juce::Label comboTrackLabel;
    juce::Label comboChannelLabel;
    juce::Label checkboxLabel;

    juce::ComboBox comboTrack;
    juce::ComboBox comboChannel;
    juce::ComboBox bankComboBox;
    juce::ComboBox presetComboBox;
    juce::ComboBox synthChannelComboBox;

    FilePicker filePicker;
    MidiFilePicker midiFilePicker;
    juce::ToggleButton checkbox;

    void handleCheckboxClick();
    void valueChanged(Value&) override;

    Value lastUIWidth, lastUIHeight;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundfontPlayerAudioProcessorEditor)
};