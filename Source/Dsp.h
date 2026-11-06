// Speakit DSP modules. All original code. Control values arrive per 32-sample slot and are smoothed per sample.
#pragma once
#include <juce_dsp/juce_dsp.h>
#include <vector>
namespace sk {
using juce::MathConstants; constexpr float twoPi = MathConstants<float>::twoPi;
struct Smooth { float v = 0, c = 0.02f; void setTime(double sr, double ms) { c = 1.f - std::exp(-1.f / float(sr * ms * 0.001)); } float next(float t) { v += (t - v) * c; return v; } };
inline float db2g(float d) { return std::pow(10.f, d / 20.f); }
inline float g2db(float g) { return 20.f * std::log10(juce::jmax(g, 1e-7f)); }

struct Biquad { float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1[2] {}, z2[2] {};
    void reset() { z1[0] = z1[1] = z2[0] = z2[1] = 0; }
    float tick(float x, int c) { float y = b0 * x + z1[c]; z1[c] = b1 * x - a1 * y + z2[c]; z2[c] = b2 * x - a2 * y; return y; }
    void set(float nb0, float nb1, float nb2, float a0, float na1, float na2) { b0 = nb0 / a0; b1 = nb1 / a0; b2 = nb2 / a0; a1 = na1 / a0; a2 = na2 / a0; }
    void lp(float f, float q, float sr) { float w = twoPi * juce::jmin(f, sr * .45f) / sr, a = std::sin(w) / (2 * q), c = std::cos(w); set((1 - c) / 2, 1 - c, (1 - c) / 2, 1 + a, -2 * c, 1 - a); }
    void hp(float f, float q, float sr) { float w = twoPi * juce::jmin(f, sr * .45f) / sr, a = std::sin(w) / (2 * q), c = std::cos(w); set((1 + c) / 2, -1 - c, (1 + c) / 2, 1 + a, -2 * c, 1 - a); }
    void peak(float f, float q, float db, float sr) { float A = std::pow(10.f, db / 40), w = twoPi * juce::jmin(f, sr * .45f) / sr, a = std::sin(w) / (2 * q), c = std::cos(w); set(1 + a * A, -2 * c, 1 - a * A, 1 + a / A, -2 * c, 1 - a / A); }
    void lshelf(float f, float db, float sr) { float A = std::pow(10.f, db / 40), w = twoPi * f / sr, c = std::cos(w), s = std::sin(w) / 2 * std::sqrt(2.f), r = 2 * std::sqrt(A) * s;
        set(A * ((A + 1) - (A - 1) * c + r), 2 * A * ((A - 1) - (A + 1) * c), A * ((A + 1) - (A - 1) * c - r), (A + 1) + (A - 1) * c + r, -2 * ((A - 1) + (A + 1) * c), (A + 1) + (A - 1) * c - r); }
    void hshelf(float f, float db, float sr) { float A = std::pow(10.f, db / 40), w = twoPi * juce::jmin(f, sr * .45f) / sr, c = std::cos(w), s = std::sin(w) / 2 * std::sqrt(2.f), r = 2 * std::sqrt(A) * s;
        set(A * ((A + 1) + (A - 1) * c + r), -2 * A * ((A - 1) + (A + 1) * c), A * ((A + 1) + (A - 1) * c - r), (A + 1) - (A - 1) * c + r, 2 * ((A - 1) - (A + 1) * c), (A + 1) - (A - 1) * c - r); }
};
struct Delay { std::vector<float> b; int w = 0, n = 0; void size(int s) { n = s; b.assign((size_t)s, 0.f); w = 0; } void clear() { std::fill(b.begin(), b.end(), 0.f); }
    void push(float x) { b[(size_t)w] = x; if (++w >= n) w = 0; } float at(float d) const { float p = float(w) - 1.f - d; while (p < 0) p += n; int i = int(p); float f = p - i; int j = i + 1 >= n ? 0 : i + 1; return b[(size_t)i] + (b[(size_t)j] - b[(size_t)i]) * f; } };

struct Gate { double sr = 48000; float env = 0, g = 1; int hold = 0; void prepare(double s) { sr = s; env = 0; g = 1; }
    void process(float* L, float* R, int n, const float* mix, float thr, float rangeDb, float relMs) { float atk = 1 - std::exp(-1 / float(sr * .001)), rel = 1 - std::exp(-1 / float(sr * relMs * .001)), floor = db2g(rangeDb), t = db2g(thr);
        for (int i = 0; i < n; ++i) { float d = juce::jmax(std::abs(L[i]), std::abs(R[i])); env += (d - env) * (d > env ? .2f : .002f); bool open = env > t; if (open) hold = int(sr * .02); else if (hold > 0) --hold;
            float tg = (open || hold > 0) ? 1.f : floor; g += (tg - g) * (tg > g ? atk : rel); float m = mix[i >> 5], o = 1 + (g - 1) * m; L[i] *= o; R[i] *= o; } } };
struct Comp { double sr = 48000; float env = 0, gr = 1; void prepare(double s) { sr = s; env = 0; }
    void process(float* L, float* R, int n, const float* mix, float thr, float ratio, float atkMs, float relMs, float gainDb, float& meter) { float a = 1 - std::exp(-1 / float(sr * atkMs * .001)), r = 1 - std::exp(-1 / float(sr * relMs * .001)), mk = db2g(gainDb); float mn = 1;
        for (int i = 0; i < n; ++i) { float d = juce::jmax(std::abs(L[i]), std::abs(R[i])); env += (d - env) * (d > env ? a : r); float over = g2db(env) - thr; float tg = over > 0 ? db2g(-over * (1 - 1 / ratio)) : 1.f; gr = tg; mn = juce::jmin(mn, gr);
            float m = mix[i >> 5], o = 1 + (gr * mk - 1) * m; L[i] *= o; R[i] *= o; } meter = mn; } };
struct Eq { Biquad hp, lp, lo, hi, p1, p2, nHp, nLp, nMid; double sr = 48000; void prepare(double s) { sr = s; for (auto* b : {&hp,&lp,&lo,&hi,&p1,&p2,&nHp,&nLp,&nMid}) b->reset(); }
    // band: main-page macro, 0 wide to 1 telephone narrow
    void process(float* L, float* R, int n, const float* mix, const float* band, float hpf, float lpf, float lf, float lg, float hf, float hg, float f1, float g1, float q1, float f2, float g2, float q2) { float s = (float)sr; hp.hp(hpf, .707f, s); lp.lp(lpf, .707f, s); lo.lshelf(lf, lg, s); hi.hshelf(hf, hg, s); p1.peak(f1, q1, g1, s); p2.peak(f2, q2, g2, s);
        for (int i = 0; i < n; ++i) { if ((i & 31) == 0) { float b = band[i >> 5]; nHp.hp(20 * std::pow(35.f, b), .8f, s); nLp.lp(20000 * std::pow(.13f, b), .8f, s); nMid.peak(1400, 1.f, 8 * b, s); }
            float m = mix[i >> 5], l = L[i], r = R[i], a = l, c = r; for (int ch = 0; ch < 2; ++ch) { float& x = ch ? c : a; x = hp.tick(x, ch); x = lp.tick(x, ch); x = lo.tick(x, ch); x = hi.tick(x, ch); x = p1.tick(x, ch); x = p2.tick(x, ch); x = nHp.tick(x, ch); x = nLp.tick(x, ch); x = nMid.tick(x, ch); }
            L[i] = l + (a - l) * m; R[i] = r + (c - r) * m; } } };
enum DriveType { dAnalog, dTube, dFuzz, dHardClip, dFold, dRectify, dNumTypes };
inline const char* const driveNames[] = {"ANALOG","TUBE","FUZZ","HARD CLIP","FOLDBACK","RECTIFY"};
struct Drive { std::unique_ptr<juce::dsp::Oversampling<float>> os; Biquad preP, preL; Delay dl, dr; float dcx[2] {}, dcy[2] {}; double sr = 48000; int lat = 0; juce::AudioBuffer<float> tmp; Smooth pre[2], mixS;
    void prepare(double s, int block) { sr = s; os = std::make_unique<juce::dsp::Oversampling<float>>(2, 2, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true); os->initProcessing((size_t)block); os->reset(); lat = (int)std::lround(os->getLatencyInSamples()); dl.size(lat + 8); dr.size(lat + 8); tmp.setSize(2, block); preP.reset(); preL.reset(); pre[0].setTime(s * 4, 8); pre[1].setTime(s * 4, 8); mixS.setTime(s, 8); }
    void clearState() { preP.reset(); preL.reset(); dl.clear(); dr.clear(); dcx[0] = dcx[1] = dcy[0] = dcy[1] = 0; os->reset(); }
    static float shape(float x, int t, float c) { float k = 1 + c * 3; switch (t) {
        case dTube: { float y = x >= 0 ? std::tanh(x * k) : std::tanh(x * k * .6f) / .6f * .7f; return y; }
        case dFuzz: { float y = std::tanh(x * k * 4); return juce::jlimit(-.8f, .8f, y + .15f * y * y); }
        case dHardClip: return juce::jlimit(-1.f + c * .7f, 1.f - c * .7f, x);
        case dFold: { float y = x * (1 + c); for (int i = 0; i < 4 && std::abs(y) > 1; ++i) y = y > 0 ? 2 - y : -2 - y; return y; }
        case dRectify: return (1 - c) * x + c * (std::abs(x) * 2 - .6f);
        default: return std::tanh(x * k + .08f * c) ; } }
    void process(float* L, float* R, int n, const float* mix, const float* preDb, float postDb, int type, float curve, float pf, float pg, float pq, float pl) { float s = (float)sr; preP.peak(pf, pq, pg, s); preL.lp(pl, .8f, s);
        float* ch[2] = {tmp.getWritePointer(0), tmp.getWritePointer(1)}; for (int i = 0; i < n; ++i) { ch[0][i] = preL.tick(preP.tick(L[i], 0), 0); ch[1][i] = preL.tick(preP.tick(R[i], 1), 1); }
        juce::dsp::AudioBlock<float> b(ch, 2, (size_t)n); auto u = os->processSamplesUp(b); int un = (int)u.getNumSamples(); float pd = db2g(postDb);
        for (int c = 0; c < 2; ++c) { auto* d = u.getChannelPointer((size_t)c); for (int i = 0; i < un; ++i) { float g = db2g(pre[c].next(preDb[(i / 4) >> 5])); float y = shape(d[i] * g, type, curve) * pd; float o = y - dcx[c] + .9995f * dcy[c]; dcx[c] = y; dcy[c] = o; d[i] = o; } }
        os->processSamplesDown(b);
        for (int i = 0; i < n; ++i) { dl.push(L[i]); dr.push(R[i]); float m = mixS.next(mix[i >> 5]); L[i] = dl.at((float)lat) * (1 - m) + ch[0][i] * m; R[i] = dr.at((float)lat) * (1 - m) + ch[1][i] * m; } } };
struct Crush { float hL = 0, hR = 0, ph = 0; void process(float* L, float* R, int n, const float* mix, const float* amt) { for (int i = 0; i < n; ++i) { float a = amt[i >> 5], m = mix[i >> 5]; float bits = 16 - a * 13, step = std::pow(2.f, 1 - bits), hold = 1 + a * a * 40; ph += 1; if (ph >= hold) { ph -= hold; hL = std::round(L[i] / step) * step; hR = std::round(R[i] / step) * step; } L[i] += (hL - L[i]) * m; R[i] += (hR - R[i]) * m; } } };
enum ModType { mTremolo, mChorus, mPhaser, mFlanger, mVibrato };
struct Mod { Delay d[2]; float ph = 0, ap[2][4] {}, fb[2] {}; double sr = 48000; void clear() { d[0].clear(); d[1].clear(); std::fill(&ap[0][0], &ap[0][0] + 8, 0.f); fb[0] = fb[1] = 0; } void prepare(double s) { sr = s; d[0].size(int(s * .05)); d[1].size(int(s * .05)); }
    void process(float* L, float* R, int n, const float* mix, int t, float depth, float hz) { float inc = hz / (float)sr; for (int i = 0; i < n; ++i) { ph += inc; if (ph > 1) ph -= 1; float m = mix[i >> 5];
            for (int c = 0; c < 2; ++c) { float* X = c ? R : L; float x = X[i], lfo = std::sin(twoPi * (ph + (c ? .25f : 0))), y = x; d[c].push(x + (t == mFlanger ? fb[c] * .6f * depth : 0));
                switch (t) { case mTremolo: y = x * (1 - depth * (.5f + .5f * lfo)); break;
                    case mChorus: y = .7f * (x + d[c].at(float(sr * (.012 + .006 * depth * (1 + lfo))))); break;
                    case mFlanger: y = .7f * (x + (fb[c] = d[c].at(float(sr * (.0008 + .0035 * depth * (1 + lfo)))))); break;
                    case mVibrato: y = d[c].at(float(sr * (.002 + .002 * depth * (1 + lfo)))); break;
                    default: { float f = 400 + 1400 * (1 + lfo) * depth, tn = std::tan(MathConstants<float>::pi * f / (float)sr), a = (tn - 1) / (tn + 1), z = x + fb[c] * .5f * depth; for (auto& s : ap[c]) { float o = a * z + s; s = z - a * o; z = o; } fb[c] = z; y = .7f * (x + z); } }
                X[i] = x + (y - x) * m; } } } };
struct Gramophone { Delay d[2]; float ph = 0, ph2 = 0, tick = 0, tickV = 0, crk = 0; double sr = 48000; void clear() { d[0].clear(); d[1].clear(); crk = tick = 0; } juce::Random r { 9 }; void prepare(double s) { sr = s; d[0].size(int(s * .05)); d[1].size(int(s * .05)); }
    void process(float* L, float* R, int n, const float* mix, float wow, float curve, float ticks, float gainDb) { float g = db2g(gainDb); for (int i = 0; i < n; ++i) { ph += .55f / (float)sr; ph2 += 7.7f / (float)sr; if (ph > 1) ph -= 1; if (ph2 > 1) ph2 -= 1; float m = mix[i >> 5];
            float dly = float(sr) * (.006f + .005f * wow * (1 + std::sin(twoPi * ph)) + .001f * curve * (1 + std::sin(twoPi * ph2)));
            if (r.nextFloat() < ticks * ticks * 30 / (float)sr) tick = 1 + r.nextFloat() * .0f, tickV = (r.nextBool() ? 1 : -1) * (.2f + r.nextFloat() * .5f);
            float hole = 1 - juce::jlimit(0.f, .85f, tick); tick *= .985f; crk = crk * .6f + (r.nextFloat() * 2 - 1) * ticks * .01f * (r.nextFloat() < .02f ? 12 : 1);
            for (int c = 0; c < 2; ++c) { float* X = c ? R : L; float x = X[i]; d[c].push(x); float y = d[c].at(dly) * hole * g + tick * tickV * .3f + crk; X[i] = x + (y - x) * m; } } } };
struct Tuning { float ph = 0, tick = 0, f = 0; Biquad bp; double sr = 48000; juce::Random r { 5 }; void prepare(double s) { sr = s; bp.reset(); }
    // x -1..1 off-station amount, y 0..1 whistle pitch, mode 0 AM 1 FM
    void process(float* L, float* R, int n, const float* mix, float x, float y, int mode) { float off = std::abs(x), hz = 300 + 3200 * y * y; bp.lp(mode ? 12000.f : 4200.f, .7f, (float)sr); for (int i = 0; i < n; ++i) { ph += hz / (float)sr; if (ph > 1) ph -= 1; float m = mix[i >> 5], nz = (r.nextFloat() * 2 - 1);
            float gate = mode ? 1.f : 1.f - .3f * off * (1 + std::sin(twoPi * ph * .01f));
            float wh = std::sin(twoPi * ph) * off * (mode ? .03f : .12f), nn = nz * off * off * (mode ? .35f : .18f);
            for (int c = 0; c < 2; ++c) { float* X = c ? R : L; float s = bp.tick(X[i], c); s = std::tanh(s * (1 + 4 * off)) / (1 + off) * gate * (1 - off * .6f); X[i] = X[i] + ((s + wh + nn) - X[i]) * m; } } } };
struct Leslie { Biquad xl, xh; Delay d[2]; void clear() { d[0].clear(); d[1].clear(); xl.reset(); xh.reset(); } float hPh = 0, lPh = 0, hRate = .8f, lRate = .66f; double sr = 48000; void prepare(double s) { sr = s; d[0].size(int(s * .03)); d[1].size(int(s * .03)); xl.lp(800, .707f, (float)s); xh.hp(800, .707f, (float)s); }
    void process(float* L, float* R, int n, const float* mix, int mode, float lo, float hi, float lazy, float width) { float tH = mode == 0 ? 0.f : mode == 1 ? .4f + lo * .6f : 3 + hi * 5, tL = mode == 0 ? 0.f : mode == 1 ? .33f + lo * .5f : 2.8f + hi * 4; float k = 1 / float(sr * (.05 + lazy * 2.5));
        for (int i = 0; i < n; ++i) { hRate += (tH - hRate) * k; lRate += (tL - lRate) * k * .4f; hPh += hRate / (float)sr; lPh += lRate / (float)sr; if (hPh > 1) hPh -= 1; if (lPh > 1) lPh -= 1; float m = mix[i >> 5];
            for (int c = 0; c < 2; ++c) { float* X = c ? R : L; float x = X[i], lf = xl.tick(x, c), hf = xh.tick(x, c); float a = twoPi * (hPh + (c ? width * .25f : 0)); d[c].push(hf); float dop = d[c].at(float(sr) * (.0008f + .0006f * (1 + std::sin(a)))); float y = dop * (.7f + .3f * std::cos(a)) + lf * (.8f + .2f * std::sin(twoPi * (lPh + (c ? width * .25f : 0)))); X[i] = x + (y - x) * m; } } } };
struct StereoDelay { Delay d[2]; Biquad f; void clear() { d[0].clear(); d[1].clear(); f.reset(); } float lpz[2] {}; double sr = 48000; Smooth tL, tR; void prepare(double s) { sr = s; d[0].size(int(s * 2.2)); d[1].size(int(s * 2.2)); f.reset(); tL.setTime(s, 80); tR.setTime(s, 80); }
    void process(float* L, float* R, int n, const float* mix, float msL, float msR, float feed, float xf, float tone) { float s = (float)sr; if (tone >= 0) f.hp(20 + tone * tone * 2000, .7f, s); else f.lp(20000 * std::pow(.05f, -tone), .7f, s);
        tL.v = tL.v == 0 ? msL : tL.v; tR.v = tR.v == 0 ? msR : tR.v; for (int i = 0; i < n; ++i) { float m = mix[i >> 5], a = tL.next(msL), b = tR.next(msR), l = d[0].at(s * a * .001f), r = d[1].at(s * b * .001f), fl = f.tick(l, 0), fr = f.tick(r, 1);
            d[0].push(juce::jlimit(-2.f, 2.f, L[i] + (fl * (1 - xf) + fr * xf) * feed)); d[1].push(juce::jlimit(-2.f, 2.f, R[i] + (fr * (1 - xf) + fl * xf) * feed)); L[i] += l * m; R[i] += r * m; } } };
struct Limiter { float g = 1, env = 0; Delay d[2]; void clear() { d[0].clear(); d[1].clear(); g = env = 1; } int la = 0; double sr = 48000; void prepare(double s) { sr = s; la = int(s * .002); d[0].size(la + 4); d[1].size(la + 4); } int latency() const { return la; }
    void process(float* L, float* R, int n, bool on) { float rel = 1 - std::exp(-1 / float(sr * .12)); for (int i = 0; i < n; ++i) { float p = juce::jmax(std::abs(L[i]), std::abs(R[i])); d[0].push(L[i]); d[1].push(R[i]); float tg = (on && p > .89f) ? .89f / p : 1.f; env = tg < env ? tg : env + (1 - env) * rel; g += (env - g) * .05f; float o = on ? juce::jmin(g, env) : 1.f; L[i] = d[0].at((float)la) * o; R[i] = d[1].at((float)la) * o; if (on) { L[i] = juce::jlimit(-.98f, .98f, L[i]); R[i] = juce::jlimit(-.98f, .98f, R[i]); } } } };
inline float lfoWave(int w, float p, juce::Random& r, float& sh, float& last) { switch (w) { case 1: return 4 * std::abs(p - .5f) - 1; case 2: return 2 * p - 1; case 3: return p < .5f ? 1.f : -1.f; case 4: if (p < last) sh = r.nextFloat() * 2 - 1; last = p; return sh; default: return std::sin(twoPi * p); } }
}
