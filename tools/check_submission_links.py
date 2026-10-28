"""Check local inline Markdown links in the contest report, sim and skill docs."""
from pathlib import Path
import re
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]


def main():
    files = [ROOT / 'readme.md']
    for folder in ('report', 'sim', 'skill'):
        files.extend(sorted((ROOT / folder).rglob('*.md')))
    valid = 0
    external = 0
    missing = []
    for doc in files:
        in_fence = False
        for number, line in enumerate(doc.read_text(encoding='utf-8', errors='replace').splitlines(), 1):
            if re.match(r'^\s*(```|~~~)', line):
                in_fence = not in_fence
                continue
            if in_fence:
                continue
            for link in re.findall(r'!?\[[^\]]*\]\(([^)]+)\)', line):
                parsed = urlsplit(link.strip().split(' ', 1)[0].strip('<>'))
                if parsed.scheme or parsed.netloc:
                    external += 1
                    continue
                if not parsed.path:
                    continue
                destination = (doc.parent / unquote(parsed.path)).resolve()
                if not destination.is_relative_to(ROOT) or not destination.exists():
                    missing.append(f'{doc.relative_to(ROOT)}:{number}: {link}')
                else:
                    valid += 1
    print(f'documents={len(files)} local_links_ok={valid} missing={len(missing)} external_unchecked={external}')
    for entry in missing:
        print('MISSING', entry)
    print('Scope: inline Markdown file links; excludes code fences, anchors and external URL availability.')
    return 1 if missing else 0


if __name__ == '__main__':
    raise SystemExit(main())
