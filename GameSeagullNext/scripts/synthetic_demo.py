#!/usr/bin/env python3
"""Runs changing generated BGRA frames through the ACTUAL C++ vision/geometry core.
The GIF is a lab demonstration, not an iOS screen capture or device proof.
Requires Pillow and build/libseagull.so (created by validate_reference.py or g++).
"""
import ctypes,json,math,pathlib
from PIL import Image,ImageDraw,ImageFont
root=pathlib.Path(__file__).resolve().parents[1]
lib=ctypes.CDLL(str(root/'build/libseagull.so'));lib.sg_analyze.argtypes=[ctypes.c_void_p,ctypes.c_int,ctypes.c_int,ctypes.c_int];lib.sg_analyze.restype=ctypes.c_void_p;lib.sg_free.argtypes=[ctypes.c_void_p]
frames=[];records=[];font=ImageFont.load_default(size=16)
for i in range(36):
    im=Image.new('RGB',(320,600),(90,65,55));d=ImageDraw.Draw(im)
    d.rectangle((30,40,289,559),fill=(28,125,112))
    for x,y in [(30,40),(290,40),(30,300),(290,300),(30,560),(290,560)]:d.ellipse((x-13,y-13,x+13,y+13),fill=(8,8,8))
    angle=math.radians(-115+i*1.35)
    d.line((155+10*math.cos(angle),420+10*math.sin(angle),155+65*math.cos(angle),420+65*math.sin(angle)),fill=(245,245,245),width=2)
    for x,y,color in [(155,420,(245,245,245)),(100,170,(220,35,40)),(210,250,(230,185,30))]:d.ellipse((x-7,y-7,x+7,y+7),fill=color)
    raw=im.convert('RGBA').tobytes('raw','BGRA');p=lib.sg_analyze(raw,320,600,1280)
    try:state=json.loads(ctypes.string_at(p))
    finally:lib.sg_free(p)
    records.append(state)
    canvas=Image.new('RGB',(692,692),'#111521');draw=ImageDraw.Draw(canvas)
    draw.text((18,14),'SYNTHETIC LAB DEMO — NOT AN IPHONE RECORDING',font=font,fill='white')
    canvas.paste(im,(16,58));view=im.copy();v=ImageDraw.Draw(view)
    colors=['white','#42d8f5','#ffb04c','#7effa5']
    for line in state.get('prediction',[]):v.line((*line['a'],*line['b']),fill=colors[line['role']],width=2)
    if state.get('shots'):
        for line in state['shots'][0]['lines']:v.line((*line['a'],*line['b']),fill=colors[3],width=2)
    canvas.paste(view,(356,58));draw.text((18,666),f'Generated input    |    Core output    |    Frame {i+1:02d}/36',font=font,fill='white');frames.append(canvas)
frames[0].save(root/'artifacts/synthetic-live-demo.gif',save_all=True,append_images=frames[1:],duration=125,loop=0)
(root/'artifacts/synthetic-live-stream.json').write_text(json.dumps(records))
result={'frames':len(records),'validFrames':sum(s['valid'] for s in records),'aimDetectedFrames':sum(s['aimConfidence']>.65 for s in records),'source':'Generated BGRA frames; no iPhone capture','differentPredictions':len({json.dumps(s['prediction']) for s in records})}
(root/'artifacts/stream-validation.json').write_text(json.dumps(result,indent=2));print(result)
assert result['validFrames']==36 and result['aimDetectedFrames']>=30 and result['differentPredictions']>=30
