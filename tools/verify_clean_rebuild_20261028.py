"""Verify the archived clean rebuild evidence and its BIT/XSA relationship."""
from hashlib import sha256
from pathlib import Path
import re
import zipfile

ROOT = Path(__file__).resolve().parents[1]
EVIDENCE = ROOT / 'build' / 'clean_rebuild_20261028'
OLD = ROOT / 'board' / 'reproduced_20261028'

def digest(data):
    return sha256(data).hexdigest()

def require(condition, message):
    if not condition:
        raise ValueError(message)

def check():
    for line in (EVIDENCE / 'sha256sums.txt').read_text(encoding='ascii').splitlines():
        expected, name = line.split('  ', 1)
        require(digest((EVIDENCE / name).read_bytes()) == expected, f'hash mismatch: {name}')
    old_bit = (OLD / 'z100_top.bit').read_bytes()
    new_bit = (EVIDENCE / 'artifacts' / 'z100_top.bit').read_bytes()
    require(digest(old_bit) == 'b286f480a1e91282c4be813f35247deffbc6de7d35f31bb9a7984dd3c7fa4a15', 'old BIT identity mismatch')
    require(digest(new_bit) == '7dd0f6ac95d9b8e5f3c62db6707ea8ddad0130bbd23ec0c9cb17faaf413ada70', 'new BIT identity mismatch')
    require(len(old_bit) == len(new_bit), 'BIT sizes differ')
    require([i for i, (a, b) in enumerate(zip(old_bit, new_bit)) if a != b] == [99, 104, 105, 107, 108, 110, 111], 'unexpected BIT differences')
    require(old_bit[166:170] == bytes.fromhex('aa995566') and old_bit[166:] == new_bit[166:], 'configuration payload mismatch')
    require(digest(new_bit[166:]) == '02c426b197721f9b367e33d9047dfe3e7c84191c90227a18f45411bd94526076', 'configuration payload identity mismatch')
    hw = []
    for path, bit in [(OLD / 'eagleeye_z035.xsa', old_bit), (EVIDENCE / 'artifacts' / 'eagleeye_z035.xsa', new_bit)]:
        with zipfile.ZipFile(path) as archive:
            require(archive.testzip() is None, f'bad XSA: {path.name}')
            bit_names = [n for n in archive.namelist() if n.endswith('.bit')]
            hwh_names = [n for n in archive.namelist() if n.endswith('system.hwh')]
            require(len(bit_names) == 1 and len(hwh_names) == 1, f'ambiguous XSA contents: {path.name}')
            require(archive.read(bit_names[0]) == bit, f'XSA BIT mismatch: {path.name}')
            hw.append(archive.read(hwh_names[0]).decode('utf-8'))
    clean = lambda x: re.sub(r'TIMESTAMP="[^"]*"', 'TIMESTAMP="<timestamp>"', x, count=1)
    require(hw[0] != hw[1] and clean(hw[0]) == clean(hw[1]), 'HWH differences exceed timestamp')
    print('PASS: manifest, BIT configuration, XSA BIT and HWH timestamp comparison')
    print('NOTE: configuration equality is not board validation of the new BIT')

if __name__ == '__main__':
    try:
        check()
    except (ValueError, OSError, zipfile.BadZipFile) as exc:
        raise SystemExit(f'FAIL: {exc}')
