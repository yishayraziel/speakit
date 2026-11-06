#!/usr/bin/env python3
"""Builds Speakit's IR bank: captured IRs from the MIT-licensed IR-Library by Conner,
plus original modeled IRs generated here from target curves. Writes FLACs and Source/IrManifest.h."""
import numpy as np, soundfile as sf, os, sys
from scipy.signal import resample_poly
from fractions import Fraction
SRC=sys.argv[1]; OUT='Resources/ir'; SR=48000
os.makedirs(OUT,exist_ok=True)
for f in os.listdir(OUT): os.remove(os.path.join(OUT,f))
# kind, group, display name, icon, max seconds, source
C=[
('spk','Phones','Smartphone Speaker','mobile',.05,'Speakers/Phones/Smartphone 2020 Speaker Dispersion.wav'),
('spk','Computers','Laptop Speakers','laptop',.4,'Speakers/Computers/Laptops/Laptop 2011 Light Speakers.wav'),
('spk','Computers','Laptop From Outside','laptop',.4,'Speakers/Computers/Laptops/Laptop 2011 Light Outside.wav'),
('spk','Toys','Distant Toy Keyboard','toy',.4,'Speakers/Toys and Novelty/Distant Tiny Keyboard Speaker.wav'),
('spk','Toys','Tiny Amp Dark','amp',.35,'Speakers/Amplifiers/Tiny T-Shirt Amplifier/Tiny T-Shirt Amplifier Tone 0.wav'),
('spk','Toys','Tiny Amp Mid','amp',.35,'Speakers/Amplifiers/Tiny T-Shirt Amplifier/Tiny T-Shirt Amplifier Tone 5.wav'),
('spk','Toys','Tiny Amp Bright','amp',.35,'Speakers/Amplifiers/Tiny T-Shirt Amplifier/Tiny T-Shirt Amplifier Tone 10.wav'),
('spk','PA and Monitors','Floor Wedge Monitor','amp',.3,'Speakers/Monitors/12in 2-Way Wedge Floor Monitor.wav'),
('spk','Cars','Car Door Speakers','car',.05,'Enclosed Spaces/Automobiles/2002 Sedan/2002 Sedan Speakers Front.wav'),
('spk','Cars','Car Rear Speakers','car',.05,'Enclosed Spaces/Automobiles/2002 Sedan/2002 Sedan Speakers Rear.wav'),
('spk','Tape and Vintage','Cassette Recorder Speaker','tape',.35,'Hardware/Tape/Portable Cassette Recorder/Portable Cassette Recorder Click Mic In Speaker Out.wav'),
('spk','Tape and Vintage','Cabinet Gramophone','tape',.3,'Hardware/Old Cabinet Gramophone/Old Cabinet Gramophone.wav'),
('spk','Tape and Vintage','Noise Machine','toy',.4,'Hardware/Noise Machine/Noise Machine Front.wav'),
('cov','Household','Clothes Dryer','box',.6,'Enclosed Spaces/Old Home Appliances/Old Home Dryer.wav'),
('cov','Household','Oven','box',.6,'Enclosed Spaces/Old Home Appliances/Old Home Oven.wav'),
('cov','Household','Washing Machine','box',.6,'Enclosed Spaces/Old Home Appliances/Old Home Washing Machine.wav'),
('cov','Household','Towel Closet','box',.5,'Rooms/Residential/Old Home/Old Home Guest Bath Towel Closet.wav'),
('cov','Household','Bathtub','box',.6,'Rooms/Residential/Old Home/Old Home Guest Bath Tub Resonator.wav'),
('cov','Household','Air Vent','box',.6,'Rooms/Residential/Old Home/Old Home AC Intake Vent.wav'),
('cov','Containers','Plastic Storage Bin','box',.5,'Objects/Containers/Storage Bins/45L Plastic Storage Bin.wav'),
('cov','Containers','Metal Keg','box',.6,'Objects/Containers/Nitro Cold Brew Keg/Nitro Cold Brew Keg Inside.wav'),
('cov','Containers','Soda Can','box',.5,'Objects/Containers/Soda Can/Soda Can (12 fl oz).wav'),
('cov','Containers','Metal Bowl','box',.5,'Objects/Containers/Metal Bowls/Small Metal Dog Bowl Mono.wav'),
('cov','Containers','Snack Tube','box',.5,'Objects/Containers/Miscellaneous/Snack Chip Cylinder Small.wav'),
('cov','Containers','Clay Jar','box',.5,'Objects/Containers/Historical/Ostia Antica Small Dolia Clap.wav'),
('cov','Tubes','Corrugated Pipe','box',.5,'Objects/Pipes and Tubes/Corrugated Pipe 1.wav'),
('cov','Tubes','Storm Drain','box',.7,'Enclosed Spaces/Miscellaneous/Storm Drain Bang Snap.wav'),
('cov','Tubes','Boat Hull','box',.7,'Enclosed Spaces/Watercraft/Catamaran Hull.wav'),
('mic','Captured','Cassette Recorder Mic','mic',.25,'Hardware/Tape/Portable Cassette Recorder/Portable Cassette Recorder Click Mic In Line Out.wav'),
('mic','Captured','Keyboard Mic Input','mic',.1,'Hardware/2004 Digital Piano/2004 Digital Piano Mic Loopback Impulse.wav'),
('mic','Captured','Contact Mic','mic',.25,'Speakers/Miscellaneous/Contact Mic on Contact Transducer.wav'),
('rom','Home','Living Room','room',1.6,'Rooms/Residential/Old Home/Old Home Living Room.wav'),
('rom','Home','Bedroom','room',1.,'Rooms/Residential/Colonial House/Colonial Bedroom (No Treatment).wav'),
('rom','Home','Home Office','room',1.,'Rooms/Residential/College House/College House Office.wav'),
('rom','Home','Tiled Bathroom','room',1.4,'Rooms/Commercial/Hotels/Bathrooms/Reflective Half Bathroom.wav'),
('rom','Home','Far Down The Hall','room',1.6,'Rooms/Residential/Old Home/Old Home Hallway Far.wav'),
('rom','Public','Coffee Shop','room',1.8,'Rooms/Commercial/Shops/Gold Coffee Shop/Gold Coffee Shop Lobby Corner.wav'),
('rom','Public','Hotel Room','room',1.,'Rooms/Commercial/Hotels/Guest Rooms/Hotel Room 1 XY (Balloon).wav'),
('rom','Public','Freight Elevator','room',1.4,'Rooms/Public/Convention Centers/Orange County Convention Center/OCCC Freight Elevator.wav'),
('rom','Public','Brick Stairwell','room',2.2,'Rooms/Public/Colleges & Universities/University of Central Florida/MSB Brick Stairwell Downstairs.wav'),
('rom','Public','Parking Garage','room',3.,'Rooms/Commercial/Parking Garages/Firenze Parking Garage.wav'),
('rom','Large','Ballroom','room',2.6,'Rooms/Commercial/Hotels/Ballrooms/Palm Ballroom.wav'),
('rom','Large','Sports Arena','room',3.,'Rooms/Public/Arenas/Giant Center/Giant Center Temporary Flown PA Upper Bleachers (High Gain).wav'),
('rom','Large','Exhibit Hall','room',3.,'Rooms/Public/Convention Centers/Phoenix Convention Center/PCC North 300 Level Exhibit Hall Combined Balloon Far.wav'),
('rom','Large','Small Church','room',3.,"Rooms/Historical/Religious/Drumheller's Little Church/Drumheller's Little Church.wav"),
('rom','Large','Concrete Bunker','room',2.5,'Rooms/Historical/Shelters/Menorcan Spanish Civil War Bunkers/Bunker 1 Rear Center.wav'),
]
# original modeled curves: (Hz, dB) points plus (Hz, dB, Q) resonances
M=[
('spk','Phones','Desk Phone Earpiece','phone',[(200,-40),(300,-14),(450,-3),(3200,-1),(3700,-18),(5000,-50)],[(1150,6,3),(2500,7,4)]),
('spk','Phones','Payphone Handset','phone',[(250,-40),(380,-12),(600,-4),(2800,-3),(3300,-20),(4500,-55)],[(900,5,2.5),(2100,9,5)]),
('spk','Radio','Transistor Radio','radio',[(150,-40),(320,-12),(600,-2),(3500,-2),(5000,-22),(8000,-55)],[(850,7,2),(2400,4,3)]),
('spk','Radio','Walkie Talkie','walkie',[(250,-45),(500,-14),(800,-3),(2600,-2),(3200,-22),(5000,-60)],[(1600,9,3.5)]),
('spk','Radio','Aircraft Cabin PA','walkie',[(300,-45),(600,-14),(1000,-3),(3500,-3),(4500,-22),(7000,-55)],[(1800,6,2.5)]),
('spk','PA and Monitors','Megaphone Horn','horn',[(250,-50),(480,-16),(750,-2),(3200,0),(4200,-16),(7000,-50)],[(1100,8,4),(2200,10,5)]),
('spk','PA and Monitors','Station Tannoy','horn',[(180,-45),(350,-14),(700,-3),(4000,-2),(5500,-24),(9000,-55)],[(1400,6,3),(3000,5,3)]),
('spk','PA and Monitors','Office Intercom','phone',[(220,-40),(420,-13),(700,-3),(3000,-3),(3800,-24),(6000,-55)],[(1300,5,2.5)]),
('spk','Television','Old CRT Television','radio',[(90,-40),(180,-12),(350,-3),(6000,-3),(8500,-20),(13000,-50)],[(700,4,2),(2800,5,3)]),
('spk','Television','Flat TV Speakers','laptop',[(120,-40),(240,-14),(500,-4),(10000,-3),(14000,-16)],[(1000,-4,1.5),(3500,6,2)]),
('mic','Modeled','Ribbon Style','mic',[(40,4),(120,3),(1000,0),(6000,-3),(12000,-12),(18000,-24)],[]),
('mic','Modeled','Handheld Dynamic','mic',[(60,-14),(150,-2),(1000,0),(8000,1),(12000,-8),(16000,-22)],[(4500,6,1.5)]),
('mic','Modeled','Lavalier','mic',[(70,-12),(200,-3),(1000,0),(14000,1),(18000,-6)],[(7500,5,1.2)]),
('mic','Modeled','Boom Headset','mic',[(120,-22),(300,-5),(1000,0),(6000,1),(9000,-14),(12000,-40)],[(3000,5,1.5)]),
('mic','Modeled','Carbon Telephone Mic','phone',[(200,-36),(400,-10),(800,-2),(3000,-2),(3800,-20),(6000,-50)],[(1500,7,2.5)]),
('mic','Modeled','Gooseneck Paging Mic','mic',[(150,-24),(350,-6),(1000,0),(5000,1),(8000,-14),(12000,-36)],[(2600,6,2)]),
]
def load(p):
    x,sr=sf.read(os.path.join(SRC,p),always_2d=True,dtype='float64')
    if sr!=SR:
        r=Fraction(SR,sr); x=resample_poly(x,r.numerator,r.denominator,axis=0)
    if x.shape[1]>2: x=x[:,:2]
    return x
