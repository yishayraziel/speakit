#pragma once
#include "PluginProcessor.h"
#include <juce_audio_utils/juce_audio_utils.h>
class SpeakitEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit SpeakitEditor(SpeakitProcessor&); ~SpeakitEditor() override;
    void resized() override; void setPage(int); void testMenu(int); // 0 MAIN 1 EDIT 2 FX 3 SAMPLES
    struct Impl; std::unique_ptr<Impl> d;
private:
    void timerCallback() override; SpeakitProcessor& p;
};
