#include "SoundfontAudioSource.h"
#include "../JuceLibraryCode/JuceHeader.h"
#include<fstream>

using namespace std;

SoundfontAudioSource::SoundfontAudioSource(AudioProcessorValueTreeState& apvts, int numberOfVoices) : valueTreeState(apvts)
{
    valueTreeState.addParameterListener("bank", this);
    valueTreeState.addParameterListener("preset", this);
    settings = new_fluid_settings();
    synth = new_fluid_synth(settings);
    fluid_synth_set_polyphony(synth, numberOfVoices);
    enableReverb(true);
    setReverbParameters(0.8f, 0.9f, 0.5f, 0.5f);
    //fluid_synth_set_chorus_on(synth, 1);
    //fluid_synth_set_chorus(synth, 45, 10.0, 0.5, 20.0, 1);
    //
    setGain(2.0f);
}



SoundfontAudioSource::~SoundfontAudioSource()
{
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}



void SoundfontAudioSource::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    adsr.setSampleRate(sampleRate);
    fluid_synth_set_sample_rate(synth, (float)sampleRate);
}

void SoundfontAudioSource::releaseResources()
{
    systemReset();
}

void SoundfontAudioSource::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
    // Remove any sounds coming in
    bufferToFill.clearActiveBufferRegion();

    //std::unique_ptr<fluid_mod_t, decltype(&delete_fluid_mod)> mod{ new_fluid_mod(), delete_fluid_mod };
    //adsr.applyEnvelopeToBuffer(*bufferToFill.buffer, startSample, bufferToFill.buffer->getNumSamples());
    //startSample += bufferToFill.buffer->getNumSamples(); // Update startSample for the next block
    // Prevent loading/unloading soundfonts & stuff while writing floats.
    // That is how you get race conditions and crashes.
    const ScopedLock l(lock);
    fluid_synth_write_float(synth,
        bufferToFill.buffer->getNumSamples(),
        bufferToFill.buffer->getWritePointer(0), 0, 1,
        bufferToFill.buffer->getWritePointer(1), 0, 1);
}

bool SoundfontAudioSource::loadSoundfont(const juce::String path)
{
    //if (file == loadedSoundfont) {
    //    // Don't reload an already loaded soundfont
    //    return false;
    //}
    loadedSoundfont = path;
    // Lock while switching soundfonts
    const ScopedLock l(lock);
    // All notes off
    fluid_synth_system_reset(synth);
    // If a soundfont is already loaded, unload the previous one (this demo doesn't go into banks, etc).
    if (fluid_synth_sfcount(synth) > 0) 
    {
        int err = fluid_synth_sfunload(synth, (unsigned int)sfontID, true);
        if (err == -1) 
        {
            return false;
        }
    }
    // Load the soundfont, store the handle
    sfontID = fluid_synth_sfload(synth, path.toStdString().c_str(), true);
    if (sfontID != -1)
    {
        iterate_presets();
    }
    refreshBanks();
    //fluid_synth_get_bank_offset(synth, sfontID
    return sfontID != -1;
}



//void SoundfontAudioSource::processMidi (const MidiMessage& message)

//{

//    if (message.isNoteOn()) {

//        noteOn(message.getNoteNumber(), message.getVelocity());

//    }

//    else if (message.isNoteOff()) {

//        noteOff(message.getNoteNumber());

//    }

//    else if (message.isController()) {

//        cc(message.getControllerNumber(), message.getControllerValue());

//    }

//    else if (message.isPitchWheel()) {

//        pitchBend(message.getPitchWheelValue());

//    }

//    else if (message.isChannelPressure()) {

//        channelPressure(message.getChannelPressureValue());

//    }

//    // Add support for other types of MIDI messages here

//}

void SoundfontAudioSource::noteOn(int note, float velocity, int channel)
{
    // Actually do note off if the velocity is 0
    //velocity == 0
    //    ? fluid_synth_noteoff(synth, channel, note)
    //    : fluid_synth_noteon(synth, channel, note, velocity);
    adsr.noteOn();
    if (velocity == 0)
    {
        fluid_synth_noteoff(synth, channel, note);
        //keyState.noteOff(channel, note, velocity);
    }
    else if (velocity == 100)
    {
        fluid_synth_noteon(synth, channel, note, velocity);
        //keyState.noteOn(channel, note, velocity);
    }
    else if (velocity < 1.0f && velocity>0.0f)
    {
        fluid_synth_noteon(synth, channel, note, velocity * 100.0f);
        //keyState->noteOn(channel, note, velocity);
    }
    else
    {
        fluid_synth_noteon(synth, channel, note, velocity/**127*/);
        //keyState.noteOn(channel, note, velocity);
    }
}

