#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_dsp/juce_dsp.h>
#include "Dsp.h"
#include "Telecom.h"
#include "IrManifest.h"
#include "BayManifest.h"
class SpeakitProcessor : public juce::AudioProcessor, private juce::Timer {
public:
    SpeakitProcessor(); ~SpeakitProcessor() override;
    void prepareToPlay(double, int) override; void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override; void processBlockBypassed(juce::AudioBuffer<float>&, juce::MidiBuffer&) override; void loadPreset(int i) { applyPreset(i); }
    juce::AudioProcessorEditor* createEditor() override; bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Speakit"; } bool acceptsMidi() const override { return true; } bool producesMidi() const override { return false; } double getTailLengthSeconds() const override { return 4.0; }
    int getNumPrograms() override; int getCurrentProgram() override { return presetIdx; } void setCurrentProgram(int) override; const juce::String getProgramName(int) override; void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override; void setStateInformation(const void*, int) override;
    juce::AudioProcessorValueTreeState apvts; static constexpr int maxB = 2048;
    std::atomic<float> inDb { -90 }, outDb { -90 }, grDb { 0 };
    void service(); // applies pending IR changes; called by timer, and by tests
    // sample bay
    struct Slot { std::atomic<bool> on { false }; std::atomic<float> gain { 1 }; std::atomic<juce::AudioBuffer<float>*> data { nullptr }; double pos = 0; float env = 0; };
    Slot bay[5][12]; std::atomic<bool> bankPre[5]; std::atomic<int> bankShown { 0 };
    void setSlot(int b, int s, bool on); void ensureLoaded(int b, int s);
    // snapshots
    void storeSnap(int i); void recallSnap(int i); bool hasSnap(int i) const { return snaps[i].isValid(); } int snapNow = -1;
    int presetIdx = 0; juce::String presetName(); void waitForBay();
private:
    void timerCallback() override { service(); } void loadIr(int kind, int idx); void chain(float* L, float* R, int n, juce::MidiBuffer* midi, int offset);
    struct Conv { juce::dsp::Convolution c { juce::dsp::Convolution::NonUniform { 512 } }; juce::AudioBuffer<float> w; void prepare(double sr, int b) { w.setSize(2, b); c.prepare({ sr, (juce::uint32)b, 2 }); c.reset(); }
        void process(float* L, float* R, int n, const float* mix) { bool any = false; for (int i = 0; i <= (n - 1) >> 5; ++i) any |= mix[i] > 1e-4f; if (!any) return; w.copyFrom(0, 0, L, n); w.copyFrom(1, 0, R, n); float* p[2] = { w.getWritePointer(0), w.getWritePointer(1) }; juce::dsp::AudioBlock<float> b(p, 2, (size_t)n); c.process(juce::dsp::ProcessContextReplacing<float>(b));
            for (int i = 0; i < n; ++i) { float m = mix[i >> 5]; L[i] += (p[0][i] - L[i]) * m; R[i] += (p[1][i] - R[i]) * m; } } };
    Conv cSpk, cCov, cMic, cRoom; int loaded[4] { -1, -1, -1, -1 }; float lPitch = -99, lDecay = -1;
    sk::Gate gate; sk::Comp comp; sk::Eq eq; sk::Drive drive; sk::Telecom tel; sk::Crush crush; sk::Mod mod; sk::Gramophone gram; sk::Tuning tune; sk::Leslie les; sk::StereoDelay dly; sk::Limiter lim;
    double sr = 48000; juce::Random rnd; float lfoPh[4] {}, sh[4] {}, last[4] {}, env[2] {}, duckEnv = 0, duckG = 1; float slotMix[16] {}; bool first = true;
    juce::AudioBuffer<float> preBus, postBus; juce::AudioFormatManager fm; juce::ThreadPool pool { 1 }; juce::ValueTree snaps[10];
    std::unordered_map<std::string, std::atomic<float>*> pv; float PS(const char* id) const { return pv.at(id)->load(); }
    mutable std::unordered_map<const void*, std::atomic<float>*> fast; bool warming = false;
    float P(const char* id) { auto i = fast.find(id); if (i != fast.end()) return i->second->load(); auto* a = pv.at(id); if (warming) fast[id] = a; return a->load(); }
    std::atomic<float>* lp[4][4] {}; sk::Smooth inS, outS; sk::Delay byp[2]; std::atomic<bool> want[5][12];
    void applyPreset(int i);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpeakitProcessor)
};
