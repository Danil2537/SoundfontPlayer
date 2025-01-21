#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "MidiConstants.h"

String filePath = "C:\\Documents\\Cymatics - Odyssey MIDI 1 - C Min.mid";

using Parameter = AudioProcessorValueTreeState::Parameter;

AudioProcessor* JUCE_CALLTYPE createPluginFilter();

SoundfontPlayerAudioProcessor::SoundfontPlayerAudioProcessor()
    : keyboardComponent(keyState, MidiKeyboardComponent::horizontalKeyboard),
    Thread("MIDI Player Thread"),
    apvts(*this, nullptr, "Parameters", createParams()), soundfontPlayer(apvts)
#ifndef JucePlugin_PreferredChannelConfigurations
    , AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
{
    loadMidiFile(filePath);
    soundfontFiles = File(String(__FILE__))
        .getParentDirectory()
        .getParentDirectory()
        .getChildFile("Soundfonts")
        .findChildFiles(File::findFiles, false, "*.sf2");



    MemoryBlock bookmarkBuffer;
    apvts.state.appendChild({ "soundFont", {
        { "path", "" },
        { "bookmark", std::move(bookmarkBuffer) },
    }, {} }, nullptr);

    MemoryBlock midiBookmarkBuffer;
    apvts.state.appendChild({ "midiFile", {
        { "path", "" },
        { "bookmark", std::move(midiBookmarkBuffer) },
    }, {} }, nullptr);

    apvts.state.appendChild({ "banks", {}, {} }, nullptr);

    updateMidiDevices();
    numTracks = 0;
    startTimer(400);

    // Initialize bank and preset parameters
    bankParam = apvts.getRawParameterValue("bank")->load();
    presetParam = apvts.getRawParameterValue("preset")->load();

    // Attach listeners to parameters
    apvts.addParameterListener("bank", this);
    apvts.addParameterListener("preset", this);

    for (File f : soundfontFiles) {
        soundfontPaths.add(f.getFullPathName());
        soundfontNames.add(f.getFileNameWithoutExtension());
    }

    if (soundfontFiles.size() > 0) {
        currentSoundfontFile = soundfontPaths[0];
        soundfontPlayer.loadSoundfont(currentSoundfontFile);
    }

}

SoundfontPlayerAudioProcessor::~SoundfontPlayerAudioProcessor()
{
}

void SoundfontPlayerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    soundfontPlayer.prepareToPlay(samplesPerBlock, sampleRate);
}

void SoundfontPlayerAudioProcessor::releaseResources()
{
    soundfontPlayer.releaseResources();
}

int SoundfontPlayerAudioProcessor::getNumPrograms()
{
    return soundfontFiles.size();
}

int SoundfontPlayerAudioProcessor::getCurrentProgram()
{
    return soundfontFiles.indexOf(currentSoundfontFile);
}

void SoundfontPlayerAudioProcessor::setCurrentProgram(int index)
{
    if (index >= 0 && index < soundfontFiles.size()) {
        currentSoundfontFile = soundfontFiles[index].getFullPathName();
        soundfontPlayer.loadSoundfont(currentSoundfontFile);
    }
}

const String SoundfontPlayerAudioProcessor::getProgramName(int index)
{
    return soundfontNames[index];
}

void SoundfontPlayerAudioProcessor::changeProgramName(int index, const String& newName)
{
    // Not implemented for this example
}

void SoundfontPlayerAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    // Create an outer XML element..
}

void SoundfontPlayerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Not implemented for this example
}

void SoundfontPlayerAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    for (const auto metadata : midiMessages)
    {
        const auto message = metadata.getMessage();

        soundfontPlayer.updateAdsrParams(
            apvts.getRawParameterValue("ATTACK")->load(),
            apvts.getRawParameterValue("DECAY")->load(),
            apvts.getRawParameterValue("SUSTAIN")->load(),
            apvts.getRawParameterValue("RELEASE")->load()
        );

        if (message.isNoteOn())
        {
            //soundfontPlayer.currentChannel = message.getChannel();
            soundfontPlayer.noteOn(message.getNoteNumber(), message.getFloatVelocity(), message.getChannel());
            keyState.noteOn(message.getChannel(), message.getNoteNumber(), message.getFloatVelocity());
        }
        else if (message.isNoteOff())
        {
            soundfontPlayer.noteOff(message.getNoteNumber(), message.getChannel());
            keyState.noteOff(message.getChannel(), message.getNoteNumber(), message.getFloatVelocity());
        }
    }

    if (auto* playHead = getPlayHead())
    {
        AudioPlayHead::CurrentPositionInfo info;
        if (playHead->getCurrentPosition(info) && info.bpm > 0)
        {
            hostTempo.store(info.bpm); // Update host tempo
        }
        tempoScale = hostTempo.load() / originalMidiTempo;
    }

    soundfontPlayer.getNextAudioBlock(AudioSourceChannelInfo(buffer));
}

void SoundfontPlayerAudioProcessor::timerCallback()
{
    keyboardComponent.grabKeyboardFocus();
    stopTimer();
}

void SoundfontPlayerAudioProcessor::updateMidiDevices()
{
    DBG("-Available Midi Devices-");
    for (MidiDeviceInfo m : MidiOutput::getAvailableDevices()) {
        DBG(m.name);
    }
    DBG("-End Midi Device List-");
}

const juce::String SoundfontPlayerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SoundfontPlayerAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool SoundfontPlayerAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool SoundfontPlayerAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

bool SoundfontPlayerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

void SoundfontPlayerAudioProcessor::loadSoundfontFile()
{
   

    ValueTree tree{ apvts.state.getChildWithName("soundFont") };
    currentSoundfontFile = tree.getProperty("path", "");
    File sf(currentSoundfontFile);
    this->loadedSoundfontName = "The current soundfont file is:" + sf.getFileNameWithoutExtension();

    soundfontPlayer.loadSoundfont(currentSoundfontFile);
}

void SoundfontPlayerAudioProcessor::uploadMidiFile()
{
    ValueTree tree{ apvts.state.getChildWithName("midiFile") };
    File midiFile(tree.getProperty("path", ""));
    loadMidiFile(midiFile.getFullPathName());
    loadedMidiName = "Current Midi File is: "+ midiFile.getFileNameWithoutExtension();
}

juce::AudioProcessorEditor* SoundfontPlayerAudioProcessor::createEditor()
{
    return new SoundfontPlayerAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SoundfontPlayerAudioProcessor();
}

bool SoundfontPlayerAudioProcessor::loadMidiFile(const String& path)
{
    File file(path);
    if (file.exists()) {
        theMidiFile.clear();
        FileInputStream inputStream(file);
        theMidiFile.readFrom(inputStream);
        DBG("Loaded Midi File, ready to play");
        theMidiFile.convertTimestampTicksToSeconds();
        setTracks();
        DBG("Number of tracks: " + std::to_string(theMidiFile.getNumTracks()));
        trackTitles.clear();
        for (int i = 0; i < theMidiFile.getNumTracks(); ++i)
        {
            DBG("Track " + std::to_string(i) + " has " + std::to_string(theMidiFile.getTrack(i)->getNumEvents()) + " events.");
        }
        numTracks = theMidiFile.getNumTracks();
        soundfontPlayer.currentChannel.store(1);
        currentTrack.store(0);
        trackHasChanged = false;

        for (int i = 0; i < numTracks; ++i)
        {
            for (int j = 0; j < theMidiFile.getTrack(i)->getNumEvents(); ++j)
            {
                auto message = theMidiFile.getTrack(i)->getEventPointer(j)->message;
                if (message.isMetaEvent() && message.getMetaEventType() == 0x03) // Track Name meta-event
                {
                    trackTitles.add(message.getTextFromTextMetaEvent()); // Extract track name
                    break;
                }
                if (message.isTempoMetaEvent())
                {
                    originalMidiTempo = message.getTempoSecondsPerQuarterNote() > 0
                        ? 60.0 / message.getTempoSecondsPerQuarterNote()
                        : 120.0;
                    break;
                }
            }
        }

        return (canPlay = true);
    }
    DBG("Cannot Play File or Find Path");
    return (canPlay = false);
}

