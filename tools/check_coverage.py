#!/usr/bin/env python3
"""Assert that COVERAGE.md accounts for every function in original/.

The repository claims nothing from the internship was dropped. That claim is
only worth something if it is checked, so this runs in CI: every `def` and
`function` under original/ must appear as a row in COVERAGE.md, and every row
must name a function that actually exists.
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DEFINITION = re.compile(r"^(?:def|function)\s+([A-Za-z_][A-Za-z_0-9]*)", re.M)


def functions_in_original():
    """Every function defined under original/, as (file stem, name)."""
    found = set()
    for source in sorted(ROOT.glob("original/**/*.py")) + sorted(ROOT.glob("original/**/*.jl")):
        for name in DEFINITION.findall(source.read_text()):
            found.add((source.name, name))
    return found


def functions_in_coverage():
    """Every function named in the first column of a COVERAGE.md table row."""
    text = (ROOT / "COVERAGE.md").read_text()
    listed = set()
    current_file = None
    for line in text.splitlines():
        heading = re.match(r"^#+\s+`([^`]+)`", line)
        if heading:
            current_file = Path(heading.group(1)).name
            continue
        row = re.match(r"^\|\s*`([A-Za-z_][A-Za-z_0-9]*)`", line)
        if row and current_file:
            listed.add((current_file, row.group(1)))
    return listed


def main():
    defined = functions_in_original()
    listed = functions_in_coverage()

    missing = sorted(defined - listed)
    invented = sorted(listed - defined)

    for source, name in missing:
        print(f"NOT ACCOUNTED FOR  {source}: {name}")
    for source, name in invented:
        print(f"NOT IN THE ORIGINAL  {source}: {name}")

    print(f"\n{len(defined)} functions in original/, {len(listed)} rows in COVERAGE.md")
    if missing or invented:
        print(f"{len(missing)} unaccounted for, {len(invented)} invented")
        return 1
    print("every function of the original is accounted for")
    return 0


if __name__ == "__main__":
    sys.exit(main())
