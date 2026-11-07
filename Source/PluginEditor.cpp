// Anal-Log editor: orange chassis, green screen with the signal chain, module pages, keycap navigation.
// Everything is laid out on the 1686 x 933 chassis and scaled as one.
#include "PluginEditor.h"
#include <BinaryData.h>
using namespace juce;

namespace {
const Colour ink(0xffe9e3d2), dimInk(0xff9a9483), amber(0xffff9a1f), orange(0xffff7a12), phos(0xff8dff9c), phosDim(0xff2e6a3a), keyInk(0xff3b362e), ledOn(0xff6bff4f);
constexpr float W = 1686.f, H = 933.f;
Image img(const char* n) { int s = 0; auto* d = BinaryData::getNamedResource(n, s); return ImageCache::getFromMemory(d, s); }
Font bar(float h, bool bold = true) { static auto a = Typeface::createSystemTypefaceFor(BinaryData::BarlowCondensed600_ttf, BinaryData::BarlowCondensed600_ttfSize), b = Typeface::createSystemTypefaceFor(BinaryData::BarlowCondensed700_ttf, BinaryData::BarlowCondensed700_ttfSize); return Font(FontOptions(bold ? b : a).withHeight(h)); }
Font vt(float h) { static auto a = Typeface::createSystemTypefaceFor(BinaryData::VT323Regular_ttf, BinaryData::VT323Regular_ttfSize); return Font(FontOptions(a).withHeight(h)); }

// Readable value text from a parameter, short for the scale around a knob, long for the readout under it.
String fmt(RangedAudioParameter* p, float v, bool shortForm) {
    if (auto* c = dynamic_cast<AudioParameterChoice*>(p)) return c->choices[jlimit(0, c->choices.size() - 1, (int)std::round(v))];
    if (dynamic_cast<AudioParameterBool*>(p)) return v > .5f ? "ON" : "OFF";
    auto u = p->getLabel(); auto r = p->getNormalisableRange();
    if (u == "dB") { if (shortForm) { int i = (int)std::round(v); return (i > 0 ? "+" : "") + String(i); } return (v > .05f ? "+" : "") + String(v, 1) + " dB"; }
    if (u == "Hz") { if (v >= 1000) { float k = v / 1000; String s = std::abs(k - std::round(k)) < .05f ? String((int)std::round(k)) : String(k, 1); return shortForm ? s + "k" : s + " kHz"; }
        if (shortForm) return v < 1 ? String(v, 2) : v < 10 ? String(v, 1) : String((int)std::round(v)); return (v < 10 ? String(v, 2) : v < 100 ? String(v, 1) : String((int)std::round(v))) + " Hz"; }
    if (u == "bit") { int b = (int)std::round(v); return String(b) + (shortForm ? "" : b == 1 ? " bit" : " bits"); }
    if (u == "ms") return String((int)std::round(v)) + (shortForm ? "" : " ms");
    if (u == "st") return (v > .5f ? "+" : "") + String((int)std::round(v)) + (shortForm ? "" : " st");
    if (r.start >= 0 && r.end <= 1.0001f) return String((int)std::round(v * 100)) + (shortForm ? "" : " %");
    if (r.start < 0 && r.end <= 1.0001f) return String(v, shortForm ? 1 : 2);
    return String(v, 1);
}

struct Lnf : LookAndFeel_V4 {
    Image knob = img("knob_dirty_png"); float menuFont = 24.f;
    void drawRotarySlider(Graphics& g, int, int, int w, int h, float pos, float a0, float a1, Slider&) override {
        float side = (float)jmin(w, h), cx = w * .5f, cy = h * .5f, r = side * .42f;
        g.setColour(ink.withAlpha(.85f)); for (int i = 0; i <= 10; ++i) { float a = a0 + (a1 - a0) * i / 10.f, r0 = r * 1.06f, r1 = r * (i % 5 == 0 ? 1.2f : 1.14f); g.drawLine(cx + std::sin(a) * r0, cy - std::cos(a) * r0, cx + std::sin(a) * r1, cy - std::cos(a) * r1, i % 5 == 0 ? 2.4f : 1.6f); }
        g.setColour(Colours::black.withAlpha(.55f)); g.fillEllipse(cx - r + 3, cy - r + 7, r * 2, r * 2);
        g.drawImage(knob, Rectangle<float>(cx - r, cy - r, r * 2, r * 2), RectanglePlacement::stretchToFit);
        float a = a0 + pos * (a1 - a0); Path p; p.addRoundedRectangle(-r * .05f, -r * .9f, r * .1f, r * .62f, r * .04f); g.setColour(Colour(0xff26221c)); g.fillPath(p, AffineTransform::rotation(a).translated(cx, cy)); }
    Font getPopupMenuFont() override { return bar(menuFont, false); }
    Rectangle<int> getTooltipBounds(const String& tip, Point<int> pos, Rectangle<int> parent) override { AttributedString s; s.append(tip, bar(24, false)); TextLayout tl; tl.createLayout(s, 420.f); int w = (int)tl.getWidth() + 28, h = (int)tl.getHeight() + 20;
        return Rectangle<int>(pos.x > parent.getCentreX() ? pos.x - w - 12 : pos.x + 18, pos.y > parent.getCentreY() ? pos.y - h - 8 : pos.y + 18, w, h).constrainedWithin(parent); }
    void drawTooltip(Graphics& g, const String& tip, int w, int h) override { auto b = Rectangle<float>((float)w, (float)h); g.setColour(Colour(0xf0121512)); g.fillRoundedRectangle(b, 6); g.setColour(phosDim.brighter(.4f)); g.drawRoundedRectangle(b.reduced(1), 6, 1.5f);
        AttributedString s; s.append(tip, bar(24, false), Colour(0xffd8ffd9)); TextLayout tl; tl.createLayout(s, (float)w - 28); tl.draw(g, b.reduced(14, 10)); }
    int getPopupMenuBorderSize() override { return roundToInt(menuFont * .4f); }
    void drawPopupMenuBackground(Graphics& g, int, int) override { g.fillAll(Colour(0xff121512)); }
    void drawPopupMenuItem(Graphics& g, const Rectangle<int>& area, bool sep, bool active, bool hl, bool ticked, bool sub, const String& text, const String& sc, const Drawable* icon, const Colour* col) override {
        setColour(PopupMenu::highlightedBackgroundColourId, Colour(0xff2e6a3a)); setColour(PopupMenu::textColourId, phos); setColour(PopupMenu::highlightedTextColourId, Colour(0xffd8ffd9));
        LookAndFeel_V4::drawPopupMenuItem(g, area, sep, active, hl, ticked, sub, text, sc, icon, col); }
};
void styled(PopupMenu& m, Component* at) { m.setLookAndFeel(&at->getLookAndFeel()); }
void choiceMenu(AudioParameterChoice* c, Component* at) { PopupMenu m; for (int i = 0; i < c->choices.size(); ++i) m.addItem(i + 1, c->choices[i], true, i == c->getIndex()); styled(m, at); m.showMenuAsync(PopupMenu::Options().withTargetComponent(at), [c](int x) { if (x > 0) { c->beginChangeGesture(); c->setValueNotifyingHost(c->convertTo0to1(float(x - 1))); c->endChangeGesture(); } }); }
void setParam(RangedAudioParameter* p, float v) { p->beginChangeGesture(); p->setValueNotifyingHost(p->convertTo0to1(v)); p->endChangeGesture(); }

using SA = AudioProcessorValueTreeState::SliderAttachment; using BA = AudioProcessorValueTreeState::ButtonAttachment;

// A cream knob with its name, a numbered scale and the value under it. Choices with few options become stepped switches.
struct CreamKnob : Component {
    Slider s; RangedAudioParameter* p; String title; std::unique_ptr<SA> att; int labels;
    CreamKnob(AudioProcessorValueTreeState& st, const String& id, const String& t) : p(st.getParameter(id)), title(t) { s.setSliderStyle(Slider::RotaryVerticalDrag); s.setTextBoxStyle(Slider::NoTextBox, false, 0, 0); s.setRotaryParameters(MathConstants<float>::pi * 1.25f, MathConstants<float>::pi * 2.75f, true); s.setPopupDisplayEnabled(false, false, nullptr); addAndMakeVisible(s); att = std::make_unique<SA>(st, id, s); s.onValueChange = [this] { repaint(); }; labels = 5; }
    Rectangle<int> dial; float side = 0; bool inl = false; float th = 30;
    void resized() override { auto b = getLocalBounds(); inl = b.getHeight() < 140; th = b.getHeight() < 210 ? 24.f : 30.f;
        if (inl) { side = jmin((float)b.getHeight() - 14, b.getWidth() * .4f); dial = Rectangle<int>((int)side, (int)side).withCentre({ (int)(side * .5f) + 8, b.getCentreY() }); labels = 0; }
        else { side = jmin(((float)b.getWidth() - 70) / .95f, ((float)b.getHeight() - th * 2 - 26) / 1.02f, 190.f); side = jmax(side, 40.f); dial = Rectangle<int>((int)side, (int)side).withCentre({ b.getCentreX(), (int)(th + 13 + side * .6f) }); labels = side < 90 ? 3 : 5; }
        s.setBounds(dial); }
    void paint(Graphics& g) override { auto& rg = p->getNormalisableRange(); auto val = fmt(p, rg.convertFrom0to1(p->getValue()), false);
        if (inl) { auto t = getLocalBounds().withLeft(dial.getRight() + 10); g.setColour(ink); g.setFont(bar(24)); g.drawFittedText(title, t.withTrimmedBottom(t.getHeight() / 2), Justification::bottomLeft, 1, .8f); g.setFont(bar(26, false)); g.drawFittedText(val, t.withTrimmedTop(t.getHeight() / 2 + 2), Justification::topLeft, 1, .8f); return; }
        g.setColour(ink); g.setFont(bar(th)); g.drawText(title, 0, 0, getWidth(), (int)th, Justification::centred);
        float r = side * .42f, cx = (float)dial.getCentreX(), cy = (float)dial.getCentreY(); g.setFont(bar(th < 30 ? 18.f : 20.f, false)); g.setColour(dimInk.brighter(.4f));
        if (auto* c = dynamic_cast<AudioParameterChoice*>(p)) { int n = c->choices.size(); for (int i = 0; i < n; ++i) { float q = n > 1 ? float(i) / (n - 1) : .5f, a = MathConstants<float>::pi * (1.25f + 1.5f * q); auto t = c->choices[i]; float rr = r * 1.42f; g.drawText(t, Rectangle<float>(90, 22).withCentre({ cx + std::sin(a) * rr * 1.12f + (q < .3f ? -14.f : q > .7f ? 14.f : 0.f), cy - std::cos(a) * rr }), Justification::centred); } }
        else for (int i = 0; i < labels; ++i) { float q = float(i) / (labels - 1), a = MathConstants<float>::pi * (1.25f + 1.5f * q), rr = r * 1.42f; g.drawText(fmt(p, rg.convertFrom0to1(q), true), Rectangle<float>(64, 22).withCentre({ cx + std::sin(a) * rr * 1.1f, cy - std::cos(a) * rr }), Justification::centred); }
        g.setColour(ink); g.setFont(bar(th < 30 ? 26.f : 30.f, false)); g.drawText(val, 0, getHeight() - (int)th - 4, getWidth(), (int)th + 4, Justification::centred); }
};
// Bat-handle toggle with OFF / ON engraved beside it.
// Bat-handle toggle: photographed switch (mirrored for OFF), OFF / ON engraved beside it.
struct Bat : ToggleButton { Image on = img("switch_on_png"), off = img("switch_off_png");
    void paintButton(Graphics& g, bool, bool) override { auto b = getLocalBounds().toFloat(); bool st = getToggleState(); auto area = b.withTrimmedBottom(24);
        float S = jmin(area.getHeight(), area.getWidth() * .62f); auto im = Rectangle<float>(S, S).withCentre(area.getCentre()); g.drawImage(st ? on : off, im, RectanglePlacement::stretchToFit);
        float nx = im.getX() + S * (st ? .49f : .51f), half = S * .3f; g.setFont(bar(20));
        g.setColour(st ? dimInk : ink); g.drawText("OFF", Rectangle<float>(nx - half - 50, b.getBottom() - 24, 50, 22), Justification::centredRight); g.setColour(st ? ink : dimInk); g.drawText("ON", Rectangle<float>(nx + half, b.getBottom() - 24, 50, 22), Justification::centredLeft); } };

struct BatCell : Component { Bat b; String title, sub; std::unique_ptr<BA> att; BatCell(AudioProcessorValueTreeState& st, const String& id, const String& t) : title(t) { addAndMakeVisible(b); att = std::make_unique<BA>(st, id, b); }
    int top() const { return 34 + (sub.isNotEmpty() ? 20 : 0); }
    void resized() override { b.setBounds(getLocalBounds().withTrimmedTop(top()).withSizeKeepingCentre(jmin(getWidth(), 130), jmin(getHeight() - top(), 110))); }
    void paint(Graphics& g) override { float th = getHeight() < 210 ? 24.f : 28.f; g.setColour(ink); g.setFont(bar(th)); g.drawText(title, 0, 0, getWidth(), (int)th, Justification::centred);
        if (sub.isNotEmpty()) { g.setColour(dimInk.brighter(.3f)); g.setFont(bar(18, false)); g.drawText(sub, 0, (int)th, getWidth(), 20, Justification::centred); } } };
// Green screen box for long choice lists (IRs, codecs, targets). Wide boxes get arrows to step through.
struct Lcd : Component { AudioParameterChoice* c; String title; bool wide; const IrInfo* ir = nullptr; int shown = -1; void refresh() { if (c->getIndex() != shown) { shown = c->getIndex(); repaint(); } }
    Lcd(AudioProcessorValueTreeState& st, const String& id, const String& t, bool w) : c(dynamic_cast<AudioParameterChoice*>(st.getParameter(id))), title(t), wide(w) {}
    bool inl() const { return !wide && getHeight() < 140; }
    Rectangle<float> box() const { if (inl()) return getLocalBounds().toFloat().withTrimmedTop(30).reduced(4, 0).withSizeKeepingCentre((float)getWidth() - 8, 50); return wide ? getLocalBounds().toFloat().withTrimmedLeft(170).reduced(0, 4) : getLocalBounds().toFloat().withTrimmedTop(34).withSizeKeepingCentre(jmin((float)getWidth() - 8, 260.f), 54); }
    void step(int d) { int n = c->choices.size(); setParam(c, float((c->getIndex() + d + n) % n)); }
    void mouseDown(const MouseEvent& e) override { auto b = box(); if (wide && e.position.x < b.getX() + 60) step(-1); else if (wide && e.position.x > b.getRight() - 60) step(1); else if (b.contains(e.position)) choiceMenu(c, this); }
    void paint(Graphics& g) override { auto b = box(); g.setColour(ink); g.setFont(bar(wide ? 32 : 28)); if (wide) g.drawText(title, 0, 0, 160, getHeight(), Justification::centredLeft); else g.drawText(title, 0, 0, getWidth(), 30, Justification::centred);
        g.setColour(Colour(0xff060d07)); g.fillRoundedRectangle(b, 8); g.setColour(Colour(0xff3a3a36)); g.drawRoundedRectangle(b, 8, 3); g.setColour(phos);
        String t = c->getCurrentChoiceName().toUpperCase(); if (ir) { auto& e = ir[c->getIndex()]; t = String(e.group).toUpperCase() + " / " + String(e.name).toUpperCase() + (e.modeled ? "  (MODELED)" : ""); }
        if (wide) { g.setFont(vt(46)); g.drawText("<", b.withWidth(60), Justification::centred); g.drawText(">", b.withLeft(b.getRight() - 60), Justification::centred); g.drawFittedText(t, b.reduced(64, 0).toNearestInt(), Justification::centred, 1, .7f); }
        else { g.setFont(vt(34)); g.drawFittedText(t + "  v", b.reduced(10, 0).toNearestInt(), Justification::centred, 1, .7f); } } };

struct Def { const char* id; const char* label; };
struct Help { const char* id; const char* sub; const char* tip; };
const Help* help(const String& id) { static const Help h[] = {
    { "crushAA", "FILTER BEFORE CRUSH", "Low-pass before the sample rate is lowered, set to the SAMPLE RATE. Removes the metallic aliasing: clean, old-sampler sound." },
    { "crushSmooth", "FILTER AFTER CRUSH", "Low-pass after the sample rate is lowered. Rounds off the steps between samples, like the output filter of an old converter." },
    { "crushGain", nullptr, "How hard the signal hits the crusher. The level is taken back off afterwards." },
    { "crushBits", nullptr, "Bit depth. Fewer bits = more grit and noise." },
    { "crushRate", nullptr, "Sample rate. Lower = more aliasing and less top end. 48k = off." },
    { "crush", nullptr, "Macro: lowers bits and sample rate together. Used by presets and the LFOs." },
    { "crushQuant", nullptr, "LINEAR: plain bit reduction. MU-LAW: telephone-style, quiet parts survive better." },
    { "crushJit", nullptr, "Unstable sample clock, like cheap digital hardware." },
    { "crushDither", nullptr, "Adds a little noise that smooths the bit steps." },
    { "crushFilt", nullptr, "Extra filter: low-pass, high-pass or band-pass." },
    { "crushPos", nullptr, "PRE: filter before the crusher. POST: after it." },
    { "driveLink", "OUTPUT FOLLOWS DRIVE", "Turns the output down as the drive goes up, so the level stays even." },
    { "driveBias", nullptr, "Shifts the curve off centre: adds even harmonics, like a tube." },
    { "driveAsym", nullptr, "Clips one side of the wave harder than the other." },
    { "driveTone", nullptr, "Low-pass after the distortion." },
    { "driveHpf", nullptr, "High-pass before the distortion. Keeps the lows from turning to mud." },
    { "muteStop", "SILENT WHEN STOPPED", "Mutes the backgrounds and tails when the transport stops." },
    { "limiter", "NEVER OVER 0 dB", "Safety limiter at the very end of the chain." },
    { "bayDuck", "QUIETER WHEN VOICE PLAYS", "Turns the Sample Bay down while the input is above the threshold." },
    { "modSync", "LOCK TO TEMPO", nullptr }, { "delaySync", "LOCK TO TEMPO", nullptr } };
    for (auto& x : h) if (id == x.id) return &x; return nullptr; }
struct ModuleInfo { const char* key; const char* title; const char* stage; const char* onId; std::vector<Def> ctl; int cols; const IrInfo* ir; const char* crt; };
const std::vector<ModuleInfo>& modules() { static const std::vector<ModuleInfo> m = {
    { "GATE", "GATE", "STAGE 1  PREPARE", "gateOn", { {"gate","THRESHOLD"},{"gateRange","RANGE"},{"gateRel","RELEASE"} }, 3, nullptr, "GATE" },
    { "COMP", "COMPRESSOR", "STAGE 1  PREPARE", "compOn", { {"comp","AMOUNT"},{"compRatio","RATIO"},{"compAtk","ATTACK"},{"compRel","RELEASE"},{"compGain","GAIN"} }, 5, nullptr, "COMP" },
    { "EQ", "EQUALIZER", "STAGE 1  PREPARE", "eqOn", { {"eq","NARROW"},{"eqHp","HPF"},{"eqLp","LPF"},{"eqLoF","LOW FREQ"},{"eqLoG","LOW GAIN"},{"eqHiF","HIGH FREQ"},{"eqHiG","HIGH GAIN"},{"eqP1F","PEAK 1"},{"eqP1G","P1 GAIN"},{"eqP1Q","P1 Q"},{"eqP2F","PEAK 2"},{"eqP2G","P2 GAIN"},{"eqP2Q","P2 Q"} }, 5, nullptr, "EQ" },
    { "DRIVE", "DRIVE", "STAGE 2  TRANSMIT", "driveOn", { {"drive","DRIVE"},{"driveTone","TONE"},{"drivePost","OUTPUT"},{"driveMix","MIX"},{"driveType","TYPE"},{"driveBias","BIAS"},{"driveAsym","ASYM"},{"driveHpf","HPF"},{"preEqLp","LPF"},{"driveLink","LINK"},{"driveCurve","CURVE"},{"preEqF","PRE EQ FREQ"},{"preEqG","PRE EQ GAIN"},{"preEqQ","PRE EQ Q"} }, 5, nullptr, "DRIVE" },
    { "TELECOM", "TELECOM", "STAGE 2  TRANSMIT", "telOn", { {"codec","CODEC"},{"line","DAMAGE"},{"telMix","MIX"} }, 2, nullptr, "TEL" },
    { "CRUSH", "CRUSH", "STAGE 2  TRANSMIT", "crushOn", { {"crushGain","GAIN"},{"crushBits","BITS"},{"crushRate","SAMPLE RATE"},{"crush","AMOUNT"},{"crushMix","MIX"},{"crushQuant","QUANTIZER"},{"crushJit","JITTER"},{"crushDither","DITHER"},{"crushAA","ANTI-ALIAS"},{"crushSmooth","SMOOTH"},{"crushFilt","FILTER"},{"crushCut","CUTOFF"},{"crushRes","RESONANCE"},{"crushPos","FILTER POS"} }, 5, nullptr, "CRUSH" },
    { "MOD", "MODULATION FX", "STAGE 2  TRANSMIT", "modOn", { {"modType","TYPE"},{"modDepth","DEPTH"},{"modSpeed","SPEED"},{"modSync","SYNC"},{"modDiv","NOTE"} }, 5, nullptr, "MOD" },
    { "PHONO", "PHONOGRAPH", "STAGE 2  TRANSMIT", "gramOn", { {"gramWow","WOW"},{"gramCurve","WARP"},{"gramTicks","TICKS"},{"gramGain","GAIN"} }, 4, nullptr, "PHONO" },
    { "TUNING", "RADIO TUNING", "STAGE 2  TRANSMIT", "tuneOn", { {"tuneX","OFF STATION"},{"tuneY","WHISTLE"},{"tuneMode","BAND"} }, 3, nullptr, "TUNE" },
    { "SPEAKER", "SPEAKER", "STAGE 3  PLAYBACK", "spkOn", { {"spkIdx","DEVICE"},{"spkLevel","LEVEL"},{"spkPos","POSITION"} }, 2, ir_spk, "SPKR" },
    { "COVER", "COVER", "STAGE 3  PLAYBACK", "coverOn", { {"coverIdx","OBJECT"},{"cover","MIX"},{"coverPitch","PITCH"} }, 2, ir_cov, "COVER" },
    { "MIC", "MICROPHONE", "STAGE 3  PLAYBACK", "micOn", { {"micIdx","MIC"},{"mic","MIX"} }, 1, ir_mic, "MIC" },
    { "ROTARY", "ROTARY SPEAKER", "STAGE 3  PLAYBACK", "leslieOn", { {"leslieMode","SPEED"},{"leslieLo","SLOW RATE"},{"leslieHi","FAST RATE"},{"leslieLazy","INERTIA"},{"leslieWidth","WIDTH"},{"leslieAmt","AMOUNT"} }, 3, nullptr, "ROTR" },
    { "DELAY", "DELAY", "STAGE 3  PLAYBACK", "delayOn", { {"delayL","TIME L"},{"delayR","TIME R"},{"delayFeed","FEEDBACK"},{"delayMix","MIX"},{"delaySync","SYNC"},{"delayDivL","NOTE L"},{"delayDivR","NOTE R"},{"delayX","CROSS"},{"delayTone","LP / HP"} }, 5, nullptr, "DELAY" },
    { "ROOM", "ROOM", "STAGE 3  PLAYBACK", "roomOn", { {"roomIdx","SPACE"},{"room","MIX"},{"roomDecay","DECAY"} }, 2, ir_rom, "ROOM" },
    { "IN/OUT", "INPUT / OUTPUT", "LEVELS AND SAFETY", nullptr, { {"input","INPUT"},{"output","OUTPUT"},{"makeup","PRESET LEVEL"},{"stereo","INPUT MODE"},{"limiter","LIMITER"},{"muteStop","MUTE ON STOP"} }, 3, nullptr, nullptr },
    { "LFO", "LFOS", "MOVE ANY CONTROL OVER TIME", nullptr, { {"lfo1Rate","LFO 1 SPEED"},{"lfo1Depth","LFO 1 DEPTH"},{"lfo1Wave","LFO 1 WAVE"},{"lfo1Dest","LFO 1 TARGET"},{"lfo2Rate","LFO 2 SPEED"},{"lfo2Depth","LFO 2 DEPTH"},{"lfo2Wave","LFO 2 WAVE"},{"lfo2Dest","LFO 2 TARGET"},{"lfo3Rate","LFO 3 SPEED"},{"lfo3Depth","LFO 3 DEPTH"},{"lfo3Wave","LFO 3 WAVE"},{"lfo3Dest","LFO 3 TARGET"},{"lfo4Rate","LFO 4 SPEED"},{"lfo4Depth","LFO 4 DEPTH"},{"lfo4Wave","LFO 4 WAVE"},{"lfo4Dest","LFO 4 TARGET"} }, 4, nullptr, nullptr },
    { "ENV", "ENVELOPES", "FOLLOW THE INPUT LEVEL", nullptr, { {"env1Amt","ENV 1 AMOUNT"},{"env1Spd","SPEED"},{"env1Dest","TARGET"},{"env2Amt","ENV 2 AMOUNT"},{"env2Spd","SPEED"},{"env2Dest","TARGET"} }, 3, nullptr, nullptr },
    { "SAMPLES", "SAMPLE BAY", "BACKGROUNDS AND SOUNDS, MIDI C1 AND UP", "bayOn", {}, 0, nullptr, nullptr },
    { "MEMORY", "MEMORY", "10 SNAPSHOTS OF EVERY SETTING", nullptr, {}, 0, nullptr, nullptr } };
    return m; }
constexpr int numModules = 15;

struct Panel { static Rectangle<int> content() { return { 650, 150, 970, 470 }; } };
// One page in the right panel. Built from the parameter types.
struct Page : Component {
    void refresh() { for (auto& c : ctl) if (auto* l = dynamic_cast<Lcd*>(c.get())) l->refresh(); }
    std::vector<std::unique_ptr<Component>> ctl; Lcd* top = nullptr; int cols;
    Page(SpeakitProcessor& p, const ModuleInfo& m) : cols(m.cols) { for (auto& d : m.ctl) { auto* pr = p.apvts.getParameter(d.id); jassert(pr != nullptr); std::unique_ptr<Component> c;
            if (auto* ch = dynamic_cast<AudioParameterChoice*>(pr)) { if (ch->choices.size() > 4) { bool wide = m.ir != nullptr || String(d.id) == "codec"; auto l = std::make_unique<Lcd>(p.apvts, d.id, d.label, wide); l->ir = m.ir; if (wide) top = l.get(); c = std::move(l); } else c = std::make_unique<CreamKnob>(p.apvts, d.id, d.label); }
            else if (dynamic_cast<AudioParameterBool*>(pr)) c = std::make_unique<BatCell>(p.apvts, d.id, d.label); else c = std::make_unique<CreamKnob>(p.apvts, d.id, d.label);
            if (auto* hp = help(d.id)) { if (auto* bc = dynamic_cast<BatCell*>(c.get())) { if (hp->sub) bc->sub = hp->sub; if (hp->tip) bc->b.setTooltip(hp->tip); } else if (auto* ck = dynamic_cast<CreamKnob*>(c.get())) { if (hp->tip) ck->s.setTooltip(hp->tip); } }
            addAndMakeVisible(*c); ctl.push_back(std::move(c)); } }
    void resized() override { auto r = getLocalBounds(); if (top) { top->setBounds(r.removeFromTop(86)); r.removeFromTop(18); } std::vector<Component*> rest; for (auto& c : ctl) if (c.get() != top) rest.push_back(c.get()); if (rest.empty()) return;
        int n = (int)rest.size(), cc = jmax(1, jmin(cols, n)), rows = (n + cc - 1) / cc; int cw = r.getWidth() / jmax(cc, 2), rh = jmin(r.getHeight() / rows, 260);
        for (int i = 0; i < n; ++i) { int row = i / cc, inRow = jmin(cc, n - row * cc); int x0 = r.getX() + (r.getWidth() - inRow * cw) / 2; rest[(size_t)i]->setBounds(x0 + (i % cc) * cw, r.getY() + row * rh + (r.getHeight() - rows * rh) / 2, cw, rh); } } };

// Keycaps under the screen: modules in signal order, then the utility pages.
struct Key : Component { String label; bool selected = false, lit = false, utility = false, hasLed = true; std::function<void()> select, toggle; Image cap = img("keycap_png");
    void mouseDown(const MouseEvent& e) override { if (hasLed && toggle && e.x > getWidth() - 34 && e.y < 30) toggle(); else if (select) select(); }
    void paint(Graphics& g) override { auto b = getLocalBounds().toFloat(); if (selected) { g.setColour(orange); g.fillRoundedRectangle(b.getX() + 8, b.getBottom() - 6, b.getWidth() - 16, 5, 2); b = b.withTrimmedBottom(10).translated(0, 3); } else b = b.withTrimmedBottom(10);
        g.setColour(Colours::black.withAlpha(.6f)); g.fillRoundedRectangle(b.translated(0, selected ? 2.f : 6.f), 8); g.drawImage(cap, b, RectanglePlacement::stretchToFit);
        if (utility) { g.setColour(Colour(0x28301e08)); g.fillRoundedRectangle(b.reduced(4), 8); } if (selected) { g.setColour(Colours::black.withAlpha(.12f)); g.fillRoundedRectangle(b.reduced(2), 8); }
        g.setColour(keyInk); g.setFont(bar(label.length() > 7 ? 23.f : 27.f)); g.drawText(label, b.reduced(6, 0).translated(0, 4), Justification::centred);
        if (hasLed) { auto l = Rectangle<float>(b.getRight() - 30, b.getY() + 12, 14, 9); if (lit) { g.setColour(ledOn.withAlpha(.35f)); g.fillRoundedRectangle(l.expanded(4), 4); } g.setColour(lit ? ledOn : Colour(0xff4a463c)); g.fillRoundedRectangle(l, 2); } } };

struct Pad : Component { SpeakitProcessor& p; int bank, slot; Slider gain; Pad(SpeakitProcessor& q, int b, int s) : p(q), bank(b), slot(s) { gain.setSliderStyle(Slider::RotaryVerticalDrag); gain.setTextBoxStyle(Slider::NoTextBox, false, 0, 0); gain.setRotaryParameters(MathConstants<float>::pi * 1.25f, MathConstants<float>::pi * 2.75f, true); gain.setRange(-48, 12); gain.setDoubleClickReturnValue(true, 0); addAndMakeVisible(gain); gain.onValueChange = [this] { p.bay[bank][slot].gain = Decibels::decibelsToGain((float)gain.getValue()); }; }
    void sync() { gain.setValue(Decibels::gainToDecibels(p.bay[bank][slot].gain.load()), dontSendNotification); } void resized() override { gain.setBounds(getWidth() - 74, getHeight() / 2 - 36, 72, 72); }
    void mouseDown(const MouseEvent&) override { p.setSlot(bank, slot, !p.bay[bank][slot].on.load()); repaint(); }
    void paint(Graphics& g) override { auto b = getLocalBounds().toFloat().reduced(4); bool on = p.bay[bank][slot].on.load(); g.setColour(Colour(0xff060d07)); g.fillRoundedRectangle(b, 8); g.setColour(on ? phos : Colour(0xff3a3a36)); g.drawRoundedRectangle(b, 8, on ? 3.f : 2.f);
        g.setColour(on ? phos : phos.withAlpha(.6f)); g.setFont(vt(32)); g.drawFittedText(String(bayTable[bank][slot].name).toUpperCase(), b.reduced(14, 6).withTrimmedRight(70).withTrimmedBottom(24).toNearestInt(), Justification::centredLeft, 1, .7f);
        g.setFont(vt(22)); g.setColour(phosDim.brighter(.6f)); g.drawText(MidiMessage::getMidiNoteName(36 + bank * 12 + slot, true, true, 3) + (bayTable[bank][slot].loop ? "  LOOP" : "  ONE SHOT"), b.reduced(14, 6), Justification::bottomLeft); } };
struct Flat : Button { String tx; bool lit = false; Flat(String t) : Button(t), tx(t) {} void paintButton(Graphics& g, bool o, bool) override { auto b = getLocalBounds().toFloat().reduced(2); g.setColour(lit ? Colour(0xff2e6a3a) : Colour(0xff060d07)); g.fillRoundedRectangle(b, 6); g.setColour(lit ? phos : Colour(0xff3a3a36).brighter(o ? .4f : 0.f)); g.drawRoundedRectangle(b, 6, 2); g.setColour(phos); g.setFont(vt(30)); g.drawText(tx, b, Justification::centred); } };
}

