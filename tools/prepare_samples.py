#!/usr/bin/env python3
"""Sample Bay content: CC0 recordings (noixer, AtomCut packs) plus original synthesized line sounds."""
import numpy as np, soundfile as sf, subprocess, os, json, glob, tempfile
SR=48000; A='/home/claude/untrusted/sfx/atomcut/packs'; N='/home/claude/untrusted/sfx/noixer/assets'; OUT='Resources/bay'
for f in os.listdir(OUT): os.remove(f'{OUT}/{f}')
tmp=tempfile.mkdtemp()
def dec(p):
    o=tmp+'/d.wav'; subprocess.run(['ffmpeg','-v','error','-y','-i',p,'-ac','2','-ar',str(SR),o],check=True); return sf.read(o,dtype='float64')[0]
def noix(pref): return glob.glob(f'{N}/{pref}*')[0]
def pk(pack,name,i=0):
    d=json.load(open(f'{A}/{pack}/pack.json')); fs=[f for f in d['files'] if f['name']==name]
    return f'{A}/{pack}/audio/'+os.path.basename(fs[i]['file'])
def loopify(x,secs,start=None):
    n=int(secs*SR); fade=int(.5*SR)
    if len(x)<n+fade: return None
    s=start if start is not None else (len(x)-n-fade)//3
    y=x[s:s+n+fade].copy(); r=np.linspace(0,1,fade)[:,None]
    y[:fade]=y[:fade]*np.sqrt(r)+y[n:n+fade]*np.sqrt(1-r)
    return y[:n]
def level(x,db=-20):
    r=np.sqrt(np.mean(x**2)); x=x*10**(db/20)/max(r,1e-9); p=np.abs(x).max()
    return x*(.95/p if p>.95 else 1)
t=lambda s:np.arange(int(s*SR))/SR
rng=np.random.default_rng(7)
def st(m): return np.stack([m,m],1)
def lp(x,a):
    y=np.zeros_like(x);v=0
    for i in range(len(x)): v+=a*(x[i]-v); y[i]=v
    return y
def dial(): return st(.3*(np.sin(2*np.pi*350*t(10))+np.sin(2*np.pi*440*t(10))))
def cad(f,on,off,total):
    p=t(total); g=((p%(on+off))<on).astype(float); return st((sum(np.sin(2*np.pi*q*p) for q in f))*g*.3)
def dtmf():
    pairs=[(697,1209),(852,1336),(941,1477),(770,1209),(697,1336),(852,1209),(941,1336),(770,1477)]; o=[]
    for a,b in pairs:
        p=t(.12); o+=[.3*(np.sin(2*np.pi*a*p)+np.sin(2*np.pi*b*p)),np.zeros(int(.08*SR))]
    return st(np.concatenate(o))
def modem():
    o=[.4*np.sin(2*np.pi*2100*t(1.2))]; p=t(.5); o.append(.3*(np.sin(2*np.pi*1200*p)+np.sin(2*np.pi*2400*p)))
    b=rng.integers(0,2,4000); f=np.repeat(1200+b*1000,12)[:SR*3]; o.append(.4*np.sin(2*np.pi*np.cumsum(f)/SR)); o.append(.25*rng.standard_normal(SR*2)*np.sin(2*np.pi*1800*t(2)))
    return st(np.concatenate(o))
def hum(f): return st(sum(.3/k*np.sin(2*np.pi*f*k*t(10)) for k in (1,2,3,5)))
def hiss(): return np.stack([rng.standard_normal(SR*10),rng.standard_normal(SR*10)],1)*.05
def crackle():
    x=rng.standard_normal(SR*10)*.005; i=rng.integers(0,SR*10,420); x[i]+=rng.standard_normal(420)*.6
    return st(x)
