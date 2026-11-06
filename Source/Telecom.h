// Speakit telecom stage: real codec round trip at 8 kHz with fixed latency and line damage.
#pragma once
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <array>
extern "C" {
#include "gsm.h"
#include <speex/speex.h>
}
namespace sk {
enum Codec { cOff, cUlaw, cAlaw, cAdpcm, cGsm, cSpeex15, cSpeex8, cSpeex2, cLpcRobot, cLpcWhisper, cNumCodecs };
inline const char* const codecNames[] = {"OFF","G711 ULAW","G711 ALAW","ADPCM 32K","GSM 13K","SPEEX 15K","SPEEX 8K","SPEEX 2K","LPC ROBOT","LPC WHISPER"};

class Telecom {
public:
    static constexpr int frame = 160, speexExtra = 40;
    ~Telecom() { release(); }
    void prepare(double sampleRate) {
        release(); sr = sampleRate;
        taps = (int(sr * 0.004) | 1); fir.assign((size_t)taps, 0.f);
        const double fc = 3650.0 / sr; const int m = taps / 2; float sum = 0;
        for (int i = 0; i < taps; ++i) { double x = i - m;
            double s = x == 0 ? 2 * fc : std::sin(2 * juce::MathConstants<double>::pi * fc * x) / (juce::MathConstants<double>::pi * x);
            double w = 0.42 + 0.5 * std::cos(juce::MathConstants<double>::pi * x / (m + 1)) + 0.08 * std::cos(2 * juce::MathConstants<double>::pi * x / (m + 1));
            fir[(size_t)i] = float(s * w); sum += fir[(size_t)i]; }
        for (auto& f : fir) f /= sum;
        hDown.assign((size_t)taps, 0.f); hUp.assign((size_t)taps, 0.f);
        ratio = 8000.0 / sr;
        gsmH = gsm_create(); int one = 1; (void)one;
        speex_bits_init(&bits);
        for (int i = 0; i < 3; ++i) { enc[i] = speex_encoder_init(&speex_nb_mode); dec[i] = speex_decoder_init(&speex_nb_mode);
            int q = i == 0 ? 8 : i == 1 ? 3 : 0; speex_encoder_ctl(enc[i], SPEEX_SET_QUALITY, &q); int e = 1; speex_decoder_ctl(dec[i], SPEEX_SET_ENH, &e); }
        reset(); active = cUlaw;
        // measure the true path latency once with an impulse, so dry always aligns
        std::vector<float> a((size_t)(sr * 0.25), 0.f), b(a.size(), 0.f); a[32] = 1.f; float best = 0; int at = 0;
        for (size_t i = 0; i < a.size(); ++i) { b[i] = tick(a[i], 0.f); if (std::abs(b[i]) > best) { best = std::abs(b[i]); at = (int)i; } }
        lat = at - 32; dry.assign((size_t)lat + 8192, {0.f, 0.f}); dw = 0; reset(); active = cOff;
    }
    int latency() const { return lat; }
    void reset() {
        std::fill(hDown.begin(), hDown.end(), 0.f); std::fill(hUp.begin(), hUp.end(), 0.f); dpos = upos = 0;
        phD = phU = 0; prev = 0; nIn = 0; rp = wp = 0; fifo.fill(0.f);
        count = prefill; for (auto& h : hist) h = 0; for (auto& v : comp) v = 0; cw = 0;
        adS = 0; adI = 0; adS2 = 0; adI2 = 0; std::fill(std::begin(lpcMem), std::end(lpcMem), 0.f); std::fill(std::begin(past), std::end(past), 0.f); lpcG = 0;
        if (gsmH) { gsm_destroy(gsmH); gsmH = gsm_create(); }
        bad = false; fade = 0;
    }
    // codec, line 0..1, mix 0..1; stereo in place. Sums to mono when coded, like a real line.
    void process(float* L, float* R, int n, const int* codec, const float* line, const float* mix, juce::Random& rnd) {
        for (int i = 0; i < n; ++i) {
            int want = codec[i >> 5];
            if (want != active) { if (fade <= 0.001f) { active = want; pending = false; if (want != cOff) resetCodecOnly(); } else pending = true; }
            float tgt = (active == cOff || want != active) ? 0.f : mix[i >> 5];
            fade += (tgt - fade) * fadeC; lineNow = line[i >> 5];
            float m = 0.5f * (L[i] + R[i]);
            float w = active == cOff ? 0.f : tick(m, lineNow);
            float noise = active == cOff ? 0.f : (rnd.nextFloat() * 2 - 1) * lineNow * lineNow * 0.05f * (0.4f + 0.6f * burst);
            auto& d = dry[(size_t)dw]; size_t r = (size_t)((dw + (int)dry.size() - lat) % (int)dry.size());
            d = {L[i], R[i]}; float dl = dry[r][0], dr = dry[r][1]; dw = (dw + 1) % (int)dry.size();
            float s = (w + noise) * fade;
            L[i] = dl * (1 - fade) + s; R[i] = dr * (1 - fade) + s;
        }
    }
private:
    void release() { if (gsmH) { gsm_destroy(gsmH); gsmH = nullptr; speex_bits_destroy(&bits);
        for (int i = 0; i < 3; ++i) { speex_encoder_destroy(enc[i]); speex_decoder_destroy(dec[i]); } } }
    void resetCodecOnly() { adS = adI = adS2 = adI2 = 0; }
    static float filt(const std::vector<float>& h, std::vector<float>& s, int& pos, float x) {
        const int N = (int)h.size(); s[(size_t)pos] = x; float o = 0; int p = pos;
        for (int i = 0; i < N; ++i) { o += h[(size_t)i] * s[(size_t)p]; if (--p < 0) p = N - 1; }
        if (++pos >= N) pos = 0; return o;
    }
    float tick(float x, float line) {
        float f = filt(fir, hDown, dpos, x);
        phD += ratio;
        if (phD >= 1.0) { phD -= 1.0; float frac = float(phD / ratio); float y = f + (prev - f) * frac;
            inBuf[nIn++] = y; if (nIn == frame) { nIn = 0; runFrame(line); } }
        prev = f;
        phU += ratio;
        if (phU >= 1.0) { phU -= 1.0; hist[0] = hist[1]; hist[1] = hist[2]; hist[2] = hist[3];
            float v = 0; if (count > 0) { v = fifo[(size_t)rp]; rp = (rp + 1) % (int)fifo.size(); --count; } hist[3] = v; }
        float t = float(phU / ratio) * float(ratio); t = float(phU);
        float c0 = hist[1], c1 = 0.5f * (hist[2] - hist[0]), c2 = hist[0] - 2.5f * hist[1] + 2 * hist[2] - 0.5f * hist[3], c3 = 0.5f * (hist[3] - hist[0]) + 1.5f * (hist[1] - hist[2]);
        float o = ((c3 * t + c2) * t + c1) * t + c0;
        return filt(fir, hUp, upos, o) ;
    }
    void runFrame(float line) {
        float out[frame]; codecRound(inBuf, out, active == cOff ? cUlaw : active, line);
        // Speex is 40 samples late by design; everything else waits the same 40.
        const bool sp = active == cSpeex15 || active == cSpeex8 || active == cSpeex2;
        for (int i = 0; i < frame; ++i) { float v = out[i];
            if (!sp) { float d = comp[(size_t)cw]; comp[(size_t)cw] = v; cw = (cw + 1) % speexExtra; v = d; }
            fifo[(size_t)((rp + count) % (int)fifo.size())] = v; ++count; }
    }
    static int ulaw(int s) { const int B = 132; int sign = s < 0 ? 0x80 : 0; if (s < 0) s = -s; s = juce::jmin(s, 32635) + B; int e = 7; for (int m = 0x4000; (s & m) == 0 && e > 0; --e, m >>= 1) {} int man = (s >> (e + 3)) & 15; return ~(sign | (e << 4) | man) & 255; }
    static int ulawD(int u) { u = ~u & 255; int t = (((u & 15) << 3) + 132) << ((u & 0x70) >> 4); return (u & 0x80) ? 132 - t : t - 132; }
    static int alaw(int s) { int sign = s >= 0 ? 0x80 : 0; if (s < 0) s = -s - 1; s = juce::jmin(s, 32767); int e = 7; for (int m = 0x4000; (s & m) == 0 && e > 0; --e, m >>= 1) {} int man = e ? (s >> (e + 3)) & 15 : (s >> 4) & 15; return (sign | (e << 4) | man) ^ 0x55; }
    static int alawD(int a) { a ^= 0x55; int t = (a & 15) << 4; int e = (a & 0x70) >> 4; if (e == 0) t += 8; else { t += 0x108; t <<= (e - 1); } return (a & 0x80) ? t : -t; }
    static int adStep(int i) { static const auto tab = [] { std::array<int, 89> t{}; double v = 7; for (auto& x : t) { x = int(v + .5); v *= 1.1; } return t; }(); return tab[(size_t)i]; }
    static void adUpd(int& pred, int& idx, int code) { static const int it[8] = {-1,-1,-1,-1,2,4,6,8}; int st = adStep(idx); int d = st >> 3; if (code & 4) d += st; if (code & 2) d += st >> 1; if (code & 1) d += st >> 2;
        pred += (code & 8) ? -d : d; pred = juce::jlimit(-32768, 32767, pred); idx = juce::jlimit(0, 88, idx + it[code & 7]); }
    void adpcm(const short* in, short* o) { for (int i = 0; i < frame; ++i) { int st = adStep(adI), d = in[i] - adS, c = 0; if (d < 0) { c = 8; d = -d; } if (d >= st) { c |= 4; d -= st; } st >>= 1; if (d >= st) { c |= 2; d -= st; } st >>= 1; if (d >= st) c |= 1;
            adUpd(adS, adI, c); adUpd(adS2, adI2, c); o[i] = (short)adS2; } }
    void lpc(const float* in, float* o, bool whisper) {
        const int N = 240, P = 10; float w[N]; std::copy(past, past + 80, w); std::copy(in, in + frame, w + 80); std::copy(in + 80, in + 160, past);
        double r[P + 1] {}; float win[N]; for (int i = 0; i < N; ++i) win[i] = w[i] * (0.54f - 0.46f * std::cos(juce::MathConstants<float>::twoPi * i / (N - 1)));
        for (int k = 0; k <= P; ++k) for (int i = k; i < N; ++i) r[k] += win[i] * win[i - k];
        double a[P + 1] {}; a[0] = 1; double e = r[0] * 1.0001 + 1e-9;
        for (int i = 1; i <= P; ++i) { double acc = r[i]; for (int j = 1; j < i; ++j) acc += a[j] * r[i - j]; double k = -acc / e; double t[P + 1]; std::copy(a, a + P + 1, t);
            for (int j = 1; j < i; ++j) a[j] = t[j] + k * t[i - j]; a[i] = k; e *= (1 - k * k); }
        int period = 80; if (!whisper) { double best = 0; for (int T = 32; T < 160; ++T) { double c = 0; for (int i = T; i < N; ++i) c += w[i] * w[i - T]; if (c > best) { best = c; period = T; } }
            period = 64; }
        float g = (float)std::sqrt(juce::jmax(e, 0.0) / N) * (whisper ? 5.5f : 22.f); float gs = lpcG, dg = (g - gs) / frame;
        for (int i = 0; i < frame; ++i) { gs += dg; float ex;
            if (whisper) ex = (noiseR.nextFloat() * 2 - 1); else { ex = (pulse == 0) ? 1.f : -0.02f; if (++pulse >= period) pulse = 0; }
            float y = ex * gs; for (int j = 1; j <= P; ++j) y -= (float)a[j] * lpcMem[j - 1]; for (int j = P - 1; j > 0; --j) lpcMem[j] = lpcMem[j - 1]; lpcMem[0] = y;
            o[i] = juce::jlimit(-1.f, 1.f, y); }
        lpcG = g;
    }
    void codecRound(const float* in, float* out, int c, float line) {
        short s[frame], o[frame]; for (int i = 0; i < frame; ++i) s[i] = (short)juce::jlimit(-32768, 32767, int(in[i] * 32767.f));
        // bursty loss: bad frames arrive in runs, like a failing cell link
        const float p = line * line * 0.4f; bad = bad ? noiseR.nextFloat() < 0.55f : noiseR.nextFloat() < p; burst = bad ? 1.f : burst * 0.8f;
        switch (c) {
            case cUlaw: for (int i = 0; i < frame; ++i) o[i] = (short)ulawD(ulaw(s[i])); break;
            case cAlaw: for (int i = 0; i < frame; ++i) o[i] = (short)alawD(alaw(s[i])); break;
            case cAdpcm: adpcm(s, o); break;
            case cGsm: { gsm_frame f; gsm_encode(gsmH, s, f); if (bad) { for (int i = 8; i < 33; ++i) f[i] = (gsm_byte)noiseR.nextInt(256); } gsm_decode(gsmH, f, o); break; }
            case cSpeex15: case cSpeex8: case cSpeex2: { int k = c - cSpeex15; float x[frame], y[frame]; for (int i = 0; i < frame; ++i) x[i] = s[i]; speex_bits_reset(&bits); speex_encode(enc[k], x, &bits);
                speex_bits_rewind(&bits); speex_decode(dec[k], bad ? nullptr : &bits, y); for (int i = 0; i < frame; ++i) o[i] = (short)juce::jlimit(-32768.f, 32767.f, y[i]); break; }
            default: { float y[frame]; lpc(in, y, c == cLpcWhisper); for (int i = 0; i < frame; ++i) out[i] = bad ? 0.f : y[i]; return; }
        }
        for (int i = 0; i < frame; ++i) out[i] = o[i] / 32768.f;
        if (bad && c != cGsm && c < cSpeex15) for (int i = 0; i < frame; ++i) out[i] = 0;
    }
    double sr = 48000, ratio = 1, phD = 0, phU = 0; int taps = 0, dpos = 0, upos = 0, lat = 0, dw = 0, active = cOff, cw = 0; bool pending = false;
    std::vector<float> fir, hDown, hUp; std::vector<std::array<float, 2>> dry;
    float prev = 0, inBuf[frame] {}, hist[4] {}, fade = 0, lineNow = 0, burst = 0, past[80] {}, lpcMem[10] {}, lpcG = 0; int nIn = 0, pulse = 0;
    static constexpr int prefill = frame + 16; static constexpr float fadeC = 0.004f;
    std::array<float, 1024> fifo {}; int rp = 0, wp = 0, count = prefill; std::array<float, speexExtra> comp {};
    int adS = 0, adI = 0, adS2 = 0, adI2 = 0; bool bad = false; gsm gsmH = nullptr; SpeexBits bits {}; void* enc[3] {}; void* dec[3] {}; juce::Random noiseR { 1234 };
};
}
