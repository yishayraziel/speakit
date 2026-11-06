import soundfile as sf,numpy as np,sys,os
root='/home/claude/untrusted/ir-lib/repo'
sel=['Speakers/Phones','Speakers/Computers','Speakers/Toys and Novelty','Speakers/Monitors','Speakers/Amplifiers/Tiny T-Shirt Amplifier','Speakers/Miscellaneous','Enclosed Spaces','Hardware/Old Cabinet Gramophone','Hardware/Tape/Portable Cassette Recorder','Hardware/2004 Digital Piano','Hardware/Noise Machine','Objects/Containers']
for s in sel:
  for d,_,fs in os.walk(os.path.join(root,s)):
    for f in sorted(fs):
      if not f.endswith('.wav'):continue
      p=os.path.join(d,f);x,sr=sf.read(p,always_2d=True);m=x.mean(1)
      X=np.abs(np.fft.rfft(m,1<<16));fr=np.fft.rfftfreq(1<<16,1/sr);e=np.cumsum(X**2)/np.sum(X**2)
      print(f"{f[:48]:48s} {sr} ch{x.shape[1]} {len(m)/sr:5.2f}s pk@{np.argmax(np.abs(m))/sr*1000:6.1f}ms lo{fr[np.searchsorted(e,.02)]:6.0f} hi{fr[np.searchsorted(e,.98)]:6.0f}")
