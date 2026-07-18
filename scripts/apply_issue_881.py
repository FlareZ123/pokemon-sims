from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LOCK_PATH = ROOT / ".issue-881.lock"


def atomic_write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=f".{path.name}.", dir=path.parent, text=True
    )
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="") as handle:
            handle.write(content)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temporary_name, path)
    except BaseException:
        try:
            os.unlink(temporary_name)
        except FileNotFoundError:
            pass
        raise


def replace_once(path: Path, old: str, new: str) -> None:
    current = path.read_text(encoding="utf-8")
    if new in current:
        return
    count = current.count(old)
    if count != 1:
        raise RuntimeError(f"Expected one replacement anchor in {path}, found {count}.")
    atomic_write(path, current.replace(old, new, 1))


def insert_after_once(path: Path, anchor: str, addition: str) -> None:
    current = path.read_text(encoding="utf-8")
    if addition.strip() in current:
        return
    count = current.count(anchor)
    if count != 1:
        raise RuntimeError(f"Expected one insertion anchor in {path}, found {count}.")
    atomic_write(path, current.replace(anchor, anchor + addition, 1))


TEST_SOURCE = '#define REGIDRAGO_SIM_NO_MAIN\n#include "../src/regidrago_sim.cpp"\n\n#include <algorithm>\n#include <iostream>\n#include <random>\n#include <stdexcept>\n#include <utility>\n#include <vector>\n\nnamespace sim {\nstruct EngineTestAccess {\n  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }\n  static const State& state(const Engine& engine) { return engine.state_; }\n  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }\n  static bool play_blender(Engine& engine) { return engine.play_brilliant_blender(); }\n  static bool play_burnet(Engine& engine) { return engine.play_professor_burnet(); }\n};\n}  // namespace sim\n\nnamespace {\nbool contains(const std::vector<sim::Card>& cards, sim::Card card) {\n  return std::find(cards.begin(), cards.end(), card) != cards.end();\n}\n\nstruct Fixture {\n  sim::Scenario scenario;\n  sim::DeckRecipe recipe;\n  std::mt19937_64 rng;\n  sim::Engine engine;\n\n  Fixture(const std::string& label, sim::DciProfile dci, std::uint64_t seed)\n      : scenario{label, dci, sim::LockMode::None, false, 4},\n        recipe(sim::baseline_recipe()),\n        rng(seed),\n        engine(scenario, recipe, rng) {}\n};\n\nsim::State blender_state() {\n  sim::State state;\n  state.turn = 2;\n  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1, sim::Tool::None};\n  state.hand = {sim::Card::BrilliantBlender, sim::Card::TateLiza, sim::Card::Grass};\n  state.deck = {sim::Card::MegaDragonite, sim::Card::ErikasInvitation,\n                sim::Card::Arven, sim::Card::QuickBall,\n                sim::Card::Grass, sim::Card::Fire};\n  return state;\n}\n\nsim::State burnet_state() {\n  sim::State state;\n  state.turn = 2;\n  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0, sim::Tool::None};\n  state.hand = {sim::Card::ProfessorBurnet};\n  state.deck = {sim::Card::MegaDragonite, sim::Card::ErikasInvitation,\n                sim::Card::Arven, sim::Card::QuickBall,\n                sim::Card::Grass, sim::Card::Fire};\n  return state;\n}\n\nvoid require_erika_thinned(const sim::State& state, const char* action) {\n  if (contains(state.deck, sim::Card::ErikasInvitation) ||\n      !contains(state.discard, sim::Card::ErikasInvitation)) {\n    throw std::runtime_error(std::string(action) + " should discard Erika.");\n  }\n  if (!contains(state.deck, sim::Card::Arven) ||\n      !contains(state.deck, sim::Card::QuickBall)) {\n    throw std::runtime_error(std::string(action) + " should preserve live connectors.");\n  }\n}\n\nvoid test_blender() {\n  Fixture fixture{"issue-881-blender", sim::DciProfile::NoDiscardControl, 88101};\n  sim::EngineTestAccess::set_state(fixture.engine, blender_state());\n  sim::EngineTestAccess::set_deck_seen(fixture.engine);\n\n  // Brilliant Blender may discard up to five deck cards. Spare selections before a\n  // legal Tate & Liza refresh remove the opponent-dependent inert Erika singleton:\n  // https://api.pokemontcg.io/v2/cards/sv8-164\n  // https://api.pokemontcg.io/v2/cards/sv3pt5-160\n  // https://api.pokemontcg.io/v2/cards/sm7-148\n  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation\n  // https://github.com/FlareZ123/pokemon-sims/issues/881\n  if (!sim::EngineTestAccess::play_blender(fixture.engine)) {\n    throw std::runtime_error("Brilliant Blender should play.");\n  }\n  require_erika_thinned(sim::EngineTestAccess::state(fixture.engine), "Blender");\n}\n\nvoid test_burnet() {\n  Fixture fixture{"issue-881-burnet", sim::DciProfile::NoDiscardControl, 88102};\n  sim::EngineTestAccess::set_state(fixture.engine, burnet_state());\n  sim::EngineTestAccess::set_deck_seen(fixture.engine);\n\n  // Burnet\'s second legal discard changes Celestial Roar\'s Energy hit from 90% to\n  // 100% in this exact state while preserving live connectors:\n  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26\n  // https://api.pokemontcg.io/v2/cards/swsh12-135\n  // https://api.pokemontcg.io/v2/cards/sv3pt5-160\n  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities\n  // https://github.com/FlareZ123/pokemon-sims/issues/881\n  if (!sim::EngineTestAccess::play_burnet(fixture.engine)) {\n    throw std::runtime_error("Professor Burnet should play.");\n  }\n  require_erika_thinned(sim::EngineTestAccess::state(fixture.engine), "Burnet");\n}\n\nvoid test_profile_boundaries() {\n  for (sim::DciProfile dci :\n       {sim::DciProfile::StrictJit, sim::DciProfile::MatchupFlexJit}) {\n    Fixture fixture{"issue-881-control", dci, 88103};\n    sim::EngineTestAccess::set_state(fixture.engine, blender_state());\n    sim::EngineTestAccess::set_deck_seen(fixture.engine);\n\n    // This thinning remains exclusive to no-discard-control:\n    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation\n    // https://github.com/FlareZ123/pokemon-sims/issues/881\n    if (!sim::EngineTestAccess::play_blender(fixture.engine)) {\n      throw std::runtime_error("Blender should remain legal in the control.");\n    }\n    const sim::State& after = sim::EngineTestAccess::state(fixture.engine);\n    if (!contains(after.deck, sim::Card::ErikasInvitation) ||\n        contains(after.discard, sim::Card::ErikasInvitation)) {\n      throw std::runtime_error("Other DCI profiles must retain Erika.");\n    }\n  }\n}\n}  // namespace\n\nint main() {\n  try {\n    test_blender();\n    test_burnet();\n    test_profile_boundaries();\n    std::cout << "Erika deck-thinning tests passed\\n";\n    return 0;\n  } catch (const std::exception& error) {\n    std::cerr << error.what() << \'\\n\';\n    return 1;\n  }\n}\n'


