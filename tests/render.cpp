// Offline renderer for demos and reels: runs audio through factory presets with the real processor
// and saves a snapshot of the editor for each preset.
// Usage: sk_render <in.wav> <out dir> <preset name> [<preset name> ...]
//   preset "CLEAN" = everything bypassed. Writes <out>/<NN>.wav (latency removed) and <out>/<NN>.png.
#include "PluginProcessor.h"
#include "Presets.h"
using namespace juce;

int main(int argc, char** argv) {
    ScopedJuceInitialiser_GUI gui;
    if (argc < 4) { printf("usage: sk_render in.wav outdir preset...\n"); return 1; }
    AudioFormatManager fm; fm.registerBasicFormats();
    std::unique_ptr<AudioFormatReader> r(fm.createReaderFor(File(argv[1])));
    if (!r) { printf("cannot read %s\n", argv[1]); return 1; }
    const double sr = r->sampleRate; const int len = (int)r->lengthInSamples, bs = 512;
    AudioBuffer<float> in(2, len); r->read(&in, 0, len, 0, true, true);
    File out(argv[2]); out.createDirectory();
    for (int a = 3; a < argc; ++a) {
        String name(argv[a]); SpeakitProcessor p; int idx = -1;
        for (int i = 0; i < p.getNumPrograms(); ++i) if (p.getProgramName(i) == name) idx = i;
        if (idx < 0 && name != "CLEAN") { printf("no preset %s\n", argv[a]); return 1; }
        if (idx >= 0) p.setCurrentProgram(idx); else p.apvts.getParameter("bypass")->setValueNotifyingHost(1);
        p.setRateAndBufferSizeDetails(sr, bs); p.prepareToPlay(sr, bs); p.service(); p.waitForBay(); Thread::sleep(400);
        const int lat = p.getLatencySamples(); AudioBuffer<float> io(2, len + lat + bs); io.clear();
        for (int c = 0; c < 2; ++c) io.copyFrom(c, 0, in, c, 0, len);
        MidiBuffer m; for (int s = 0; s < io.getNumSamples(); s += bs) { int n = jmin(bs, io.getNumSamples() - s); float* ch[2] = { io.getWritePointer(0, s), io.getWritePointer(1, s) }; AudioBuffer<float> v(ch, 2, n); p.processBlock(v, m); m.clear(); }
        AudioBuffer<float> res(2, len); for (int c = 0; c < 2; ++c) res.copyFrom(c, 0, io, c, lat, len);
        String nn = String(a - 3).paddedLeft('0', 2);
        File wf = out.getChildFile(nn + ".wav"); wf.deleteFile();
        if (auto os = wf.createOutputStream()) { WavAudioFormat w; std::unique_ptr<AudioFormatWriter> wr(w.createWriterFor(os.release(), sr, 2, 24, {}, 0)); wr->writeFromAudioSampleBuffer(res, 0, len); }
        { std::unique_ptr<AudioProcessorEditor> ed(p.createEditor()); for (int i = 0; i < 30; ++i) MessageManager::getInstance()->runDispatchLoopUntil(20);
          auto img = ed->createComponentSnapshot(ed->getLocalBounds(), true, 1.0f); File pf = out.getChildFile(nn + ".png"); pf.deleteFile();
          if (auto os = pf.createOutputStream()) PNGImageFormat().writeImageToStream(img, *os);
          printf("%s  %s  latency %d  editor %dx%d\n", nn.toRawUTF8(), name.toRawUTF8(), lat, ed->getWidth(), ed->getHeight()); }
        p.releaseResources();
    }
    return 0;
}
