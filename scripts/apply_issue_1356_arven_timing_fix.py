from __future__ import annotations

import os
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator, TextIO


FSS_PATH = Path("src/trace_engine_v2/part_010_fss_override.inc")
FSS_USE_PATH = Path("src/trace_engine_v2/part_011_fss_latias_override.inc")
TEST_PATH = Path("tests/issue_1356_fss_treasure_energy_tests.cpp")
LOCK_PATH = FSS_PATH.with_suffix(FSS_PATH.suffix + ".issue1356-timing.lock")


@contextmanager
def locked_file(path: Path) -> Iterator[TextIO]:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle = path.open("a+", encoding="utf-8")
    try:
        if os.name == "nt":
            import msvcrt

            handle.seek(0)
            msvcrt.locking(handle.fileno(), msvcrt.LK_LOCK, 1)
        else:
            import fcntl

            fcntl.flock(handle.fileno(), fcntl.LOCK_EX)
        yield handle
    finally:
        if os.name == "nt":
            import msvcrt

            handle.seek(0)
            msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)
        else:
            import fcntl

            fcntl.flock(handle.fileno(), fcntl.LOCK_UN)
        handle.close()


def atomic_write(path: Path, content: str) -> None:
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(content, encoding="utf-8")
    os.replace(temporary, path)


def replace_once(text: str, old: str, new: str, description: str) -> str:
    if new in text:
        return text
    if text.count(old) != 1:
        raise RuntimeError(f"Unexpected {description} anchor count: {text.count(old)}")
    return text.replace(old, new, 1)


def main() -> int:
    with locked_file(LOCK_PATH):
        fss = FSS_PATH.read_text(encoding="utf-8")
        fss = replace_once(
            fss,
            "        scenario_.max_turn < 3 || !deck_seen_ ||\n"
            "        !state_.supporter_used || state_.vstar_power_used ||",
            "        scenario_.max_turn < 3 || !deck_seen_ ||\n"
            "        state_.vstar_power_used ||",
            "Forest Seal Stone Arven-resolution timing",
        )
        fss = replace_once(
            fss,
            "        scenario_.max_turn < 3 || !deck_seen_ ||\n"
            "        state_.vstar_power_used ||\n"
            "        !state_.manual_energy_used || item_locked() || !state_.active ||",
            "        scenario_.max_turn < 3 || !deck_seen_ ||\n"
            "        !state_.manual_energy_used || item_locked() || !state_.active ||",
            "Forest Seal Stone in-progress target timing",
        )
        atomic_write(FSS_PATH, fss)

        fss_use = FSS_USE_PATH.read_text(encoding="utf-8")
        fss_use = replace_once(
            fss_use,
            "          case Card::Grass:\n"
            "            return grass_needed() > 0 && !state_.manual_energy_used;\n"
            "          case Card::Fire:\n"
            "            return fire_needed() > 0 && !state_.manual_energy_used;",
            "          case Card::Grass:\n"
            "            // The confirmed split route banks Grass for the next modeled turn\n"
            "            // after this turn's manual attachment is already used. Treasure\n"
            "            // independently supplies Regidrago VSTAR and preserves the T3 payload:\n"
            "            // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156\n"
            "            // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113\n"
            "            // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136\n"
            "            // Turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules\n"
            "            // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1356\n"
            "            return (grass_needed() > 0 && !state_.manual_energy_used) ||\n"
            "                fss_should_split_treasure_vstar_and_next_turn_energy();\n"
            "          case Card::Fire:\n"
            "            // The same proved split applies when Fire is the banked next-turn\n"
            "            // attachment: https://github.com/FlareZ123/pokemon-sims/issues/1356\n"
            "            return (fire_needed() > 0 && !state_.manual_energy_used) ||\n"
            "                fss_should_split_treasure_vstar_and_next_turn_energy();",
            "Forest Seal Stone next-turn Energy advancement gate",
        )
        atomic_write(FSS_USE_PATH, fss_use)

        tests = TEST_PATH.read_text(encoding="utf-8")
        tests = replace_once(
            tests,
            "  static Card fss_target(const Engine& engine) {\n"
            "    return engine.fss_target_after_search_started();\n"
            "  }",
            "  static Card fss_target(const Engine& engine) {\n"
            "    return engine.fss_target_after_search_started();\n"
            "  }\n"
            "  static bool use_fss(Engine& engine) { return engine.use_fss(); }",
            "focused Forest Seal Stone use-gate accessor",
        )
        tests = replace_once(
            tests,
            "  state.supporter_used = true;\n  state.manual_energy_used = true;",
            "  // Forest Seal Stone is evaluated during Arven's resolution, before the\n"
            "  // engine commits the Supporter-slot flag. The public route must be live at\n"
            "  // this exact timing boundary: https://github.com/FlareZ123/pokemon-sims/issues/1356\n"
            "  state.supporter_used = false;\n  state.manual_energy_used = true;",
            "focused Arven timing fixture",
        )
        tests = replace_once(
            tests,
            "void resolve_star_alchemy_for_fire(sim::State& state) {\n"
            "  state.vstar_power_used = true;",
            "void resolve_star_alchemy_for_fire(sim::State& state) {\n"
            "  // Arven has completed before the later Item sequence begins.\n"
            "  // https://api.pokemontcg.io/v2/cards/sv1-166\n"
            "  // https://www.pokemon.com/us/pokemon-tcg/rules\n"
            "  state.supporter_used = true;\n"
            "  state.vstar_power_used = true;",
            "post-Arven fixture transition",
        )
        tests = replace_once(
            tests,
            "void test_split_is_symmetric_for_missing_grass() {",
            "void test_live_fss_gate_banks_next_turn_fire() {\n"
            "  std::mt19937_64 rng(135608);\n"
            "  sim::Engine engine(test_scenario(), test_recipe(), rng);\n"
            "  sim::EngineTestAccess::set_state(engine, split_state());\n"
            "\n"
            "  // Star Alchemy marks its VSTAR Power used immediately before choosing the\n"
            "  // searched card. The split helper must remain valid throughout that legal\n"
            "  // in-progress resolution and bank Fire for the next modeled turn:\n"
            "  // https://api.pokemontcg.io/v2/cards/swsh12-156\n"
            "  // https://api.pokemontcg.io/v2/cards/sm6-113\n"
            "  // https://github.com/FlareZ123/pokemon-sims/issues/1356\n"
            "  expect(sim::EngineTestAccess::use_fss(engine),\n"
            "         \"The live Forest Seal Stone gate should bank Fire for T3\");\n"
            "  const sim::State& result = sim::EngineTestAccess::state(engine);\n"
            "  expect(result.vstar_power_used, \"Star Alchemy should be consumed\");\n"
            "  expect(has(result.hand, sim::Card::Fire),\n"
            "         \"Star Alchemy should move Fire Energy to hand\");\n"
            "}\n"
            "\n"
            "void test_split_is_symmetric_for_missing_grass() {",
            "focused Forest Seal Stone use-gate regression",
        )
        tests = replace_once(
            tests,
            "  test_treasure_takes_vstar_and_fss_takes_fire();\n"
            "  test_split_is_symmetric_for_missing_grass();",
            "  test_treasure_takes_vstar_and_fss_takes_fire();\n"
            "  test_live_fss_gate_banks_next_turn_fire();\n"
            "  test_split_is_symmetric_for_missing_grass();",
            "focused Forest Seal Stone use-gate invocation",
        )
        atomic_write(TEST_PATH, tests)

    LOCK_PATH.unlink(missing_ok=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
