#!/usr/bin/env python3
"""Compare two PicoDet prediction JSON files after each tensor is run on the board.

Each JSON file is a list of {category_id, score, bbox:[x,y,w,h]} records.
Matched boxes must share category and have IoU >= 0.5.
"""
import argparse,json
from pathlib import Path

def iou(a,b):
    ax0,ay0,aw,ah=a['bbox']; bx0,by0,bw,bh=b['bbox']
    x=max(0,min(ax0+aw,bx0+bw)-max(ax0,bx0))
    y=max(0,min(ay0+ah,by0+bh)-max(ay0,by0))
    inter=x*y
    return inter/(aw*ah+bw*bh-inter) if aw*ah+bw*bh-inter>0 else 0

def main():
    p=argparse.ArgumentParser()
    p.add_argument('bicubic_json',type=Path)
    p.add_argument('bilinear_json',type=Path)
    p.add_argument('--threshold',type=float,default=.5)
    a=p.parse_args()
    old=[x for x in json.loads(a.bicubic_json.read_text()) if x['score']>=a.threshold]
    new=[x for x in json.loads(a.bilinear_json.read_text()) if x['score']>=a.threshold]
    candidates=sorted(((iou(x,y),i,j) for i,x in enumerate(old) for j,y in enumerate(new)
                       if x['category_id']==y['category_id']),reverse=True)
    used_old=set();used_new=set();matches=[]
    for overlap,i,j in candidates:
        if overlap<.5: break
        if i not in used_old and j not in used_new:
            used_old.add(i);used_new.add(j);matches.append((overlap,i,j))
    score_delta=[abs(old[i]['score']-new[j]['score']) for _,i,j in matches]
    print(json.dumps({'bicubic_count':len(old),'bilinear_count':len(new),
                      'matched':len(matches),'removed':len(old)-len(matches),
                      'added':len(new)-len(matches),
                      'max_score_delta':max(score_delta,default=0)},indent=2))

if __name__=='__main__': main()
