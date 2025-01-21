#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MyColours {
public:
    static Colour getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour uiColour, Colour fallback);

};