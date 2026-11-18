#!/usr/bin/env python3
"""Optional external regression image. Source asset remains outside this project.
Usage: python scripts/validate_reference.py /path/to/pool-tool-screenshot.png
Requires Pillow; shared core is built automatically with g++.
"""
import ctypes,json,pathlib,subprocess,sys,time
from PIL import Image
root=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['g++','-std=c++17','-O2','-shared','-fPIC',*[str(p) for p in (root/'core').glob('*.cpp')],'-o',str(root/'build/libseagull.so')],check=True)
lib=ctypes.CDLL(str(root/'build/libseagull.so'))
lib.sg_analyze.argtypes=[ctypes.c_void_p,ctypes.c_int,ctypes.c_int,ctypes.c_int];lib.sg_analyze.restype=ctypes.c_void_p;lib.sg_free.argtypes=[ctypes.c_void_p]
im=Image.open(sys.argv[1]).crop((112,133,974,1703));im.thumbnail((640,640));raw=im.convert('RGBA').tobytes('raw','BGRA')
start=time.perf_counter();pointer=lib.sg_analyze(raw,im.width,im.height,im.width*4)
try: state=json.loads(ctypes.string_at(pointer))
finally:lib.sg_free(pointer)
milliseconds=(time.perf_counter()-start)*1000
# Approximate centers hand-labeled by visual inspection in the ORIGINAL 1948x1852 image.
labels=[(606,248),(656,349),(764,445),(740,485),(442,526),(820,586),(371,613),(736,780),(838,900),(329,916),(426,998),(530,1040),(262,1158),(438,1279),(734,1595)]
truth=[((x-112)*im.width/862,(y-133)*im.height/1570) for x,y in labels]
remaining=list(state['balls']);errors=[]
for x,y in truth:
    nearest=min(remaining,key=lambda b:(b['p'][0]-x)**2+(b['p'][1]-y)**2)
    errors.append(((nearest['p'][0]-x)**2+(nearest['p'][1]-y)**2)**.5);remaining.remove(nearest)
report={'source':'https://github.com/mikee478/pool-tool/blob/dcf9656afcd740e956ca4f830205d705c46d42c6/pool-tool-screenshot.png','evaluation':'Development fixture used during tuning; NOT held-out validation. Approximate labels.','imageSize':[im.width,im.height],'ballCount':len(state['balls']),'pocketCount':len(state['pockets']),'valid':state['valid'],'meanCenterErrorPixels':sum(errors)/len(errors),'maxCenterErrorPixels':max(errors),'hostMilliseconds':milliseconds,'manualClassCheck':'15/15 kinds appear consistent with visible solid/stripe/cue/eight appearance; ball numbers are not recognized.'}
(root/'artifacts/repository-image-detection.json').write_text(json.dumps(state,indent=2))
(root/'artifacts/reference-validation.json').write_text(json.dumps(report,indent=2))
print(json.dumps(report,indent=2))
assert state['valid'] and len(state['balls'])==15 and len(state['pockets'])==6 and max(errors)<8