def weight(n):
    f=np.fft.rfftfreq(n,1/SR); return 1/(1+(150/np.maximum(f,1))**4)/(1+(f/6000)**4)
def norm(x,kind):
    # equal perceived level on speech-band noise, so swapping IRs does not jump in level
    n=1<<18; X=np.fft.rfft(x.mean(1),n); e=np.sqrt(np.mean(np.abs(X)**2*weight(n))/np.mean(weight(n)))
    return x/max(e,1e-9)
def trim(x,maxs):
    m=np.abs(x).max(1); pk=m.max(); on=np.argmax(m>pk*.05); on=max(0,on-24); x=x[on:]
    x=x[:int(maxs*SR)]; f=max(16,len(x)//4); x=x.copy(); x[-f:]*=np.cos(np.linspace(0,np.pi/2,f))[:,None]**2
    return x
def model(pts,res,n=4096):
    f=np.fft.rfftfreq(n,1/SR); lf=np.log(np.maximum(f,1)); P=np.array(pts,float)
    db=np.interp(lf,np.log(P[:,0]),P[:,1])
    for r in res:
        w=np.log2(np.maximum(f,1)/r[0])*r[2]; db+=r[1]/(1+w*w)
    # real cepstrum minimum phase: an original filter, no measured data involved
    lm=np.log(10**(db/20)+1e-7); c=np.fft.irfft(lm,n); w=np.zeros(n); w[0]=1; w[1:n//2]=2; w[n//2]=1
    h=np.fft.irfft(np.exp(np.fft.rfft(c*w)),n)[:2048]; h*=np.cos(np.linspace(0,np.pi/2,2048))**2
    return h[:,None]
rows=[]; cnt={}
for kind,g,name,icon,maxs,src in C:
    x=norm(trim(load(src),maxs),kind); rows.append((kind,g,name,icon,x,'captured'))
for kind,g,name,icon,p,r in M:
    rows.append((kind,g,name,icon,norm(model(p,r),kind),'modeled'))
order={'spk':0,'cov':1,'mic':2,'rom':3}; rows.sort(key=lambda r:(order[r[0]],r[1])); tot=0
L=['// Generated by tools/prepare_irs.py - do not edit','#pragma once','struct IrInfo{const char* group;const char* name;const char* icon;const char* res;bool modeled;};']
for k in order:
    L.append(f'static const IrInfo ir_{k}[]={{')
    for kind,g,name,icon,x,o in rows:
        if kind!=k: continue
        i=cnt.get(k,0); cnt[k]=i+1; fn=f'{k}_{i:02d}.flac'
        pk=np.abs(x).max(); s=.9/pk; sf.write(f'{OUT}/{fn}',x*s,SR,subtype='PCM_24',format='FLAC'); tot+=os.path.getsize(f'{OUT}/{fn}')
        L.append(f'  {{"{g}","{name}","{icon}","{k}_{i:02d}_flac",{str(o=="modeled").lower()}}}, // gain {1/s:.5f}')
        L.append(f'  ')
    L.append('};'); L.append(f'static const int ir_{k}_count={cnt[k]};')
# stored peak-normalised; gain table restores the level
for k in order:
    gs=[]
    for kind,g,name,icon,x,o in rows:
        if kind==k: gs.append(f'{np.abs(x).max()/.9:.6f}f')
    L.append(f'static const float ir_{k}_gain[]={{{",".join(gs)}}};')
open('Source/IrManifest.h','w').write('\n'.join(l for l in L if l.strip())+'\n')
print(cnt,'bytes',tot)
