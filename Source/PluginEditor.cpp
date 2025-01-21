#include "PluginProcessor.h"
#include "PluginEditor.h"


SoundfontPlayerAudioProcessorEditor::SoundfontPlayerAudioProcessorEditor(SoundfontPlayerAudioProcessor& p)
    : AudioProcessorEditor(&p),
    audioProcessor(p),
    loadSoundfontButton("Load Soundfont"),
    loadMidiFileButton("Load MIDI File"),
    playButton("Play"),
    stopButton("Stop"),
    setSynthChannelButton("Set Synth Channel"),
    soundfontTitleLabel("Soundfont Title", ""),
    midiFileTitleLabel("MIDI File Title", ""),
    loadSoundfontLabel("Load Soundfont", "Load Soundfont:"),
    loadMidiFileLabel("Load MIDI File", "Load MIDI File:"),
    playButtonLabel("Play", "Play:"),
    stopButtonLabel("Stop", "Stop:"),
    synthChannelLabel("Current Synth Channel", "Current Synth Channel:"),
    comboTrackLabel("Track Selector", "Midi File Track:"),
    comboChannelLabel("Channel Selector", "Midi File Channel:"),
    checkboxLabel("Checkbox Label", "Play All Tracks and Channels?"),
    tablesComponent(p.apvts),
    filePicker(p.apvts),
    midiFilePicker(p.apvts)
{
    addAndMakeVisible(soundfontSelector = new ComboBox("SoundfontSelector"));
    soundfontSelector->addListener(this);
    soundfontSelector->addItemList(audioProcessor.soundfontNames, 1);
    soundfontSelector->setSelectedItemIndex(audioProcessor.getCurrentProgram());

    addAndMakeVisible(audioProcessor.keyboardComponent);
    audioProcessor.keyState.addListener(this);

    // Add new UI elements
    addAndMakeVisible(loadSoundfontButton);
    addAndMakeVisible(loadMidiFileButton);
    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    //addAndMakeVisible(setSynthChannelButton);
    addAndMakeVisible(midiFileTitleLabel);
    addAndMakeVisible(filePicker);
    addAndMakeVisible(midiFilePicker);
    addAndMakeVisible(loadSoundfontButton);
    addAndMakeVisible(loadSoundfontLabel);


    addAndMakeVisible(loadMidiFileLabel);


    addAndMakeVisible(playButtonLabel);

   
    addAndMakeVisible(stopButtonLabel);

    addAndMakeVisible(synthChannelLabel);

    
    addAndMakeVisible(comboTrackLabel);

    addAndMakeVisible(comboChannelLabel);

    addAndMakeVisible(checkboxLabel);

    addAndMakeVisible(soundfontTitleLabel);
    soundfontTitleLabel.setText("Current Soundfont File is: ", juce::NotificationType::sendNotification);
    addAndMakeVisible(midiFileTitleLabel);
    midiFileTitleLabel.setText("Current MIDI File is: ", juce::NotificationType::sendNotification);
    //updateChannelComboBox();
    loadSoundfontButton.onClick = [&]() 
    { /* Handle Load Soundfont */ 
            audioProcessor.loadSoundfontFile();
            updateUIAfterSoundfontLoad();
            soundfontTitleLabel.setText(audioProcessor.loadedSoundfontName,juce::NotificationType::sendNotification);
    };
    loadMidiFileButton.onClick = [&]()
    { /* Handle Load MIDI File */ 
            audioProcessor.uploadMidiFile();
            midiFileTitleLabel.setText(audioProcessor.loadedMidiName, juce::NotificationType::sendNotification);
            updateTrackComboBox();
            
    };
    playButton.onClick = [&]() 
    { /* Handle Play */ 
            DBG("PLAYING");
            audioProcessor.play();
    };
    stopButton.onClick = [&]()
    { /* Handle Stop */
            DBG("STOPPING");
            audioProcessor.stop();
    };


    tablesComponent.setWantsKeyboardFocus(false);
    addAndMakeVisible(tablesComponent);
    //audioProcessor.apvts.addParameterListener("bank", tablesComponent);



    addAndMakeVisible(presetComboBox);
    presetComboBox.addListener(this);

    // Bank ComboBox
    addAndMakeVisible(bankComboBox);
    bankComboBox.addListener(this);

    addAndMakeVisible(synthChannelComboBox);
    synthChannelComboBox.addListener(this);
    populateSynthChannelComboBox();
    setSynthChannelButton.onClick = [&]()
        {
            audioProcessor.soundfontPlayer.currentChannel.store(synthChannelComboBox.getSelectedId());
        };

    // Click on this combo box to select the track that needs to be played
    addAndMakeVisible(comboTrack);
    comboTrack.addListener(this);
    updateTrackComboBox();

    addAndMakeVisible(comboChannel);
    comboChannel.addListener(this);
    updateChannelComboBox();

    checkbox.setButtonText("Play All Tracks and Channels together?");
    checkbox.onClick = [this]() { handleCheckboxClick(); };
    addAndMakeVisible(checkbox);




    //attackSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    //attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 50);
    //addAndMakeVisible(attackSlider);
    //attackSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "ATTACK", attackSlider);

    //decaySlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    //decaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 50);
    //addAndMakeVisible(decaySlider);
    //decaySliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "DECAY", decaySlider);

    //sustainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    //sustainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 50);
    //addAndMakeVisible(sustainSlider);
    //sustainSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "SUSTAIN", sustainSlider);

    //releaseSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    //releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 50);
    //addAndMakeVisible(releaseSlider);
    //releaseSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "RELEASE", releaseSlider);


    //
    //addAndMakeVisible(progressBar);
    //progressBar.setColour(ProgressBar::backgroundColourId, Colours::grey);
    //progressBar.setColour(ProgressBar::foregroundColourId, Colours::green);

    setSize(800, 550);

    //trackbarSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    //trackbarSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    //trackbarSlider.addListener(this);
    //addAndMakeVisible(trackbarSlider);

    //currentTimeLabel.setText("0:00", juce::dontSendNotification);
    //currentTimeLabel.setJustificationType(juce::Justification::centred);
    //addAndMakeVisible(currentTimeLabel);

    //totalTimeLabel.setText("0:00", juce::dontSendNotification);
    //totalTimeLabel.setJustificationType(juce::Justification::centred);
    //addAndMakeVisible(totalTimeLabel);





    // Start timer to update playback position
    startTimerHz(30); // Update 30 times per second
}

