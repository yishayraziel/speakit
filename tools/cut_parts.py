#!/usr/bin/env python3
"""Cuts the metal Dry/Wet knob and the grille out of the main skin plate (run before removing plate_main.jpg)."""
from PIL import Image, ImageDraw
S=2048/1932; im=Image.open('Resources/skin/plate_main.jpg').convert('RGBA')
def cut(cx,cy,r,out):
    cx,cy,r=cx*S,cy*S,r*S; x,y,d=int(cx-r),int(cy-r),int(2*r); c=im.crop((x,y,x+d,y+d))
    m=Image.new('L',(d*2,d*2),0); ImageDraw.Draw(m).ellipse((6,6,d*2-6,d*2-6),fill=255); c.putalpha(m.resize((d,d))); c.save(out)
cut(1572,698,142,'Resources/skin/knob_metal.png'); cut(308,598,272,'Resources/skin/grille.png')
