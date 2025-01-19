#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Pills.h"
#include "TableComponent.h"
#include <memory>

using namespace std;

class TablesComponent : public Component
{
public:
    TablesComponent(
        AudioProcessorValueTreeState& valueTreeState
    );

    void resized() override;

    bool keyPressed(const KeyPress &key) override;

public:
    AudioProcessorValueTreeState& valueTreeState;

    Pills banks;
    TableComponent presetTable;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TablesComponent)
};