void SoundfontPlayerAudioProcessor::setTracks()
{
    numTracks = theMidiFile.getNumTracks();
    tracks = new MidiMessageSequence[numTracks];

    for (int i = 0; i < numTracks; i++)
        tracks[i] = *theMidiFile.getTrack(i);

    printAllTracks();
}

void SoundfontPlayerAudioProcessor::play()
{
    if (canPlay) {
        startThread();
    }
}

void SoundfontPlayerAudioProcessor::run()
{
    if (playAllTracks)
    {
        playAll();
    }
    else { playFile(); }
}

void SoundfontPlayerAudioProcessor::playFile()
{
    if (numTracks > 0)
    {
        int localCurrentTrack = currentTrack.load();

        int numTrackEvents = tracks[localCurrentTrack].getNumEvents();
        int i = 0;
        bool sustainOn = false;
        double lastTempo = originalMidiTempo;
        double currentTimeInSeconds = 0.0;
        if (channelHasMessages(localCurrentTrack, soundfontPlayer.currentChannel.load()))
        {
            while (i < numTrackEvents && !threadShouldExit())
            {
                MidiMessage* currMsg = &tracks[localCurrentTrack].getEventPointer(i)->message;
                if (!currMsg->isEndOfTrackMetaEvent())
                {
                    if (currMsg->getChannel() == soundfontPlayer.currentChannel.load())
                    {
                        MidiMessage* nextMsg = &tracks[localCurrentTrack].getEventPointer(i + 1)->message;

                        double thisMsgTimestamp = currMsg->getTimeStamp(), nextMsgTimestamp = nextMsg->getTimeStamp();

                        double waitTimeSecs = 1000 * (nextMsgTimestamp - thisMsgTimestamp) / tempoScale;
                        currentPositionInSeconds = currentTimeInSeconds + (thisMsgTimestamp - currentTimeInSeconds) * (lastTempo / originalMidiTempo);
                        currentTimeInSeconds = thisMsgTimestamp;

                        if (currMsg->isNoteOn())
                        {
                            soundfontPlayer.noteOn(currMsg->getNoteNumber(), currMsg->getFloatVelocity(), soundfontPlayer.currentChannel.load());
                            keyState.noteOn(soundfontPlayer.currentChannel.load(), currMsg->getNoteNumber(), currMsg->getFloatVelocity());
                        }
                        else if (currMsg->isNoteOff())
                        {
                            soundfontPlayer.noteOff(currMsg->getNoteNumber(), soundfontPlayer.currentChannel.load());
                            keyState.noteOff(soundfontPlayer.currentChannel.load(), currMsg->getNoteNumber(), currMsg->getFloatVelocity());
                        }
                        else if (currMsg->isController())
                        {
                            if (currMsg->getControllerNumber() == 64) // Sustain pedal
                            {
                                sustainOn = currMsg->getControllerValue() >= 64;
                            }
                        }
                        else if (currMsg->isTempoMetaEvent())
                        {
                            lastTempo = 60.0 / currMsg->getTempoSecondsPerQuarterNote();
                        }

                        if (waitTimeSecs != 0) wait((int)waitTimeSecs);
                    }
                }
                else {
                    threadShouldExit();
                }
                i++;

                double progress = currentPositionInSeconds / totalLengthInSeconds;
                //updateProgressBar(progress);
            }
        }
        else
        {
            DBG("No messages in selected channel " + std::to_string(soundfontPlayer.currentChannel.load()) + " of selected track" + std::to_string(currentTrack.load()));
        }
        currentPositionInSeconds = 0.0;
        DBG("Closing Play() thread");
    }
}