SoundfontPlayerAudioProcessorEditor::~SoundfontPlayerAudioProcessorEditor()
{
    stopTimer();
}

void SoundfontPlayerAudioProcessorEditor::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void SoundfontPlayerAudioProcessorEditor::resized()
{
    const int margin = 10; // Margin around components
    const int labelHeight = 20;
    const int buttonHeight = 30;
    const int selectorHeight = 40;
    const int filePickerHeight = 25;
    const int spacing = 10; // Space between elements

    int y = margin; // Start from the top with some margin

    // File pickers
    filePicker.setBounds(margin, y, getWidth() - 2 * margin, filePickerHeight);
    y += filePickerHeight + spacing;

    midiFilePicker.setBounds(margin, y, getWidth() - 2 * margin, filePickerHeight);
    y += filePickerHeight + spacing;

    loadSoundfontButton.setBounds(margin, y, (getWidth() / 4)-20, buttonHeight);
    loadMidiFileButton.setBounds(margin + (getWidth() / 4), y, (getWidth() / 4)-20, buttonHeight);
    playButton.setBounds(margin + (getWidth() / 4) * 2, y, (getWidth() / 4)-20, buttonHeight);
    stopButton.setBounds(margin + (getWidth() / 4)*3, y, (getWidth() / 4)-20, buttonHeight);
    y += buttonHeight + spacing;

    // Soundfont and MIDI file titles
    soundfontTitleLabel.setBounds(margin, y, (getWidth() - 3 * margin) / 2, labelHeight);
    y += labelHeight + spacing;
    midiFileTitleLabel.setBounds(margin, y, (getWidth() - 3 * margin) / 2, labelHeight);
    y += labelHeight + spacing;

    // Track and channel selectors
    comboTrackLabel.setBounds(margin, y, getWidth() / 4, labelHeight);
    y += labelHeight + spacing;
    comboTrack.setBounds(margin, y, getWidth() / 2 - margin, selectorHeight);
    y += selectorHeight + spacing;
    comboChannelLabel.setBounds(margin, y, getWidth() / 4, labelHeight);
    y += labelHeight + spacing;
    comboChannel.setBounds(margin, y, getWidth() / 2 - margin, selectorHeight);
    y += selectorHeight + spacing;

    // Synth channel and checkbox
    synthChannelLabel.setBounds(margin, y, getWidth() / 4, labelHeight);
    y += labelHeight + spacing;
    synthChannelComboBox.setBounds(margin, y, getWidth() / 2 - margin, selectorHeight);
    y += selectorHeight + spacing;
    //checkboxLabel.setBounds(margin, y, (getWidth() - margin) / 4, labelHeight);
    //y += labelHeight + spacing;
    checkbox.setBounds(margin, y, getWidth() / 2, 25);
    y += selectorHeight + spacing;

    // Tables component
    const int tablesHeight = 150;
    const int keyboardHeight = 80;

    tablesComponent.setBounds(margin+(getWidth()/2), 120, (getWidth() /2)-margin, getHeight()-110- keyboardHeight-15);

    // MIDI keyboard component
    audioProcessor.keyboardComponent.setBounds(margin, getHeight() - keyboardHeight - margin, getWidth() - 2 * margin, keyboardHeight);

    // Save the dimensions
    lastUIWidth = getWidth();
    lastUIHeight = getHeight();
}


