import json,glob,os,collections
c=collections.Counter()
for p in sorted(glob.glob('/home/claude/untrusted/sfx/atomcut/packs/*/pack.json')):
    d=json.load(open(p)); n=os.path.basename(os.path.dirname(p))
    a=glob.glob(os.path.dirname(p)+'/audio/*'); lic=d.get('license'); c[str(lic)]+=1
    print(f"{n[:58]:58s} {str(lic)[:12]:12s} {len(a):4d} {str(d.get('kind'))[:10]:10s} {str(d.get('attribution'))[:50]}")
print(c)