void SoundfontPlayerAudioProcessor::playAll()
{
    if (numTracks > 0)
    {
        std::vector<int> eventIndices(numTracks, 0); // Track progress through each track
        std::vector<double> trackTimeStamps(numTracks, 0.0);
        double lastTempo = originalMidiTempo;
        double globalCurrentTime = 0.0;

        while (!threadShouldExit())
        {
            double nextEventTime = std::numeric_limits<double>::max();
            int trackToProcess = -1;

            // Find the next event to process across all tracks
            for (int trackIndex = 0; trackIndex < numTracks; ++trackIndex)
            {
                if (eventIndices[trackIndex] < tracks[trackIndex].getNumEvents())
                {
                    double eventTime = tracks[trackIndex].getEventPointer(eventIndices[trackIndex])->message.getTimeStamp();
                    if (eventTime < nextEventTime)
                    {
                        nextEventTime = eventTime;
                        trackToProcess = trackIndex;
                    }
                }
            }

            if (trackToProcess == -1)
                break; // All events processed

            double waitTimeSecs = (nextEventTime - globalCurrentTime) * 1000.0 / tempoScale;
            if (waitTimeSecs > 0)
                wait((int)waitTimeSecs);

            globalCurrentTime = nextEventTime;

            // Process the next event on the selected track
            MidiMessage* currMsg = &tracks[trackToProcess].getEventPointer(eventIndices[trackToProcess])->message;
            if (currMsg->isNoteOn())
            {
                soundfontPlayer.noteOn(currMsg->getNoteNumber(), currMsg->getFloatVelocity(), currMsg->getChannel());
                keyState.noteOn(currMsg->getChannel(), currMsg->getNoteNumber(), currMsg->getFloatVelocity());
            }
            else if (currMsg->isNoteOff())
            {
                soundfontPlayer.noteOff(currMsg->getNoteNumber(), currMsg->getChannel());
                keyState.noteOff(currMsg->getChannel(), currMsg->getNoteNumber(), currMsg->getFloatVelocity());
            }
            else if (currMsg->isController() && currMsg->getControllerNumber() == 64) // Sustain pedal
            {
                // Handle sustain pedal if needed
            }
            else if (currMsg->isTempoMetaEvent())
            {
                lastTempo = 60.0 / currMsg->getTempoSecondsPerQuarterNote();
            }

            ++eventIndices[trackToProcess];

            // Update progress bar
            double progress = globalCurrentTime / totalLengthInSeconds;
            //updateProgressBar(progress);
        }
    }
    else
    {
        DBG("No tracks to play.");
    }

    DBG("Finished playing all tracks.");
}

bool SoundfontPlayerAudioProcessor::channelHasMessages(int trackIndex, int channel)
{
    if (trackIndex < 0 || trackIndex >= numTracks)
    {
        DBG("Invalid track index");
        return false;
    }

    int numTrackEvents = tracks[trackIndex].getNumEvents();

    for (int i = 0; i < numTrackEvents; ++i)
    {
        MidiMessage* currMsg = &tracks[trackIndex].getEventPointer(i)->message;

        if (currMsg->getChannel() == channel)
        {
            return true; // Found a message belonging to the channel
        }
    }

    return false; // No messages for the specified channel
}

void SoundfontPlayerAudioProcessor::stop()
{
    stopThread(20);
    keyState.allNotesOff(1);
}

void SoundfontPlayerAudioProcessor::printAllTracks()
{
    for (int i = 0; i < numTracks; i++)
    {
        int numTrackEvents = tracks[i].getNumEvents();
        DBG("Track #" + std::to_string(i) + " NumTrack Events #" + std::to_string(numTrackEvents));

        for (int j = 0; j < tracks[i].getNumEvents(); j++)
            DBG(String(std::to_string(j)) + " @" + std::to_string(tracks[i].getEventPointer(j)->message.getTimeStamp()) + " " + tracks[i].getEventPointer(j)->message.getDescription());
    }
}

int SoundfontPlayerAudioProcessor::getNumTracks()
{
    return numTracks;
}

void SoundfontPlayerAudioProcessor::setCurrentTrack(int value)
{
    jassert(value >= 0 && value < numTracks);

    if (numTracks == 0)
        return;

    currentTrack.store(value);
    trackHasChanged = true;
}