void SoundfontPlayerAudioProcessorEditor::handleCheckboxClick()
{
    if (checkbox.getToggleState())
    {
        juce::Logger::writeToLog("Checkbox enabled!");
        audioProcessor.playAllTracks = true;
    }
    else
    {
        juce::Logger::writeToLog("Checkbox disabled!");
        audioProcessor.playAllTracks = false;
    }
}

void SoundfontPlayerAudioProcessorEditor::valueChanged(Value&) {
    setSize(lastUIWidth.getValue(), lastUIHeight.getValue());
}

void SoundfontPlayerAudioProcessorEditor::handleNoteOn(MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity)
{
    //audioProcessor.soundfontPlayer.noteOn(midiNoteNumber, velocity);
    //audioProcessor.keyState.noteOn(midiChannel, midiNoteNumber, velocity);
}
void SoundfontPlayerAudioProcessorEditor::handleNoteOff(MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity)
{
    //audioProcessor.soundfontPlayer.noteOff(midiNoteNumber, velocity);
    //audioProcessor.keyState.noteOff(midiChannel, midiNoteNumber, velocity);
}

void SoundfontPlayerAudioProcessorEditor::comboBoxChanged(ComboBox* comboBoxThatWasChanged)
{
    if (comboBoxThatWasChanged == soundfontSelector) {
        audioProcessor.setCurrentProgram(soundfontSelector->getSelectedItemIndex());
    }
    else if (comboBoxThatWasChanged == &presetComboBox)
    {
        auto selectedPresetIndex = presetComboBox.getSelectedItemIndex();
        if (selectedPresetIndex >= 0 && selectedPresetIndex < audioProcessor.soundfontPlayer.getPresets().size())
        {
            const auto& selectedPreset = audioProcessor.soundfontPlayer.getPresets()[selectedPresetIndex];
            audioProcessor.soundfontPlayer.loadPreset(selectedPreset.bankNum, selectedPreset.presetNum);
        }
    }
    else if (comboBoxThatWasChanged == &bankComboBox)
    {
        auto selectedBank = bankComboBox.getSelectedId() - 1;
        updatePresetComboBox(selectedBank);
    }
    else if (comboBoxThatWasChanged == &comboTrack)
    {
        audioProcessor.setCurrentTrack(comboBoxThatWasChanged->getSelectedId() - 1);
    }
    else if (comboBoxThatWasChanged == &comboChannel)
    {
        audioProcessor.setCurrentChannel(comboBoxThatWasChanged->getSelectedId());
    }
    else if (comboBoxThatWasChanged == &synthChannelComboBox)
    {
        audioProcessor.soundfontPlayer.currentChannel.store(comboBoxThatWasChanged->getSelectedId());
    }
}

