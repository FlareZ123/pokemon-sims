from __future__ import annotations

import os
from pathlib import Path

FSS = Path("src/trace_engine_v2/part_010_fss_override.inc")
TEST = Path("tests/issue_1185_quick_ball_strict_surplus_energy_tests.cpp")


def atomic_write(path: Path, content: str) -> None:
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(content, encoding="utf-8")
    os.replace(temporary, path)


def replace_once(text: str, old: str, new: str, label: str) -> str:
    if new in text:
        return text
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"Unexpected {label} count: {count}")
    return text.replace(old, new, 1)


def main() -> int:
    fss = FSS.read_text(encoding="utf-8")
    fss = replace_once(
        fss,
        "        scenario_.max_turn < 4 || state_.vstar_power_used ||\n"
        "        !supporter_allowed() || state_.manual_energy_used ||",
        "        scenario_.max_turn < 4 ||\n"
        "        !supporter_allowed() || state_.manual_energy_used ||",
        "Star Alchemy resolver gate",
    )
    atomic_write(FSS, fss)

    test = TEST.read_text(encoding="utf-8")
    test = replace_once(
        test,
        "  expect(sim::EngineTestAccess::fss_target(fss) == sim::Card::Grass,\n"
        "         \"The complete K1 route must search Grass.\");\n\n"
        "  sim::State one_grass = seed23_fss_state();",
        "  expect(sim::EngineTestAccess::fss_target(fss) == sim::Card::Grass,\n"
        "         \"The complete K1 route must search Grass.\");\n\n"
        "  sim::State resolving_state = seed23_fss_state();\n"
        "  resolving_state.vstar_power_used = true;\n"
        "  std::mt19937_64 resolving_rng{140306};\n"
        "  sim::Engine resolving =\n"
        "      make_engine(strict, resolving_rng, std::move(resolving_state));\n"
        "  // The Star Alchemy resolver marks the VSTAR Power used before selecting its\n"
        "  // searched card, so the same legal route must remain admitted at resolution:\n"
        "  // https://api.pokemontcg.io/v2/cards/swsh12-156\n"
        "  // https://github.com/FlareZ123/pokemon-sims/issues/1403\n"
        "  expect(sim::EngineTestAccess::seed23_fss_route(resolving),\n"
        "         \"The route must survive Star Alchemy's used-power resolver state.\");\n"
        "  expect(sim::EngineTestAccess::fss_target(resolving) == sim::Card::Grass,\n"
        "         \"Star Alchemy resolution must keep the Grass target.\");\n\n"
        "  sim::State one_grass = seed23_fss_state();",
        "Star Alchemy resolver regression",
    )
    atomic_write(TEST, test)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