def static():
    g=(rng.random(SR*10//480+1)>.5).repeat(480)[:SR*10]; n=rng.standard_normal(SR*10)
    return st(n*(.2+.8*lp(g.astype(float),.002))*.12)
def tuning():
    p=t(10); f=900+700*np.sin(2*np.pi*.17*p)+300*np.sin(2*np.pi*.61*p); return st(.15*np.sin(2*np.pi*np.cumsum(f)/SR)+.08*rng.standard_normal(len(p)))
def morse():
    o=[];
    for d in rng.integers(1,4,70):
        o+=[.3*np.sin(2*np.pi*750*t(.07*d)),np.zeros(int(.07*SR))]
    return st(np.concatenate(o))
def click_hangup():
    x=np.zeros(SR*2); x[int(.2*SR):int(.2*SR)+200]=rng.standard_normal(200)*.8; x[int(.32*SR):int(.32*SR)+120]=rng.standard_normal(120)*.5; return st(x)
# (bank, name, kind, source, loop, seconds or None)
K='opengameart-30-cc0-sfx-loops'; S='opengameart-swamp-environment-audio'; D='opengameart-door-open-door-close-set'
B=[('STREET',[('City Street',noix('448378'),20),('Bar Crowd',noix('394290'),20),('Subway Station',noix('60956'),20),('Train Interior',noix('133099'),20),('Train Passing',noix('260105'),20),('Church Bells',noix('501115'),20),('Rain',noix('156049'),20),('Thunderstorm',noix('19283'),20),('Waves',noix('450755'),20),('Forest',noix('517042'),20),('Fish Tank',noix('197804'),16),('Film Static',noix('324404'),14)]),
('INDOORS',[('Typing',noix('399603'),16),('Clothes Dryer',noix('234458'),14),('Machine Room',pk(K,'Machine 1'),None),('Air Pump',pk(K,'Pump 1'),None),('Water Pipe',pk(K,'Water Flowing'),None),('Boiling',pk(K,'Water Boiling'),None),('Door Close',pk(D,'Qubodup Door Close01'),None),('Door Open',pk(D,'Qubodup Door Open01'),None),('Crickets',pk(S,'Cricket 1'),None),('Cicadas',pk(S,'Cicada 1'),None),('Night Air',pk(S,'Atmosphere 1'),None),('Rolling',pk(K,'Rolling'),None)]),
('LINE',[('Dial Tone',dial,'L'),('Ringback',lambda:cad((440,480),2,4,12),'L'),('Busy Tone',lambda:cad((480,620),.5,.5,6),'L'),('Keypad Dial',dtmf,'O'),('Modem',modem,'O'),('Mains Hum 50',lambda:hum(50),'L'),('Mains Hum 60',lambda:hum(60),'L'),('Line Hiss',hiss,'L'),('Vinyl Crackle',crackle,'L'),('Radio Static',static,'L'),('Dial Sweep',tuning,'L'),('Morse Code',morse,'L')]),
('MACHINES',[('Alarm 1',pk(K,'Alarm 1'),None),('Alarm 2',pk(K,'Alarm 2'),None),('Ambient 1',pk(K,'Ambient 1'),None),('Ambient 2',pk(K,'Ambient 2'),None),('Machine 2',pk(K,'Machine 2'),None),('Machine 3',pk(K,'Machine 3'),None),('Machine 4',pk(K,'Machine 4'),None),('Noise 1',pk(K,'Noise 1'),None),('Noise 2',pk(K,'Noise 2'),None),('Saw',pk(K,'Saw'),None),('Weird 1',pk(K,'Weird 1'),None),('Weird 2',pk(K,'Weird 2'),None)]),
]
J=json.load(open(f'{A}/kenney-music-jingles/pack.json')); jf=J['files']; picks=[jf[i] for i in np.linspace(0,len(jf)-1,10).astype(int)]
B.append(('MUSIC',[(f'Jingle {i+1}',f'{A}/kenney-music-jingles/audio/'+os.path.basename(f['file']),None) for i,f in enumerate(picks)]+[('Choir Pad',noix('530715'),20),('Ethereal Voices',noix('273643'),20)]))
L=['// Generated by tools/prepare_samples.py - do not edit','#pragma once','struct BayInfo{const char* name;const char* res;bool loop;};','static const char* const bayBanks[5]={'+','.join(f'"{b[0]}"' for b in B)+'};','static const BayInfo bayTable[5][12]={']; tot=0
for bi,(bn,items) in enumerate(B):
    assert len(items)==12,bn; L.append(' {')
    for si,it in enumerate(items):
        name,src,mode=it; loop=True
        if callable(src):
            x=src(); loop=(mode=='L')
        else:
            x=dec(src); dur=len(x)/SR
            if mode: y=loopify(x,mode); x=y if y is not None else x
            else:
                loop=dur>2.5 and name not in('Door Close','Door Open')
                if loop and dur>16: y=loopify(x,16); x=y if y is not None else x
        x=level(x,-20 if loop else -14); fn=f'bay_{bi}_{si:02d}'
        sf.write(tmp+'/o.wav',x,SR,subtype='PCM_16')
        subprocess.run(['ffmpeg','-v','error','-y','-i',tmp+'/o.wav','-c:a','libvorbis','-q:a','3',f'{OUT}/{fn}.ogg'],check=True)
        tot+=os.path.getsize(f'{OUT}/{fn}.ogg'); L.append(f'  {{"{name}","{fn}_ogg",{str(loop).lower()}}},')
    L.append(' },')
L.append('};'); open('Source/BayManifest.h','w').write('\n'.join(L)+'\n'); print('MB',tot/1e6)