void SoundfontPlayerAudioProcessorEditor::updatePresetComboBox(int bankNum)
{
    presetComboBox.clear();

    const auto& presets = audioProcessor.soundfontPlayer.getPresets();
    for (size_t i = 0; i < presets.size(); ++i)
    {
        if (presets[i].bankNum == bankNum)
        {
            presetComboBox.addItem(presets[i].name + " (Preset: " + juce::String(presets[i].presetNum) + ")", i + 1);
        }
    }

    presetComboBox.setSelectedId(1);
}

void SoundfontPlayerAudioProcessorEditor::updateSynthChannelComboBox(int channelNum)
{
}

void SoundfontPlayerAudioProcessorEditor::populateBankComboBox()
{
    bankComboBox.clear();

    std::set<int> uniqueBanks;
    const auto& presets = audioProcessor.soundfontPlayer.getPresets();
    for (const auto& preset : presets)
    {
        uniqueBanks.insert(preset.bankNum);
    }

    int index = 1;
    for (const auto& bank : uniqueBanks)
    {
        bankComboBox.addItem("Bank " + juce::String(bank), index++);
    }

    bankComboBox.setSelectedId(1);
}

void SoundfontPlayerAudioProcessorEditor::updateUIAfterSoundfontLoad()
{
    audioProcessor.soundfontPlayer.iterate_presets();
    populateBankComboBox();
    updatePresetComboBox(bankComboBox.getSelectedId() - 1);
}


void SoundfontPlayerAudioProcessorEditor::updateTrackComboBox()
{
    comboTrack.clear();

    for (auto i = 0; i < audioProcessor.getNumTracks(); i++)
        comboTrack.addItem(audioProcessor.trackTitles[i],i+1);
        //comboTrack.addItem("Track number " + String(i + 1), i + 1);

    comboTrack.setSelectedId(audioProcessor.getCurrentTrack() + 1, dontSendNotification);
}
void SoundfontPlayerAudioProcessorEditor::updateChannelComboBox()
{
    for (auto i = 1; i <= 16; i++)
        comboChannel.addItem("Channel " + String(i), i);

    comboChannel.setSelectedId(audioProcessor.getCurrentChannel(), dontSendNotification);
}
void SoundfontPlayerAudioProcessorEditor::populateSynthChannelComboBox()
{
    for (auto i = 1; i <= 16; i++)
        synthChannelComboBox.addItem("Channel " + String(i), i);

    synthChannelComboBox.setSelectedId(1, dontSendNotification);
}
//void SoundfontPlayerAudioProcessorEditor::updateProgressBar(double progress)
//{
//    
//}

void SoundfontPlayerAudioProcessorEditor::timerCallback()
{
    double totalTime = audioProcessor.calculateTotalLengthInSeconds();
    double currentTime = audioProcessor.getCurrentProgress();

    //// Update slider range and position
    //trackbarSlider.setRange(0.0, totalTime, 0.01);
    //trackbarSlider.setValue(currentTime, juce::dontSendNotification);

    //// Update labels
    //currentTimeLabel.setText(juce::String(currentTime, 2) + "s", juce::dontSendNotification);
    //totalTimeLabel.setText(juce::String(totalTime, 2) + "s", juce::dontSendNotification);

}

void SoundfontPlayerAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    //if (slider == &trackbarSlider && slider->isMouseButtonDown())
    //{
    //    // Set playback position in the processor
    //    double newTime = slider->getValue();
    //    audioProcessor.setCurrentPlaybackPosition(newTime);
    //}
}

bool SoundfontPlayerAudioProcessorEditor::keyPressed(const KeyPress& key) {
    const int cursorKeys[] = {
            KeyPress::leftKey,
            KeyPress::rightKey,
            KeyPress::upKey,
            KeyPress::downKey
    };
    if (any_of(
        begin(cursorKeys),
        end(cursorKeys),
        [&](int i) { return i == key.getKeyCode(); }
    )) {
        return tablesComponent.keyPressed(key);
    }
    else {
        return audioProcessor.keyboardComponent.keyPressed(key);
    }
    return false;
}

bool SoundfontPlayerAudioProcessorEditor::keyStateChanged(bool isKeyDown)
{
    return audioProcessor.keyboardComponent.keyStateChanged(isKeyDown);
}
