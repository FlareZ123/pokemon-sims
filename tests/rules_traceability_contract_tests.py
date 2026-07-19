#!/usr/bin/env python3
"""Require every rule ID emitted by production trace calls to be registered."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TRACE_CALL = re.compile(r'\btrace\s*\(\s*"[^"]*"\s*,\s*"([^"]*)"', re.DOTALL)
RULE_ID = re.compile(r'\bR-[A-Z0-9-]+\b')
REGISTERED_ID = re.compile(r'^\| `(?P<id>R-[A-Z0-9-]+)` \|', re.MULTILINE)


def main() -> int:
    emitted: set[str] = set()
    for path in (ROOT / "src" / "trace_engine_v2").glob("*.inc"):
        for rule_field in TRACE_CALL.findall(path.read_text(encoding="utf-8")):
            emitted.update(RULE_ID.findall(rule_field))

    register = (ROOT / "docs" / "RULES_TRACEABILITY.md").read_text(encoding="utf-8")
    registered = set(REGISTERED_ID.findall(register))
    missing = sorted(emitted - registered)
    if missing:
        raise AssertionError(f"unregistered emitted rule IDs: {', '.join(missing)}")

    # Guzma's second switch is conditional on the opponent switch occurring, and the
    # repository models that prerequisite through an explicit opponent-Bench state:
    # https://api.pokemontcg.io/v2/cards/sm3-115
    # https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#opponent-actions
    # https://github.com/FlareZ123/pokemon-sims/issues/1033
    if "R-GUZMA-01" not in emitted or "R-GUZMA-01" not in registered:
        raise AssertionError("R-GUZMA-01 must be emitted and registered")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
