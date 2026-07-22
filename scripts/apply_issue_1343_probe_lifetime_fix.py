from __future__ import annotations

from pathlib import Path


def main() -> int:
    path = Path("audit/issue_1343_arven_held_payload_probe.cpp")
    text = path.read_text(encoding="utf-8")
    old = "  sim::Engine engine(scenario, sim::baseline_recipe(), rng);"
    new = "  const auto recipe = sim::baseline_recipe();\n  sim::Engine engine(scenario, recipe, rng);"
    count = text.count(old)
    if count == 0:
        return 0
    if count != 5:
        raise RuntimeError(f"Expected 5 temporary recipe constructions, found {count}")
    path.write_text(text.replace(old, new), encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
