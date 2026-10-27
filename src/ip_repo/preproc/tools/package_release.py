#!/usr/bin/env python3
"""Make a clean, self-contained source archive without workspace products."""
from pathlib import Path
import hashlib,zipfile

root=Path(__file__).resolve().parents[1]
names=['README.md','component.xml']
for folder in ('rtl','constraints','scripts','tb','tools','driver','vectors','xgui'):
    names += [p.relative_to(root).as_posix() for p in (root/folder).rglob('*')
              if p.is_file() and '__pycache__' not in p.parts and p.suffix!='.pyc']
names += ['build/vivado_impl/utilization_synth.rpt',
          'build/vivado_impl/timing_synth.rpt',
          'build/vivado_impl/utilization_impl.rpt',
          'build/vivado_impl/timing_impl.rpt']
names=sorted(set(names))
for name in names:
    if not (root/name).is_file(): raise SystemExit(f'missing {name}')
manifest=''.join(f'{hashlib.sha256((root/n).read_bytes()).hexdigest()}  {n}\n' for n in names)
(root/'MANIFEST.sha256').write_text(manifest,encoding='utf-8')
out=root.parent/'picodet_preproc_ip_source_20261017.zip'
with zipfile.ZipFile(out,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=6) as z:
    for name in names+['MANIFEST.sha256']:
        z.write(root/name,arcname='picodet_preproc_ip/'+name)
print(f'{out}: {len(names)+1} files, {out.stat().st_size} bytes')