// The screen: name, version, preset browser, live signal chain, scope or drive curve, meters.
struct Crt : Component { SpeakitProcessor& p; std::function<int()> sel; std::function<void(int)> pick; std::function<bool(int)> isOn; std::function<void(int)> step; std::function<void()> presetMenu; std::vector<std::pair<Rectangle<float>, int>> hits; Rectangle<float> prevR, nextR, nameR;
    Image bg = img("chassis_jpg"); Crt(SpeakitProcessor& q) : p(q) { setOpaque(true); }
    void mouseDown(const MouseEvent& e) override { for (auto& h : hits) if (h.first.contains(e.position)) { pick(h.second); return; } if (prevR.contains(e.position)) step(-1); else if (nextR.contains(e.position)) step(1); else if (nameR.contains(e.position)) presetMenu(); }
    void paint(Graphics& g) override { auto full = getLocalBounds().toFloat(); auto r = full.reduced(30, 24); hits.clear(); g.drawImage(bg, 0, 0, getWidth(), getHeight(), 87, 129, getWidth(), getHeight());
        g.setColour(phos); g.setFont(vt(64)); g.drawText("Anal-Log", r.getX(), r.getY() - 8, 300, 60, Justification::centredLeft); g.setFont(vt(26)); g.setColour(phos.withAlpha(.8f)); g.drawText("v" + String(JucePlugin_VersionString), r.getRight() - 120, r.getY() + 10, 120, 26, Justification::centredRight);
        g.drawText("by Yishay Raziel", r.getRight() - 200, r.getY() + 34, 200, 24, Justification::centredRight);
        float y = r.getY() + 58; g.setColour(phos.withAlpha(.6f)); g.fillRect(r.getX(), y, r.getWidth(), 2.f); y += 12;
        // preset row
        g.setColour(phos); g.setFont(vt(38)); prevR = { r.getX(), y, 36, 40 }; nextR = { r.getRight() - 36, y, 36, 40 }; nameR = { r.getX() + 40, y, r.getWidth() - 80, 40 };
        g.drawText("<", prevR, Justification::centred); g.drawText(">", nextR, Justification::centred); g.setFont(vt(32)); g.drawFittedText(p.presetName() + (p.isModified() ? " *" : ""), nameR.toNearestInt(), Justification::centred, 1, .7f); y += 44;
        int s = sel ? sel() : 0; auto& ms = modules(); g.setFont(vt(30)); g.setColour(phos); String state = s < numModules ? String(ms[(size_t)s].title) + " / " + (isOn(s) ? "ON" : "OFF") : String(ms[(size_t)s].title); g.drawText("> " + state, r.getX(), y, r.getWidth(), 32, Justification::centredLeft); y += 36;
        // signal chain, three rows, in processing order
        static const int rows[3][6] = { { -1, 0, 1, 2, -2, -2 }, { 3, 4, 5, 6, 7, 8 }, { 9, 10, 11, 12, 13, 14 } }; float bw = (r.getWidth() - 34) / 6.f, bh = 34;
        for (int row = 0; row < 3; ++row) { float ry = y + row * 44; g.setColour(phos.withAlpha(.7f)); g.setFont(vt(26)); g.drawText(String(row + 1), r.getX(), ry, 24, bh, Justification::centred);
            for (int c = 0; c < 6; ++c) { int m = rows[row][c]; if (m == -2) continue; auto b = Rectangle<float>(r.getX() + 32 + c * bw, ry, bw - 8, bh); if (c > 0) { g.setColour(phos.withAlpha(.5f)); g.fillRect(b.getX() - 8, b.getCentreY() - 1, 8.f, 2.f); }
                if (m == -1) { g.setColour(phos); g.drawRect(b, 1.5f); g.setFont(vt(26)); g.drawText("IN", b, Justification::centred); continue; }
                bool on = isOn(m), here = m == s; hits.push_back({ b, m }); if (on) { g.setColour(phos.withAlpha(here ? 1.f : .82f)); g.fillRect(b); g.setColour(Colour(0xff06210c)); } else { g.setColour(phos.withAlpha(here ? 1.f : .4f)); g.drawRect(b, here ? 2.5f : 1.5f); }
                g.setFont(vt(24)); g.drawText(ms[(size_t)m].crt, b, Justification::centred); if (here) { g.setColour(phos); g.fillRect(b.getX(), b.getBottom() + 3, b.getWidth(), 3.f); } }
            if (row < 2) { float ex = row == 0 ? r.getX() + 32 + 3 * bw + bw - 8 : r.getX() + 32 + 6 * bw - 8; g.setColour(phos.withAlpha(.5f)); g.fillRect(ex, ry + bh / 2 - 1, 8.f, 2.f); } }
        { float ry = y + 2 * 44; g.setColour(phos); g.setFont(vt(24)); g.drawText("OUT", r.getRight() - 4, ry, 40, bh, Justification::centredLeft); } y += 3 * 44 + 6;
        // drive curve on the DRIVE page, otherwise a scrolling picture of the output
        auto gr = Rectangle<float>(r.getX(), y, r.getWidth(), 60); g.setColour(phos.withAlpha(.18f)); for (int i = 1; i < 4; ++i) g.fillRect(gr.getX(), gr.getY() + gr.getHeight() * i / 4, gr.getWidth(), 1.f); for (int i = 1; i < 8; ++i) g.fillRect(gr.getX() + gr.getWidth() * i / 8, gr.getY(), 1.f, gr.getHeight());
        g.setColour(phos); Path path; if (s == 3) { float dr = *p.apvts.getRawParameterValue("drive"), c = *p.apvts.getRawParameterValue("driveCurve"), bi = *p.apvts.getRawParameterValue("driveBias") * .35f, as = *p.apvts.getRawParameterValue("driveAsym"); int t = (int)*p.apvts.getRawParameterValue("driveType"); float gain = Decibels::decibelsToGain(dr) * .25f, norm = 1.f;
                for (int i = 0; i <= 120; ++i) { float x = -1 + 2 * i / 120.f, v = x * gain + bi; v *= v >= 0 ? 1 + as : 1 - as; norm = jmax(norm, std::abs(sk::Drive::shape(v, t, c))); }
                for (int i = 0; i <= 120; ++i) { float x = -1 + 2 * i / 120.f, v = x * gain + bi; v *= v >= 0 ? 1 + as : 1 - as; float yv = sk::Drive::shape(v, t, c) / norm; Point<float> pt(gr.getX() + gr.getWidth() * i / 120.f, gr.getCentreY() - yv * gr.getHeight() * .46f); if (i == 0) path.startNewSubPath(pt); else path.lineTo(pt); } }
        else { int w = p.scopeW.load(); for (int i = 0; i < 256; ++i) { float v = jmin(1.f, p.scope[(w + i) % 256].load()); float x = gr.getX() + gr.getWidth() * i / 255.f; path.startNewSubPath(x, gr.getCentreY() - v * gr.getHeight() * .48f); path.lineTo(x, gr.getCentreY() + v * gr.getHeight() * .48f); } }
        g.strokePath(path, PathStrokeType(s == 3 ? 3.f : 1.6f)); y += 72;
        // meters
        auto meter = [&](const char* lab, float db, float my) { g.setFont(vt(26)); g.setColour(phos); g.drawText(lab, r.getX(), my, 50, 22, Justification::centredLeft); auto m = Rectangle<float>(r.getX() + 54, my + 4, r.getWidth() - 54, 14); g.setColour(phos.withAlpha(.16f)); g.fillRect(m); float v = jlimit(0.f, 1.f, (db + 60) / 60); g.setColour(db > -1 ? Colour(0xffffb35c) : phos); g.fillRect(m.withWidth(m.getWidth() * v));
            g.setColour(phos.withAlpha(.7f)); for (float t : { -18.f, -6.f }) g.fillRect(m.getX() + m.getWidth() * (t + 60) / 60, m.getY() - 4, 2.f, m.getHeight() + 8); };
        meter("IN", p.inDb.load(), y); meter("OUT", p.outDb.load(), y + 26);
        for (float sy = 0; sy < full.getHeight(); sy += 4) { g.setColour(Colours::black.withAlpha(.13f)); g.fillRect(0.f, sy, full.getWidth(), 1.5f); } } };