void SoundfontAudioSource::noteOff(int note, int channel)
{
    adsr.noteOff();
    fluid_synth_noteoff(synth, channel, note);
    //keyState.noteOff(channel, note, 0);
}



void SoundfontAudioSource::cc(int control, int value, int channel)
{
    fluid_synth_cc(synth, channel, control, value);
}

int SoundfontAudioSource::getCc(int control, int channel)
{
    int value = 0;
    fluid_synth_get_cc(synth, channel, control, &value);
    return value;
}

void SoundfontAudioSource::pitchBend(int value, int channel)
{
    fluid_synth_pitch_bend(synth, channel, value);
}

int SoundfontAudioSource::getPitchBend(int channel)
{
    int value = 0;
    fluid_synth_get_pitch_bend(synth, channel, &value);
    return value;
}

void SoundfontAudioSource::setPitchBendRange(int value, int channel)
{
    fluid_synth_pitch_wheel_sens(synth, channel, value);
}

int SoundfontAudioSource::getPitchBendRange(int channel)
{
    int value = 0;
    fluid_synth_get_pitch_wheel_sens(synth, channel, &value);
    return value;
}

void SoundfontAudioSource::channelPressure(int value, int channel)
{
    fluid_synth_channel_pressure(synth, channel, value);
}

void SoundfontAudioSource::setGain(float gain)
{
    fluid_synth_set_gain(synth, gain);
}

float SoundfontAudioSource::getGain()
{
    return fluid_synth_get_gain(synth);
}

void SoundfontAudioSource::systemReset()
{
    fluid_synth_system_reset(synth);
}

void SoundfontAudioSource::iterate_presets()
{
    fluid_sfont_t* sf = fluid_synth_get_sfont_by_id(synth, sfontID);
    fluid_preset_t* preset = new fluid_preset_t();
    PresetInfos.clear();
    // Reset the iteration
    sf->iteration_start(sf);
    // Go through all presets in the soundfont
    while (sf->iteration_next(sf, preset) != 0)
    {
        PresetInfo info;
        info.name = preset->get_name(preset);
        info.bankNum = preset->get_banknum(preset);
        info.presetNum = preset->get_num(preset);
        PresetInfos.push_back(info);
    }
    delete preset;
}

void SoundfontAudioSource::loadPreset(int bankNum, int presetNum)
{
    const ScopedLock l(lock);
    currentProgram = presetNum;
    fluid_synth_program_change(synth, currentChannel.load(), presetNum);
    fluid_synth_bank_select(synth, currentBank, bankNum);      // Select the bank
    DBG("Loading preset: Bank " << bankNum << ", Preset " << presetNum);
}

void SoundfontAudioSource::updateAdsrParams(float a, float d, float s, float r)
{
    adsrParams.attack = a;
    adsrParams.decay = d;
    adsrParams.sustain = s;
    adsrParams.release = r;
    adsr.setParameters(adsrParams);
}

void SoundfontAudioSource::parameterChanged(const String& parameterID, float newValue)
{
    if (programChangeParams.contains(parameterID)) 
    {
        int bank, preset;
        {
            RangedAudioParameter* param{ valueTreeState.getParameter("bank") };
            jassert(dynamic_cast<AudioParameterInt*>(param) != nullptr);
            AudioParameterInt* castParam{ dynamic_cast<AudioParameterInt*>(param) 
            };
            bank = castParam->get();
        }
        {
            RangedAudioParameter* param{ valueTreeState.getParameter("preset") };
            jassert(dynamic_cast<AudioParameterInt*>(param) != nullptr);
            AudioParameterInt* castParam{ dynamic_cast<AudioParameterInt*>(param) };
            preset = castParam->get();
        }
        int bankOffset{ fluid_synth_get_bank_offset(synth, sfontID) };
        currentProgram = preset;
        fluid_synth_program_select(synth, 1, sfontID, static_cast<unsigned int>(bankOffset + bank), static_cast<unsigned int>(preset));
    }
}

void SoundfontAudioSource::valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged, const Identifier& property)
{
    if (treeWhosePropertyHasChanged.getType() == StringRef("soundFont")) {
        if (property == StringRef("path")) 
        {
            String soundFontPath = treeWhosePropertyHasChanged.getProperty("path", "");
            if (soundFontPath.isNotEmpty()) 
            {
                loadSoundfont(soundFontPath);
            }
        }
    }
}

