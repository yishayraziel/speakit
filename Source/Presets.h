#pragma once
// category | name | semicolon list of id=value. Text values match IR names and choice labels. bay=BANK/Name@dB
// Append new presets at the end only: sessions store the preset index. Menus and arrows walk them
// grouped by category (presetOrder), in the order categories first appear.
static const struct { const char* cat; const char* name; const char* s; } factory[] = {
{"PHONES","DESK PHONE - BAD LINE","spk=Desk Phone Earpiece;codec=GSM 13K;line=0.45;compOn=1;comp=0.45;eqOn=1;eq=0.3;bay=LINE/Line Hiss@-30"},
{"PHONES","DESK PHONE - CLEAN","spk=Desk Phone Earpiece;codec=G711 ULAW;line=0;eqOn=1;eq=0.2"},
{"PHONES","CALLER ON HOLD","makeup=-5;spk=Payphone Handset;codec=G711 ALAW;line=0.1;compOn=1;comp=0.5;driveOn=1;drive=8;driveType=TUBE"},
{"PHONES","PAYPHONE ON THE STREET","makeup=2;spk=Payphone Handset;codec=ADPCM 32K;line=0.2;roomOn=1;roomIdx=Coffee Shop;room=0.12;bay=STREET/City Street@-22"},
{"PHONES","MOBILE - DRIVING AWAY","makeup=-2;spk=Smartphone Speaker;codec=GSM 13K;line=0.7;compOn=1;comp=0.6;bay=STREET/City Street@-30"},
{"PHONES","VOIP - THIN AND WATERY","makeup=12;spk=Laptop Speakers;codec=SPEEX 2K;line=0.25"},
{"PHONES","VIDEO CALL","makeup=9;spk=Laptop Speakers;codec=SPEEX 15K;line=0.05;roomOn=1;roomIdx=Home Office;room=0.15"},
{"RADIO & COMMS","WALKIE TALKIE","spk=Walkie Talkie;codec=SPEEX 8K;line=0.2;compOn=1;comp=0.85;driveOn=1;drive=18;driveType=HARD CLIP;gateOn=1;gate=-42;bay=LINE/Radio Static@-28"},
{"PA & ANNOUNCEMENTS","COCKPIT ANNOUNCEMENT","makeup=2;spk=Aircraft Cabin PA;compOn=1;comp=0.7;driveOn=1;drive=10;roomOn=1;roomIdx=Freight Elevator;room=0.15;bay=LINE/Line Hiss@-24"},
{"PA & ANNOUNCEMENTS","MEGAPHONE IN THE STREET","makeup=-5;spk=Megaphone Horn;compOn=1;comp=0.9;driveOn=1;drive=24;driveType=FUZZ;roomOn=1;roomIdx=Parking Garage;room=0.28;bay=STREET/City Street@-20"},
{"PA & ANNOUNCEMENTS","STADIUM PA","makeup=8;spk=Station Tannoy;compOn=1;comp=0.7;roomOn=1;roomIdx=Sports Arena;room=0.5;delayOn=1;delayL=420;delayR=470;delayFeed=0.2;delayMix=0.15;bay=STREET/Bar Crowd@-26"},
{"PA & ANNOUNCEMENTS","TRAIN STATION ANNOUNCER","makeup=5;spk=Station Tannoy;compOn=1;comp=0.6;roomOn=1;roomIdx=Exhibit Hall;room=0.45;bay=STREET/Subway Station@-22"},
{"RADIO & COMMS","TRANSISTOR RADIO","makeup=-4;spk=Transistor Radio;eqOn=1;eq=0.2;tuneOn=1;tuneX=0.15;tuneY=0.4;compOn=1;comp=0.5"},
{"RADIO & COMMS","RADIO OFF STATION","makeup=-2;spk=Transistor Radio;tuneOn=1;tuneX=0.75;tuneY=0.55;bay=LINE/Dial Sweep@-30"},
{"PLACES","NEIGHBOR'S PARTY","spk=Floor Wedge Monitor;coverOn=1;coverIdx=Towel Closet;cover=0.7;eqOn=1;eq=0.45;roomOn=1;roomIdx=Far Down The Hall;room=0.45"},
{"PLACES","TV IN THE NEXT ROOM","makeup=-2;spk=Old CRT Television;roomOn=1;roomIdx=Far Down The Hall;room=0.5;eqOn=1;eq=0.25"},
{"PLACES","CAR STEREO FROM OUTSIDE","makeup=5;spk=Car Door Speakers;coverOn=1;coverIdx=Boat Hull;cover=0.5;eqOn=1;eq=0.35;bay=STREET/City Street@-24"},
{"VINTAGE","OLD GRAMOPHONE","makeup=-6;spk=Cabinet Gramophone;gramOn=1;gramWow=0.3;gramCurve=0.3;gramTicks=0.45;eqOn=1;eq=0.25"},
{"VINTAGE","CASSETTE DICTATION","makeup=-3;spk=Cassette Recorder Speaker;micOn=1;micIdx=Cassette Recorder Mic;mic=0.8;gramOn=1;gramWow=0.15;gramTicks=0;compOn=1;comp=0.45"},
{"FX","INSIDE A BIN","makeup=3;spk=Tiny Amp Mid;coverOn=1;coverIdx=Plastic Storage Bin;cover=0.9"},
{"FX","SPINNING CABINET","makeup=-4;spk=Floor Wedge Monitor;leslieOn=1;leslieMode=FAST;leslieAmt=0.9;driveOn=1;drive=8"},
{"FX","ROBOT","makeup=4;spk=Laptop Speakers;codec=LPC ROBOT;line=0;delayOn=1;delayL=90;delayR=120;delayFeed=0.25;delayMix=0.2"},
{"FX","WHISPER SYNTH","spk=Flat TV Speakers;codec=LPC WHISPER;line=0;roomOn=1;roomIdx=Tiled Bathroom;room=0.2"},
{"FX","8-BIT TOY","makeup=3;spk=Distant Toy Keyboard;crushOn=1;crush=0.65"},
{"RADIO & COMMS","BUNKER SIGNAL","makeup=4;spk=Office Intercom;codec=SPEEX 2K;line=0.35;crushOn=1;crush=0.25;roomOn=1;roomIdx=Concrete Bunker;room=0.4;bay=LINE/Morse Code@-30"},
{"PLACES","SHOWER RADIO","makeup=8;spk=Tiny Amp Bright;roomOn=1;roomIdx=Tiled Bathroom;room=0.55;modOn=1;modType=CHORUS;modDepth=0.2;modSpeed=0.6"},
// added in 0.4.6
{"PHONES","ANSWERING MACHINE","makeup=2;spk=Desk Phone Earpiece;codec=G711 ULAW;line=0.1;gramOn=1;gramWow=0.25;gramCurve=0.1;gramTicks=0;compOn=1;comp=0.55;bay=LINE/Line Hiss@-30"},
{"PHONES","CALL CENTER AGENT","makeup=7;micOn=1;micIdx=Boom Headset;spk=Desk Phone Earpiece;codec=G711 ALAW;line=0.05;compOn=1;comp=0.6;roomOn=1;roomIdx=Office Far;room=0.12;bay=INDOORS/Typing@-30"},
{"PHONES","CONFERENCE CALL ROOM","makeup=15;spk=Laptop Speakers;codec=SPEEX 15K;line=0.1;gateOn=1;gate=-48;roomOn=1;roomIdx=Conference Room;room=0.35"},
{"PHONES","MOBILE - NO RECEPTION","makeup=-5;spk=Smartphone Speaker;codec=GSM 13K;line=0.9;compOn=1;comp=0.5"},
{"PHONES","CALLING FROM THE PUB","makeup=3;spk=Smartphone Speaker;codec=GSM 13K;line=0.15;roomOn=1;roomIdx=Pub;room=0.3;bay=STREET/Bar Crowd@-22"},
{"PHONES","COMMUTER ON THE TRAIN","spk=Smartphone Speaker;codec=GSM 13K;line=0.4;roomOn=1;roomIdx=Subway Car;room=0.25;bay=STREET/Train Interior@-22"},
{"PHONES","CALL FROM THE BUS STOP","makeup=4;spk=Smartphone Speaker;codec=ADPCM 32K;line=0.2;roomOn=1;roomIdx=Tram Stop Shelter;room=0.3;bay=STREET/Rain@-22"},
{"RADIO & COMMS","POLICE SCANNER","makeup=2;spk=Walkie Talkie;codec=SPEEX 8K;line=0.3;compOn=1;comp=0.9;driveOn=1;drive=14;driveType=HARD CLIP;eqOn=1;eq=0.5;bay=LINE/Radio Static@-24"},
{"RADIO & COMMS","PILOT TO TOWER","makeup=11;micOn=1;micIdx=Boom Headset;spk=Walkie Talkie;codec=SPEEX 8K;line=0.15;compOn=1;comp=0.8;gateOn=1;gate=-45;bay=LINE/Radio Static@-30"},
{"RADIO & COMMS","AM TALK SHOW 1950s","makeup=2;micOn=1;micIdx=Classic Radio Ribbon;spk=Transistor Radio;tuneOn=1;tuneX=0.05;tuneY=0.3;tuneMode=AM;compOn=1;comp=0.6"},
{"RADIO & COMMS","WARTIME BROADCAST","makeup=2;micOn=1;micIdx=1940s Broadcast Dynamic;spk=Transistor Radio;gramOn=1;gramWow=0.1;gramCurve=0.1;gramTicks=0.3;eqOn=1;eq=0.4;bay=LINE/Radio Static@-26"},
{"RADIO & COMMS","FM IN THE CAR","makeup=5;spk=Car Door Speakers;tuneOn=1;tuneMode=FM;tuneX=0.05;tuneY=0.3;compOn=1;comp=0.5;roomOn=1;roomIdx=Inside A Car;room=0.2"},
{"PA & ANNOUNCEMENTS","SUBWAY PLATFORM","makeup=5;spk=Station Tannoy;compOn=1;comp=0.7;roomOn=1;roomIdx=Subway Platform;room=0.45;bay=STREET/Subway Station@-24"},
{"PA & ANNOUNCEMENTS","SUPERMARKET PAGING","makeup=4;micOn=1;micIdx=Gooseneck Paging Mic;spk=Office Intercom;compOn=1;comp=0.6;roomOn=1;roomIdx=Supermarket;room=0.4"},
{"PA & ANNOUNCEMENTS","SCHOOL INTERCOM","makeup=-3;micOn=1;micIdx=Gooseneck Paging Mic;spk=Office Intercom;driveOn=1;drive=6;eqOn=1;eq=0.3;roomOn=1;roomIdx=Classroom;room=0.35"},
{"PA & ANNOUNCEMENTS","COACH WITH A MEGAPHONE","spk=Megaphone Horn;compOn=1;comp=0.85;driveOn=1;drive=16;roomOn=1;roomIdx=Gymnasium;room=0.4"},
{"PA & ANNOUNCEMENTS","MALL ANNOUNCEMENT","makeup=4;spk=Station Tannoy;compOn=1;comp=0.6;roomOn=1;roomIdx=Mall Food Court;room=0.4"},
{"PLACES","MUSIC IN THE STAIRWELL","makeup=-2;spk=Floor Wedge Monitor;eqOn=1;eq=0.3;roomOn=1;roomIdx=Concrete Stairwell;room=0.6"},
{"PLACES","DOWNSTAIRS NEIGHBOR'S TV","makeup=7;spk=Flat TV Speakers;coverOn=1;coverIdx=Air Vent;cover=0.6;eqOn=1;eq=0.5;roomOn=1;roomIdx=Basement;room=0.4"},
{"PLACES","PIZZERIA RADIO","makeup=2;spk=Transistor Radio;tuneOn=1;tuneX=0.08;tuneY=0.3;roomOn=1;roomIdx=Pizzeria;room=0.4;bay=STREET/Bar Crowd@-26"},
{"PLACES","LAUNDROMAT RADIO","makeup=2;spk=Transistor Radio;roomOn=1;roomIdx=Laundry Room;room=0.35;bay=INDOORS/Clothes Dryer@-24"},
{"PLACES","BUSKER ACROSS THE STREET","makeup=5;spk=Tiny Amp Mid;eqOn=1;eq=0.2;roomOn=1;roomIdx=City Street;room=0.5;bay=STREET/City Street@-22"},
{"VINTAGE","1930s NEWSREEL","makeup=-5;micOn=1;micIdx=Film Boom Ribbon Dark;spk=Cabinet Gramophone;gramOn=1;gramWow=0.2;gramCurve=0.2;gramTicks=0.4;eqOn=1;eq=0.35;bay=STREET/Film Static@-24"},
{"VINTAGE","CRYSTAL MIC DEMO TAPE","makeup=13;micOn=1;micIdx=Crystal Element;spk=Cassette Recorder Speaker;gramOn=1;gramWow=0.2;gramTicks=0;bay=LINE/Line Hiss@-30"},
{"VINTAGE","HARMONICA BULLET MIC","makeup=-7;micOn=1;micIdx=1950s PA Mic;spk=Tiny Amp Dark;driveOn=1;drive=20;driveType=TUBE;roomOn=1;roomIdx=Pub;room=0.2"},
{"VINTAGE","BROKEN STAGE MIC","makeup=2;micOn=1;micIdx=Broken Dynamic;spk=Floor Wedge Monitor;driveOn=1;drive=10;roomOn=1;roomIdx=Sports Bar;room=0.3"},
{"VINTAGE","LO-FI RIBBON VOCAL","makeup=2;spkOn=0;micOn=1;micIdx=Budget Ribbon;driveOn=1;drive=8;driveType=TUBE;compOn=1;comp=0.5"},
{"FX","ROBOT IN A TIN CAN","makeup=3;spk=Tiny Amp Bright;codec=LPC ROBOT;line=0;coverOn=1;coverIdx=Soda Can;cover=0.8"},
{"FX","VOICE DOWN THE ALLEY","makeup=3;spk=Flat TV Speakers;codec=LPC WHISPER;line=0;roomOn=1;roomIdx=Downtown Canyon;room=0.5;delayOn=1;delayL=300;delayR=410;delayFeed=0.3;delayMix=0.2"},
};
// display order: categories in first-appearance order, presets in index order within a category
#include <vector>
#include <string>
inline std::vector<int> presetOrder() { std::vector<std::string> cs; const int n = (int)(sizeof(factory) / sizeof(factory[0]));
    for (int i = 0; i < n; ++i) { bool f = false; for (auto& c : cs) f |= c == factory[i].cat; if (!f) cs.push_back(factory[i].cat); }
    std::vector<int> o; for (auto& c : cs) for (int i = 0; i < n; ++i) if (c == factory[i].cat) o.push_back(i); return o; }
