#!/usr/bin/env python3
"""Check named modules, external ports and register bases in XSA or HWH."""

import argparse
import json
import xml.etree.ElementTree as ET
from pathlib import Path
from zipfile import BadZipFile, ZipFile


def check(root, contract):
    if not isinstance(contract, dict) or not any(
        contract.get(key) for key in ("modules", "top_ports", "register_bases")
    ):
        raise ValueError("contract contains no checks")
    for key in ("modules", "top_ports"):
        value = contract.get(key, [])
        if not isinstance(value, list) or not all(isinstance(x, str) and x for x in value):
            raise ValueError(f"{key} must be a list of nonempty strings")
    bases = contract.get("register_bases", {})
    if not isinstance(bases, dict) or not all(
        isinstance(k, str) and k and isinstance(v, str) for k, v in bases.items()
    ):
        raise ValueError("register_bases must map nonempty names to address strings")
    modules = {node.get("INSTANCE") for node in root.findall(".//MODULE")}
    ports = {
        node.get("NAME")
        for node in root.findall("./EXTERNALPORTS/PORT")
    }
    ranges = {}
    for node in root.findall(".//MEMRANGE"):
        name = node.get("INSTANCE")
        value = node.get("BASEVALUE")
        if name and value:
            ranges.setdefault(name, set()).add(int(value, 0))

    problems = []
    problems.extend(f"missing module: {name}" for name in contract.get("modules", []) if name not in modules)
    problems.extend(f"missing top port: {name}" for name in contract.get("top_ports", []) if name not in ports)
    for name, value in contract.get("register_bases", {}).items():
        base = int(value, 0)
        if base not in ranges.get(name, set()):
            problems.append(f"register base mismatch: {name} expected {value}")
    return problems


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--xsa", type=Path, help="XSA containing root system.hwh")
    source.add_argument("--hwh", type=Path, help="standalone HWH metadata")
    parser.add_argument("--config", type=Path, required=True)
    args = parser.parse_args()
    try:
        contract = json.loads(args.config.read_text(encoding="utf-8"))
        if args.xsa:
            with ZipFile(args.xsa) as archive:
                root = ET.fromstring(archive.read("system.hwh"))
        else:
            root = ET.fromstring(args.hwh.read_bytes())
        problems = check(root, contract)
    except (OSError, ValueError, KeyError, ET.ParseError, BadZipFile) as error:
        parser.exit(2, f"INVALID: {error}\n")
    print(json.dumps({"passed": not problems, "problems": problems}, ensure_ascii=False))
    return 0 if not problems else 1


if __name__ == "__main__":
    raise SystemExit(main())
