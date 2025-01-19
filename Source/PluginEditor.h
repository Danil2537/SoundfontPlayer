#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "FilePicker.h"
#include "MidiFilePicker.h"
#include"TablesComponent.h"

class SoundfontPlayerAudioProcessorEditor : public AudioProcessorEditor,
    public ComboBox::Listener,
    public MidiKeyboardStateListener,
    private Value::Listener,
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
    void populateBankComboBox();
    void updateUIAfterSoundfontLoad();
    void updateTrackComboBox();
    void updateChannelComboBox();
    ProgressBar progressBar;
    void updateProgressBar(double progress);

    void timerCallback() override;

    // Slider listener
    void sliderValueChanged(juce::Slider* slider) override;
    bool keyPressed(const KeyPress& key) override;
    bool keyStateChanged(bool isKeyDown) override;
    TablesComponent tablesComponent;
private:
    SoundfontPlayerAudioProcessor& audioProcessor;
    //MidiKeyboardState keyState;
    //ScopedPointer<MidiKeyboardComponent> keyboardComponent;
    ScopedPointer<ComboBox> soundfontSelector;

    // New UI elements
    TextButton loadSoundfontButton;
    TextButton loadMidiFileButton;
    TextButton playButton;
    TextButton stopButton;
    Label soundfontTitleLabel;
    Label midiFileTitleLabel;
    juce:: ComboBox comboTrack;
    juce::ComboBox comboChannel;
    juce::ComboBox bankComboBox;
    juce::ComboBox presetComboBox;
    FilePicker filePicker;
    MidiFilePicker midiFilePicker;
    
    void valueChanged(Value&) override;

    Value lastUIWidth, lastUIHeight;
    juce::Slider attackSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackSliderAttachment;
    juce::Slider decaySlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decaySliderAttachment;
    juce::Slider sustainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainSliderAttachment;
    juce::Slider releaseSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseSliderAttachment;


    juce::Slider trackbarSlider; // Trackbar slider for playback
    juce::Label currentTimeLabel; // Label to display current time
    juce::Label totalTimeLabel;   // Label to display total time
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundfontPlayerAudioProcessorEditor)
};