int SoundfontPlayerAudioProcessor::getCurrentTrack()
{
    if (numTracks == 0)
        return -1;

    else
        return currentTrack.load();
}

void SoundfontPlayerAudioProcessor::setCurrentChannel(int value)
{
    soundfontPlayer.currentChannel.store(value);
}

int SoundfontPlayerAudioProcessor::getCurrentChannel()
{
    return soundfontPlayer.currentChannel.load();
}

void SoundfontPlayerAudioProcessor::sendAllNotesOff(MidiBuffer& midiMessages)
{
    for (auto i = 1; i <= 16; i++)
    {
        midiMessages.addEvent(MidiMessage::allNotesOff(i), 0);
        midiMessages.addEvent(MidiMessage::allSoundOff(i), 0);
        midiMessages.addEvent(MidiMessage::allControllersOff(i), 0);
    }

    isPlayingSomething = false;
}

juce::AudioProcessorValueTreeState::ParameterLayout SoundfontPlayerAudioProcessor::createParams()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // ADSR
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ATTACK", "Attack", juce::NormalisableRange<float>{0.1f, 1.0f}, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("DECAY", "Decay", juce::NormalisableRange<float>{0.1f, 1.0f}, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("SUSTAIN", "Sustain", juce::NormalisableRange<float>{0.1f, 1.0f}, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("RELEASE", "Release", juce::NormalisableRange<float>{0.1f, 3.0f}, 0.3f));

    // SF
    params.push_back(std::make_unique<juce::AudioParameterInt>("bank", "which bank is selected in the soundfont", MidiConstants::midiMinValue, 128, MidiConstants::midiMinValue, "Bank"));
    params.push_back(std::make_unique<juce::AudioParameterInt>("preset", "which patch (aka patch, program, instrument) is selected in the soundfont", MidiConstants::midiMinValue, MidiConstants::midiMaxValue, MidiConstants::midiMinValue, "Preset"));

    return { params.begin(), params.end() };
}

//void SoundfontPlayerAudioProcessor::updateProgressBar(double progress)
//{
//    MessageManager::callAsync([this, progress]() {
//        if (auto* editor = dynamic_cast<SoundfontPlayerAudioProcessorEditor*>(getActiveEditor()))
//        {
//            editor->updateProgressBar(progress);
//        }
//        });
//}

double SoundfontPlayerAudioProcessor::calculateTotalLengthInSeconds()
{
    return theMidiFile.getLastTimestamp() * tempoScale;
}

double SoundfontPlayerAudioProcessor::getCurrentProgress()
{
    return currentPositionInSeconds;
}

void SoundfontPlayerAudioProcessor::setCurrentPlaybackPosition(double timeInSeconds)
{
    if (canPlay && timeInSeconds >= 0 && timeInSeconds <= totalLengthInSeconds)
    {
        currentPositionInSeconds = timeInSeconds;
        // Adjust track position based on new time
        theMidiFile.setTicksPerQuarterNote(timeInSeconds / tempoScale);
    }
}

void SoundfontPlayerAudioProcessor::parameterChanged(const String& parameterID, float newValue)
{
    if (parameterID == "bank") {
        int bank = static_cast<int>(newValue);
        soundfontPlayer.currentBank = bank;
        // Update UI components
        if (auto* editor = dynamic_cast<SoundfontPlayerAudioProcessorEditor*>(getActiveEditor())) {
            editor->tablesComponent.banks.updatePillToggleStates();
        }
    }
    else if (parameterID == "preset") {
        int preset = static_cast<int>(newValue);
        soundfontPlayer.currentProgram = preset;
        soundfontPlayer.loadPreset(bankParam, soundfontPlayer.currentProgram);
        // Update UI components
        if (auto* editor = dynamic_cast<SoundfontPlayerAudioProcessorEditor*>(getActiveEditor())) {
            editor->tablesComponent.presetTable.selectCurrentPreset();
        }
    }
}