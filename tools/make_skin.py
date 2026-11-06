#!/usr/bin/env python3
"""Turns Yishay's render into a working skin: removes baked pointers, LEDs and text; cuts sprites."""
import cv2, numpy as np, sys
src=cv2.imread(sys.argv[1]); S=2048/1932
def sc(*v): return [int(round(x*S)) for x in v]
p=src.copy()
def inp(img,mask,r=5): return cv2.inpaint(img,mask,r,cv2.INPAINT_TELEA)
mask=np.zeros(p.shape[:2],np.uint8)
smalls=[(704,563),(959,563),(1212,563),(704,957),(959,957),(1212,957),(704,1355),(959,1355),(1212,1355)]
big=[(206,1705),(736,1705),(1268,1705)]
def ptr(c,r,w):
    x,y=sc(*c); r=int(r*S); w=int(w*S)
    cv2.rectangle(mask,(x-w,y-r),(x+w,y+int(4*S)),255,-1)
for c in smalls: ptr(c,60,6)
for c in big: ptr(c,82,7)
p=inp(p,mask,4)
# clean knob sprite from LFO 1 knob
cx,cy=sc(206,1705); R=int(70*S)
k=p[cy-R:cy+R,cx-R:cx+R].copy(); a=np.zeros((2*R,2*R),np.uint8); cv2.circle(a,(R,R),R-3,255,-1,cv2.LINE_AA); a=cv2.GaussianBlur(a,(5,5),0)
cv2.imwrite('Resources/skin/knob.png',np.dstack([k,a]))
# give every small knob the same clean cap so COMP matches the rest
for c in smalls:
    x,y=sc(*c); r=int(50*S); s=cv2.resize(k,(2*r,2*r),interpolation=cv2.INTER_AREA); al=cv2.resize(a,(2*r,2*r))[:,:,None]/255.
    roi=p[y-r:y+r,x-r:x+r]; p[y-r:y+r,x-r:x+r]=(s*al+roi*(1-al)).astype(np.uint8)
# toggle sprite
tx,ty=sc(704,420); r=int(42*S); t=src[ty-r:ty+r,tx-r:tx+r].copy(); ta=np.zeros((2*r,2*r),np.uint8); cv2.circle(ta,(r,r),r-2,255,-1,cv2.LINE_AA); ta=cv2.GaussianBlur(ta,(7,7),0)
cv2.imwrite('Resources/skin/toggle.png',np.dstack([t,ta]))
# metal knob pointer
m2=np.zeros(p.shape[:2],np.uint8); a0=np.array(sc(1578,655)); a1=np.array(sc(1598,545)); cv2.line(m2,tuple(a0),tuple(a1),255,int(16*S)); p=inp(p,m2,6)
# flat dark fills for live readouts
def fill(x0,y0,x1,y1,col):
    a,b,c,d=sc(x0,y0,x1,y1); roi=p[b:d,a:c]; p[b:d,a:c]=col
fill(765,92,1458,178,(14,18,18)); fill(868,776,1056,848,(10,14,18)); fill(106,1052,508,1398,(26,28,28))
# meter LEDs
x0,y0,x1,y1=sc(1806,342,1878,1404); p[y0:y1,x0:x1]=(14,15,15)
# version number and blank texture
tex=src[sc(920)[0]:sc(1380)[0],sc(1400)[0]:sc(1760)[0]].copy()
def texfill(x0,y0,x1,y1):
    a,b,c,d=sc(x0,y0,x1,y1); h,w=tex.shape[:2]; T=np.vstack([np.hstack([tex,tex[:,::-1]]),np.hstack([tex[::-1],tex[::-1,::-1]])]); T=np.tile(T,(4,4,1))
    mk=np.zeros((d-b,c-a),np.float32)+1; e=10; mk[:e]*=np.linspace(0,1,e)[:,None]; mk[-e:]*=np.linspace(1,0,e)[:,None]; mk[:,:e]*=np.linspace(0,1,e); mk[:,-e:]*=np.linspace(1,0,e)
    p[b:d,a:c]=(T[:d-b,:c-a]*mk[:,:,None]+p[b:d,a:c]*(1-mk[:,:,None])).astype(np.uint8)
texfill(1760,132,1850,178)
texfill(1610,1530,1900,1880-85); texfill(1610,1770,1890,1830)
cv2.imwrite('Resources/skin/plate_main.png',p,[cv2.IMWRITE_PNG_COMPRESSION,9])
q=p.copy(); texfill(40,262,1892,1472)
cv2.imwrite('Resources/skin/plate_page.png',p,[cv2.IMWRITE_PNG_COMPRESSION,9])
for n in ['plate_main','plate_page']:
    im=cv2.imread(f'Resources/skin/{n}.png'); cv2.imwrite(f'Resources/skin/{n}.jpg',im,[cv2.IMWRITE_JPEG_QUALITY,92])