void SoundfontAudioSource::refreshBanks() 
{
    ValueTree banks{ "banks" };
    iterate_presets();
    if (!PresetInfos.empty()) 
    {
        int greatestEncounteredBank{ -1 };
        ValueTree bank;
        for (const auto& presetInfo : PresetInfos) 
        {
            int bankNum{ presetInfo.bankNum };
            if (bankNum > greatestEncounteredBank) {
                if (greatestEncounteredBank > -1) 
                {
                    banks.appendChild(bank, nullptr);
                }
                bank = { "bank", {
                    { "num", bankNum }
                } };
                greatestEncounteredBank = bankNum;
            }
            bank.appendChild({ "preset", {
                { "num", presetInfo.presetNum },
                { "name", presetInfo.name }
            }, {} }, nullptr);
        }
        if (greatestEncounteredBank > -1) {
            banks.appendChild(bank, nullptr);
        }
    }
    valueTreeState.state.getChildWithName("banks").copyPropertiesAndChildrenFrom(banks, nullptr);
    valueTreeState.state.getChildWithName("banks").sendPropertyChangeMessage("synthetic");
    //#if JUCE_DEBUG
    //    //    unique_ptr<XmlElement> xml{valueTreeState.state.createXml()};
    //    //    Logger::outputDebugString(xml->createDocument("",false,false));
    //#endif
}

void SoundfontAudioSource::enableReverb(bool isOn)
{
    fluid_synth_set_reverb_on(synth, isOn ? 1 : 0);
}

void SoundfontAudioSource::setReverbParameters(float size, float damp, float width, float level)
{
    fluid_synth_set_reverb(synth, size, damp, width, level);
}

void SoundfontAudioSource::renderMidiToWav(const juce::String& midiFilePath, const juce::String& wavFilePath, double sampleRate, int blockSize)
{
    juce::MidiFile midiFile;
    //if (!midiFile.readFrom(juce::File(midiFilePath)))
    //{
    //    jassertfalse; // Failed to read MIDI file
    //    return;
    //}

    midiFile.convertTimestampTicksToSeconds();

    juce::WavAudioFormat wavFormat;
    juce::File outputFile(wavFilePath);
    std::unique_ptr<juce::AudioFormatWriter> writer(wavFormat.createWriterFor(outputFile.createOutputStream().release(), sampleRate, 2, 24, {}, 0));

    if (writer == nullptr)
    {
        jassertfalse; // Failed to create writer
        return;
    }

    int numSamplesWritten = 0;
    double currentTime = 0.0;
    double lastTime = 0.0;

    while (currentTime < midiFile.getLastTimestamp())
    {
        juce::AudioBuffer<float> buffer(2, blockSize);
        buffer.clear();

        for (int track = 0; track < midiFile.getNumTracks(); ++track)
        {
            const juce::MidiMessageSequence& seq = *midiFile.getTrack(track);

            for (int i = 0; i < seq.getNumEvents(); ++i)
            {
                const juce::MidiMessageSequence::MidiEventHolder* eventHolder = seq.getEventPointer(i);
                const juce::MidiMessage& msg = eventHolder->message;

                if (msg.getTimeStamp() >= lastTime && msg.getTimeStamp() < currentTime)
                {
                    if (msg.isNoteOn())
                    {
                        noteOn(msg.getNoteNumber(), msg.getFloatVelocity(), msg.getChannel());
                    }
                    else if (msg.isNoteOff())
                    {
                        noteOff(msg.getNoteNumber(), msg.getChannel());
                    }
                    else if (msg.isController())
                    {
                        cc(msg.getControllerNumber(), msg.getControllerValue(), msg.getChannel());
                    }
                    else if (msg.isPitchWheel())
                    {
                        pitchBend(msg.getPitchWheelValue(), msg.getChannel());
                    }
                    else if (msg.isChannelPressure())
                    {
                        channelPressure(msg.getChannelPressureValue(), msg.getChannel());
                    }
                }
            }
        }

        getNextAudioBlock(juce::AudioSourceChannelInfo(buffer));

        writer->writeFromAudioSampleBuffer(buffer, 0, blockSize);

        numSamplesWritten += blockSize;
        lastTime = currentTime;
        currentTime += blockSize / sampleRate;
    }

    writer->flush();
}