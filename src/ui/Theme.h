#pragma once

#include <JuceHeader.h>

namespace fr3z0
{

inline const juce::Colour kMagenta { 0xffE038E9 };
inline const juce::Colour kPurple  { 0xff8A1AC6 };
inline const juce::Colour kViolet  { 0xff4B0082 };
inline const juce::Colour kCyan    { 0xff2FC1EE };
inline const juce::Colour kBlue    { 0xff3143D4 };
inline const juce::Colour kLight   { 0xffF0F8FF };
inline const juce::Colour kBlack   { 0xff000000 };
inline const juce::Colour kPanel   { 0xff140020 };
inline const juce::Colour kWell    { 0xff0a0014 };

inline juce::Font boldFont (float pt)
{
    return juce::Font (juce::FontOptions (pt).withStyle ("Bold"));
}

} // namespace fr3z0