def apply() -> None:
    blender_path = ROOT / "src/trace_engine_v2/part_010_blender_thinning_override.inc"
    old_blender = """    if (thin_before_refresh) {
      for (const Card card : {Card::PathToPeak, Card::MawileGX, Card::Guzma,
"""
    new_blender = """    if (thin_before_refresh) {
      // Erika's Invitation replaced retired Mawile-GX in the canonical recipe and is
      // opponent-dependent in this goldfish model. Use spare legal Blender capacity
      // to remove it before the same-turn refresh:
      // https://api.pokemontcg.io/v2/cards/sv8-164
      // https://api.pokemontcg.io/v2/cards/sv3pt5-160
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CARD_AUDIT.md#erikas-invitation
      // https://github.com/FlareZ123/pokemon-sims/issues/881
      for (const Card card : {Card::PathToPeak, Card::ErikasInvitation, Card::Guzma,
"""
    replace_once(blender_path, old_blender, new_blender)

    burnet_path = ROOT / "src/trace_engine_v2/part_011_burnet_thinning_override.inc"
    old_burnet = """    if (discarded.size() < 2U && active_can_use_celestial_roar &&
        needed_energy_remains && state_.deck.size() > 3U) {
      for (const Card card : {Card::PathToPeak, Card::MawileGX, Card::Guzma,
"""
    new_burnet = """    if (discarded.size() < 2U && active_can_use_celestial_roar &&
        needed_energy_remains && state_.deck.size() > 3U) {
      // Burnet may discard up to two cards. After the Dragon payload, remove the
      // modeled inert Erika singleton to improve Celestial Roar's top-three Energy
      // hit without consuming a live connector:
      // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
      // https://api.pokemontcg.io/v2/cards/swsh12-135
      // https://api.pokemontcg.io/v2/cards/sv3pt5-160
      // https://github.com/FlareZ123/pokemon-sims/issues/881
      for (const Card card : {Card::PathToPeak, Card::ErikasInvitation, Card::Guzma,
"""
    replace_once(burnet_path, old_burnet, new_burnet)

    test_path = ROOT / "tests/erika_deck_thinning_tests.cpp"
    if not test_path.exists() or test_path.read_text(encoding="utf-8") != TEST_SOURCE:
        atomic_write(test_path, TEST_SOURCE)

    cmake_path = ROOT / "CMakeLists.txt"
    target_anchor = (
        "add_executable(regidrago_blender_fss_tate_hold_tests tests/blender_fss_tate_hold_tests.cpp)\n"
        "target_compile_options(regidrago_blender_fss_tate_hold_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)\n"
    )
    target_addition = (
        "\n# Erika deck-thinning regression: https://github.com/FlareZ123/pokemon-sims/issues/881\n"
        "add_executable(regidrago_erika_deck_thinning_tests tests/erika_deck_thinning_tests.cpp)\n"
        "target_compile_options(regidrago_erika_deck_thinning_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)\n"
    )
    insert_after_once(cmake_path, target_anchor, target_addition)

    test_anchor = (
        "add_test(NAME regidrago_blender_fss_tate_hold COMMAND regidrago_blender_fss_tate_hold_tests)\n"
    )
    test_addition = (
        "add_test(NAME regidrago_erika_deck_thinning COMMAND regidrago_erika_deck_thinning_tests)\n"
    )
    insert_after_once(cmake_path, test_anchor, test_addition)


if __name__ == "__main__":
    LOCK_PATH.touch(exist_ok=True)
    with LOCK_PATH.open("r+", encoding="utf-8") as lock_handle:
        fcntl.flock(lock_handle.fileno(), fcntl.LOCK_EX)
        apply()
