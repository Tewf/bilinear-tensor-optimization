#!/usr/bin/env python3
"""Ask every backend the same rank question and tabulate what each one cost.

Four instruments answer "is there an algorithm with k products", and they answer
it in different currencies: a tree search counts nodes, a SAT solver counts
conflicts, a MILP counts branch-and-bound nodes. The only currency they share is
wall-clock on one machine, so that is what this measures, and it measures it on
the same tensors with the same targets.

Each backend appears twice where it can: once plain, once with the orbit
reduction, since the whole claim of this repository's symmetry work is that
quotienting the search by the map's automorphisms is worth more than the term
ordering the published encodings break symmetry with.

    tools/compare_backends.py --build build --fixtures fixtures

Nothing here proves anything. A FOUND is checked by the command that produced it
against the map it must compute; a `no` is only as good as the search that
exhausted, which is why the timeout column matters and is reported.
"""
import argparse
import re
import subprocess
import time
from pathlib import Path

# (tensor, target, is-a-matmul-of-this-shape-or-None). Targets are chosen so
# each row is a question with a known answer, so a wrong FOUND is visible.
QUESTIONS = [
    ("f2_2x2", 3, None, "yes"),
    ("f2_2x2", 2, None, "no"),
    ("matmul_2x2x2", 7, (2, 2, 2), "yes"),
    ("matmul_2x2x2", 6, (2, 2, 2), "no"),
    ("f2_2x3", 5, None, "yes"),
]


def run(command, timeout):
    """(verdict, seconds). Verdict is 'yes', 'no', 'timeout' or 'error'."""
    started = time.monotonic()
    try:
        done = subprocess.run(command, capture_output=True, text=True, timeout=timeout,
                              stdin=subprocess.DEVNULL)
    except subprocess.TimeoutExpired:
        return "timeout", timeout
    elapsed = time.monotonic() - started
    # The verdict is read before the exit code, because `decide-rank` reports a
    # negative answer *through* a non-zero exit and would otherwise look broken.
    text = done.stdout
    if re.search(r"\bFOUND\b", text):
        return "yes", elapsed
    if re.search(r"\bNO\b|:\s*no\b", text):
        return "no", elapsed
    return "error", elapsed


def backends(build, fixtures, name, target, shape):
    """Every (label, command) worth asking this question of."""
    tensor = str(Path(fixtures) / f"{name}.tensor")
    build = Path(build)
    plans = [
        ("tree search", [str(build / "bilinear_rank/decide-rank"), tensor, "--target", str(target)]),
        ("ILP", [str(build / "bilinear_rank/decide-rank-by-ilp"), tensor, "--target", str(target)]),
        ("SAT", [str(build / "satisfiability/decide-rank-by-sat"), tensor, "--target", str(target)]),
        ("SAT + ordering",
         [str(build / "satisfiability/decide-rank-by-sat"), tensor, "--target", str(target),
          "--break-symmetry"]),
    ]
    if shape is not None:
        symmetry = ["--symmetry", "matmul", *map(str, shape)]
        plans.insert(1, ("tree search + orbits",
                         [str(build / "bilinear_rank/decide-rank"), tensor, "--target",
                          str(target), *symmetry]))
        plans.insert(3, ("ILP + orbits",
                         [str(build / "bilinear_rank/decide-rank-by-ilp"), tensor, "--target",
                          str(target), *symmetry]))
    return plans


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", default="build")
    parser.add_argument("--fixtures", default="fixtures")
    parser.add_argument("--timeout", type=float, default=300.0)
    parser.add_argument("--only", default=None, help="substring filter on the backend label")
    options = parser.parse_args()

    wrong = 0
    for name, target, shape, expected in QUESTIONS:
        print(f"\n{name} at k = {target}   (the answer is {expected})")
        print(f"  {'backend':<24} {'verdict':<9} seconds")
        for label, command in backends(options.build, options.fixtures, name, target, shape):
            if options.only and options.only not in label:
                continue
            if not Path(command[0]).exists():
                print(f"  {label:<24} {'absent':<9} -")
                continue
            verdict, seconds = run(command, options.timeout)
            flag = ""
            if verdict in ("yes", "no") and verdict != expected:
                flag = "   <-- WRONG"
                wrong += 1
            print(f"  {label:<24} {verdict:<9} {seconds:8.2f}{flag}")

    if wrong:
        print(f"\n{wrong} backend(s) gave the wrong answer.")
    return 1 if wrong else 0


if __name__ == "__main__":
    raise SystemExit(main())
