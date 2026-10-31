#!/usr/bin/env python3
"""Verify source provenance and the matching curated reference archives."""
import hashlib
from pathlib import Path
import re
import zipfile

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / 'sha256sums.txt'

def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda: f.read(1048576), b''):
            h.update(chunk)
    return h.hexdigest()

def files():
    for p in sorted(ROOT.rglob('*')):
        if '.git' in p.relative_to(ROOT).parts or '__pycache__' in p.relative_to(ROOT).parts:
            continue
        if p.is_symlink():
            raise ValueError('symlink: ' + str(p))
        for part in p.relative_to(ROOT).parts:
            if not re.fullmatch(r'[a-z0-9_.-]+', part):
                raise ValueError('invalid path: ' + str(p))
        if p.is_file() and p != MANIFEST:
            yield p

def main():
    compared = 0
    archive_only = 0
    with zipfile.ZipFile(ROOT / 'original/eagleeye_z035.zip') as z:
        bad = z.testzip()
        if bad:
            raise ValueError('archive CRC: ' + bad)
        entries = [x for x in z.infolist() if not x.is_dir()]
        if len(entries) != 226:
            raise ValueError('unexpected source file count: ' + str(len(entries)))
        for entry in entries:
            if not entry.filename.startswith('eagleeye_z035/'):
                raise ValueError('unexpected source path')
            rel = entry.filename[len('eagleeye_z035/'):].lower()
            if rel == 'readme.md':
                rel = 'report/source_readme.md'
            elif rel == 'sha256sums.txt':
                rel = 'report/source_sha256sums.txt'
            elif rel in (
                'skill/readme.md',
                'skill/preproc-fp32-golden/readme.md',
                'skill/roi-review-control/readme.md',
                'skill/roi-review-control/tests/test_check_hw_contract.py',
                'skill/roi-review-control/tools/check_hw_contract.py',
                'report/ai_collaboration_record.md',
                'report/design_report_draft.md',
                'report/readme.md',
            ):
                # The original bytes remain in the CRC-checked archive;
                # the displayed files have since been edited or replaced.
                archive_only += 1
                continue
            elif rel == 'skill/lanczos-2x-stream/build/xsim.log':
                rel = 'sim/lanczos_legacy_xsim.txt'
            elif rel == 'skill/lanczos-2x-stream/sha256sums.txt':
                rel = 'sim/lanczos_legacy_rtl_sha256.txt'
            elif rel.startswith('skill/lanczos-2x-stream/'):
                # Preserved in the CRC-checked original archive, not displayed as a Skill.
                archive_only += 1
                continue
            target = ROOT / rel
            if not target.is_file() or hashlib.sha256(z.read(entry)).hexdigest() != digest(target):
                raise ValueError('source mismatch: ' + entry.filename)
            compared += 1
        print('SOURCE_PASS compared=' + str(compared) + ' archive_only=' + str(archive_only))
    for name in ('github_a_fovea', 'github_c_attention', 'github_linux_petalinux'):
        mirror = ROOT / 'reference' / name
        with zipfile.ZipFile(ROOT / 'original' / (name + '.zip')) as z:
            bad = z.testzip()
            if bad:
                raise ValueError('reference CRC: ' + name + '/' + bad)
            entries = [x for x in z.infolist() if not x.is_dir()]
            names = [x.filename.lower() for x in entries]
            if len(names) != len(set(names)):
                raise ValueError('reference case collision: ' + name)
            actual = {p.relative_to(mirror).as_posix() for p in mirror.rglob('*') if p.is_file()}
            if set(names) != actual:
                raise ValueError('reference file list mismatch: ' + name)
            for entry in entries:
                if hashlib.sha256(z.read(entry)).hexdigest() != digest(mirror / entry.filename.lower()):
                    raise ValueError('reference mismatch: ' + entry.filename)
            print('REFERENCE_PASS ' + name + ' files=' + str(len(entries)))
    expected = set(MANIFEST.read_text(encoding='utf-8').splitlines())
    actual = {digest(p) + '  ' + p.relative_to(ROOT).as_posix() for p in files()}
    if actual != expected:
        raise ValueError('manifest mismatch')
    print('PASS original_files=226 repository_files=' + str(len(actual)))

if __name__ == '__main__':
    main()