struct SpeakitEditor::Impl : Component {
    SpeakitProcessor& p; Lnf lnf; TooltipWindow tips { this, 500 }; Image chassis = img("chassis_jpg"); int sel = 3; Crt crt; std::vector<std::unique_ptr<Key>> keys; std::vector<std::unique_ptr<Component>> pages; Bat moduleSw; std::unique_ptr<BA> moduleAtt; std::unique_ptr<CreamKnob> wet; Bat bypass; std::unique_ptr<BA> bypassAtt;
    std::unique_ptr<Pad> pads[5][12]; Flat bankB[5] { Flat(bayBanks[0]), Flat(bayBanks[1]), Flat(bayBanks[2]), Flat(bayBanks[3]), Flat(bayBanks[4]) }; Flat prePost { "POST" }; std::unique_ptr<CreamKnob> bayGain, bayThr; std::unique_ptr<BatCell> bayDuck; std::vector<std::unique_ptr<Key>> snaps; Component samplesPage, memoryPage;
    float V(const char* id) { return *p.apvts.getRawParameterValue(id); }
    bool on(int m) { if (m == 4) return V("telOn") > .5f && V("codec") > .5f; auto* id = modules()[(size_t)m].onId; return id && V(id) > .5f; }
    void toggle(int m) { if (m == 4) { if (V("codec") < .5f) { setParam(p.apvts.getParameter("codec"), 4); setParam(p.apvts.getParameter("telOn"), 1); } else setParam(p.apvts.getParameter("telOn"), V("telOn") > .5f ? 0.f : 1.f); return; } auto* id = modules()[(size_t)m].onId; if (id) setParam(p.apvts.getParameter(id), V(id) > .5f ? 0.f : 1.f); }
    Impl(SpeakitProcessor& q) : p(q), crt(q) { setLookAndFeel(&lnf); setSize((int)W, (int)H); auto& ms = modules();
        addAndMakeVisible(crt); crt.setBounds(87, 129, 478, 463); crt.sel = [this] { return sel; }; crt.pick = [this](int m) { show(m); }; crt.isOn = [this](int m) { return on(m); }; crt.step = [this](int d) { int n = p.getNumPrograms(); p.loadPreset((p.getCurrentProgram() + d + n) % n); syncPads(); }; crt.presetMenu = [this] { PopupMenu m; for (int i = 0; i < p.getNumPrograms(); ++i) m.addItem(i + 1, p.getProgramName(i), true, i == p.getCurrentProgram()); styled(m, &crt); m.showMenuAsync(PopupMenu::Options().withTargetComponent(crt), [this](int x) { if (x > 0) { p.loadPreset(x - 1); syncPads(); } }); };
        static const int order[20] = { 0,1,2,3,4,5,6,7,8,9, 10,11,12,13,14, 15,16,17,18,19 };
        for (int i = 0; i < 20; ++i) { int m = order[i]; auto k = std::make_unique<Key>(); k->label = ms[(size_t)m].key; k->utility = m >= numModules; k->hasLed = m < numModules || m == 16 || m == 17 || m == 18; k->select = [this, m] { show(m); }; if (m < numModules) k->toggle = [this, m] { toggle(m); };
            int row = i / 10, col = i % 10; k->setBounds(72 + col * 116 + (row == 1 && col >= 5 ? 10 : 0), 696 + row * 98, 106, 92); addAndMakeVisible(*k); keys.push_back(std::move(k)); }
        for (size_t m = 0; m < ms.size(); ++m) { if (ms[m].ctl.empty()) { pages.push_back(nullptr); continue; } auto pg = std::make_unique<Page>(p, ms[m]); pg->setBounds(Panel::content()); addChildComponent(*pg); pages.push_back(std::move(pg)); }
        addChildComponent(moduleSw); moduleSw.setBounds(1480, 56, 150, 92); moduleSw.onClick = [this] { if (sel == 4) { bool want = moduleSw.getToggleState(); if (want && V("codec") < .5f) setParam(p.apvts.getParameter("codec"), 4); setParam(p.apvts.getParameter("telOn"), want ? 1.f : 0.f); } };
        wet = std::make_unique<CreamKnob>(p.apvts, "wet", "DRY / WET"); wet->setBounds(1290, 684, 220, 208); addAndMakeVisible(*wet);
        addAndMakeVisible(bypass); bypass.setBounds(1515, 735, 130, 110); bypassAtt = std::make_unique<BA>(p.apvts, "bypass", bypass);
        // samples page
        samplesPage.setBounds(Panel::content()); addChildComponent(samplesPage); for (int b = 0; b < 5; ++b) { samplesPage.addAndMakeVisible(bankB[b]); bankB[b].setBounds(b * 136, 0, 128, 52); bankB[b].onClick = [this, b] { p.bankShown = b; showBank(); };
            for (int s = 0; s < 12; ++s) { pads[b][s] = std::make_unique<Pad>(p, b, s); pads[b][s]->setBounds((s % 4) * 242, 64 + (s / 4) * 92, 240, 88); samplesPage.addChildComponent(*pads[b][s]); } }
        samplesPage.addAndMakeVisible(prePost); prePost.setBounds(700, 0, 270, 52); prePost.onClick = [this] { int b = p.bankShown; p.bankPre[b] = !p.bankPre[b].load(); showBank(); };
        bayGain = std::make_unique<CreamKnob>(p.apvts, "bayGain", "GAIN"); bayGain->setBounds(80, 340, 230, 130); bayDuck = std::make_unique<BatCell>(p.apvts, "bayDuck", "DUCK UNDER VOICE"); bayDuck->b.setTooltip(help("bayDuck")->tip); bayDuck->setBounds(380, 344, 220, 120); bayThr = std::make_unique<CreamKnob>(p.apvts, "bayThr", "DUCK THRESHOLD"); bayThr->setBounds(660, 340, 230, 130);
        for (auto* c : std::initializer_list<Component*> { bayGain.get(), bayDuck.get(), bayThr.get() }) samplesPage.addAndMakeVisible(c);
        // memory page
        memoryPage.setBounds(Panel::content()); addChildComponent(memoryPage); for (int i = 0; i < 10; ++i) { auto k = std::make_unique<Key>(); k->label = String(i + 1); k->select = [this, i] { if (ModifierKeys::getCurrentModifiers().isShiftDown() || !p.hasSnap(i)) p.storeSnap(i); else p.recallSnap(i); }; k->setBounds(70 + (i % 5) * 170, 40 + (i / 5) * 150, 150, 132); memoryPage.addAndMakeVisible(*k); snaps.push_back(std::move(k)); }
        show(3); }
    ~Impl() override { setLookAndFeel(nullptr); }
    void syncPads() { for (auto& b : pads) for (auto& s : b) s->sync(); }
    void showBank() { int sb = p.bankShown; for (int b = 0; b < 5; ++b) { bankB[b].lit = b == sb; bankB[b].repaint(); for (int s = 0; s < 12; ++s) { pads[b][s]->setVisible(b == sb); if (b == sb) pads[b][s]->sync(); } } prePost.tx = p.bankPre[sb].load() ? "PLAYS BEFORE CHAIN" : "PLAYS AFTER CHAIN"; prePost.lit = p.bankPre[sb].load(); prePost.repaint(); }
    void show(int m) { sel = m; for (size_t i = 0; i < pages.size(); ++i) if (pages[i]) pages[i]->setVisible((int)i == m); samplesPage.setVisible(m == 18); memoryPage.setVisible(m == 19); if (m == 18) showBank();
        auto* id = modules()[(size_t)m].onId; moduleAtt.reset(); moduleSw.setVisible(id != nullptr); if (id && m != 4) moduleAtt = std::make_unique<BA>(p.apvts, id, moduleSw); tick(); repaint(); }
    void tick() { auto& ms = modules(); for (size_t i = 0; i < keys.size(); ++i) { int m = (int)i; bool lit = m < numModules ? on(m) : m == 16 ? (V("lfo1Dest") + V("lfo2Dest") + V("lfo3Dest") + V("lfo4Dest")) > .5f : m == 17 ? (V("env1Dest") + V("env2Dest")) > .5f && (std::abs(V("env1Amt")) + std::abs(V("env2Amt"))) > .001f : m == 18 ? [&] { for (auto& b : p.bay) for (auto& s : b) if (s.on.load()) return true; return false; }() : false;
            if (keys[i]->lit != lit || keys[i]->selected != (m == sel)) { keys[i]->lit = lit; keys[i]->selected = m == sel; keys[i]->repaint(); } }
        if (sel == 4) moduleSw.setToggleState(on(4), dontSendNotification); bool active = sel >= numModules ? (sel != 18 || V("bayOn") > .5f) : on(sel); float a = active ? 1.f : .42f; if (sel < (int)pages.size() && pages[(size_t)sel] && pages[(size_t)sel]->getAlpha() != a) pages[(size_t)sel]->setAlpha(a);
        for (int i = 0; i < (int)snaps.size(); ++i) { bool l = p.hasSnap(i), s = i == p.snapNow; if (snaps[(size_t)i]->lit != l || snaps[(size_t)i]->selected != s) { snaps[(size_t)i]->lit = l; snaps[(size_t)i]->selected = s; snaps[(size_t)i]->repaint(); } }
        if (sel == 18) for (auto& s : pads[p.bankShown.load()]) s->repaint(); crt.repaint(); (void)ms;
        if (sel < (int)pages.size() && pages[(size_t)sel]) if (auto* pg = dynamic_cast<Page*>(pages[(size_t)sel].get())) pg->refresh();
        int hs = modules()[(size_t)sel].onId ? (sel == 4 ? on(4) : V(modules()[(size_t)sel].onId) > .5f) : -1; if (hs != headerOn) { headerOn = hs; repaint(1340, 60, 150, 80); } }
    int headerOn = -2;
    void paint(Graphics& g) override { g.drawImage(chassis, getLocalBounds().toFloat()); auto& m = modules()[(size_t)sel];
        g.setColour(ink); g.setFont(bar(54)); g.drawText(m.title, 650, 62, 760, 56, Justification::centredLeft); g.setColour(dimInk); g.setFont(bar(25, false)); g.drawText(m.stage, 652, 112, 760, 26, Justification::centredLeft);
        g.setColour(orange); g.fillRect(650.f, 144.f, 970.f, 3.f); g.setColour(amber); g.fillRect(650.f, 150.f, 970.f, 2.f);
        if (m.onId) { bool o = sel == 4 ? on(4) : V(m.onId) > .5f; g.setColour(o ? amber : dimInk); g.setFont(bar(40)); g.drawText(o ? "ON" : "OFF", 1360, 70, 120, 48, Justification::centredRight); g.setFont(bar(20)); g.setColour(dimInk); g.drawText(sel == 18 ? "SAMPLE BAY" : "MODULE", 1360, 108, 120, 22, Justification::centredRight); }
        if (sel == 19) { g.setColour(ink); g.setFont(bar(26, false)); g.drawText("Click an empty slot to store the current settings. Click a stored slot to recall it. Shift-click to overwrite.", 650, 500, 970, 30, Justification::centred); }
        g.setColour(ink); g.setFont(bar(28)); g.drawText("BYPASS", 1520, 702, 120, 30, Justification::centred); }
};

SpeakitEditor::SpeakitEditor(SpeakitProcessor& q) : AudioProcessorEditor(q), p(q) { d = std::make_unique<Impl>(q); addAndMakeVisible(*d); setResizable(true, true); setResizeLimits(900, (int)(900 * H / W), 1686, 933); getConstrainer()->setFixedAspectRatio(W / H); setSize(1350, (int)(1350 * H / W)); startTimerHz(24); }
SpeakitEditor::~SpeakitEditor() { stopTimer(); d = nullptr; }
void SpeakitEditor::resized() { if (d) { float k = getWidth() / W; d->setTransform(AffineTransform::scale(k)); d->lnf.menuFont = 19.f / k; } }
void SpeakitEditor::setPage(int i) { d->show(i); }
void SpeakitEditor::testMenu(int i) { if (i == 0) d->crt.presetMenu(); else { d->show(9); if (auto* pg = dynamic_cast<Page*>(d->pages[9].get())) if (pg->top) choiceMenu(pg->top->c, pg->top); } }
void SpeakitEditor::timerCallback() { d->tick(); }
