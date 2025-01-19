//
// Created by Alex Birch on 03/10/2017.
//

#include "MidiFilePicker.h"

// #ifdef __APPLE__
//   #include <CoreFoundation/CFURL.h>
// #endif
#if JUCE_MAC || JUCE_IOS
#include <juce_core/native/juce_mac_CFHelpers.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFError.h>
using juce::CFUniquePtr;
#endif
#include "../../Source/MyColours.h"
#include "../../Source/Util.h"

MidiFilePicker::MidiFilePicker(
    AudioProcessorValueTreeState& valueTreeState
    // FluidSynthModel& fluidSynthModel
)
    : fileChooser{
        "File",
        File(),
        true,
        false,
        false,
        "*.sf2;*.sf3",
        String(),
        "Choose a midiFile file to load into the synthesizer" }
        , valueTreeState{ valueTreeState }
    // , fluidSynthModel{fluidSynthModel}
    // , currentPath{}
#if JUCE_MAC || JUCE_IOS
    , bookmarkCreationOptions{ kCFURLBookmarkCreationWithSecurityScope }
#endif
{
    // faster (rounded edges introduce transparency)
    setOpaque(true);

    // setDisplayedFilePath(fluidSynthModel.getCurrentmidiFileAbsPath());
    setDisplayedFilePath(valueTreeState.state.getChildWithName("midiFile").getProperty("path", ""));

    addAndMakeVisible(fileChooser);
    fileChooser.addListener(this);
    valueTreeState.state.addListener(this);
    //    valueTreeState.state.getChildWithName("midiFile").sendPropertyChangeMessage("path");

#if JUCE_MAC || JUCE_IOS
    bookmarkCreationOptions |= kCFURLBookmarkCreationSecurityScopeAllowOnlyReadAccess;
#endif
}
MidiFilePicker::~MidiFilePicker() {
    fileChooser.removeListener(this);
    valueTreeState.state.removeListener(this);
}

void MidiFilePicker::resized() {
    Rectangle<int> r(getLocalBounds());
    fileChooser.setBounds(r);
}

/**
 * This is required to support setOpaque(true)
 */
void MidiFilePicker::paint(Graphics& g)
{
    g.fillAll(MyColours::getUIColourIfAvailable(LookAndFeel_V4::ColourScheme::UIColour::windowBackground, juce::Colours::lightgrey));
}

void MidiFilePicker::filenameComponentChanged(FilenameComponent*) {
#if JUCE_MAC || JUCE_IOS
    CFUniquePtr<CFStringRef> fileExtensionCF{ fileChooser.getCurrentFile().getFullPathName().toCFString() };
    CFUniquePtr<CFURLRef> cfURL{ CFURLCreateWithFileSystemPath(NULL, fileExtensionCF.get(), CFURLPathStyle::kCFURLPOSIXPathStyle, false) };
    CFErrorRef* cfError;

    // CFURLCreateBookmarkData causes this error:
    // cannot open file at line 45340 of [d24547a13b]
    // os_unix.c:45340: (0) open(/var/db/DetachedSignatures) - Undefined error: 0
    CFUniquePtr<CFDataRef> cfData{ CFURLCreateBookmarkData(NULL, cfURL.get(), bookmarkCreationOptions, NULL, NULL, cfError) };

    const UInt8* cfDataBytePtr{ CFDataGetBytePtr(cfData.get()) };
    CFIndex cfDataByteLength{ CFDataGetLength(cfData.get()) };
    {
        Value value{ valueTreeState.state.getChildWithName("midiFile").getPropertyAsValue("bookmark", nullptr) };
        var var{ static_cast<const void*>(cfDataBytePtr), static_cast<size_t>(cfDataByteLength) };
        value.setValue(var);
    }
#endif
    // currentPath = fileChooser.getCurrentFile().getFullPathName();
    // fluidSynthModel.onFileNameChanged(fileChooser.getCurrentFile().getFullPathName(), -1, -1);
    Value value{ valueTreeState.state.getChildWithName("midiFile").getPropertyAsValue("path", nullptr) };
    value.setValue(fileChooser.getCurrentFile().getFullPathName());
    //    value = fileChooser.getCurrentFile().getFullPathName();
}

void MidiFilePicker::valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged,
    const Identifier& property) {
    if (treeWhosePropertyHasChanged.getType() == StringRef("midiFile")) {
        // if (&treeWhosePropertyHasChanged == &valueTree) {
        if (property == StringRef("path")) {
            String midiFilePath = treeWhosePropertyHasChanged.getProperty("path", "");
            DEBUG_PRINT(midiFilePath);
            setDisplayedFilePath(midiFilePath);
            // if (midiFilePath.isNotEmpty()) {
            //     loadFont(midiFilePath);
            // }
        }
    }
}

void MidiFilePicker::setDisplayedFilePath(const String& path) {
    if (!shouldChangeDisplayedFilePath(path)) {
        return;
    }
    // currentPath = path;
    fileChooser.setCurrentFile(File(path), true, dontSendNotification);
}

bool MidiFilePicker::shouldChangeDisplayedFilePath(const String& path) {
    if (path.isEmpty()) {
        return false;
    }
    if (path == currentPath) {
        return false;
    }
    return true;
}
