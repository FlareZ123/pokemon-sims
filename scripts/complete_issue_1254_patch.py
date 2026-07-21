from __future__ import annotations

import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def atomic_write(path: Path, text: str) -> None:
    descriptor, temporary_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as handle:
            handle.write(text)
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
    text = path.read_text(encoding="utf-8")
    if text.count(old) != 1:
        raise RuntimeError(f"Expected one source anchor in {path}, found {text.count(old)}")
    atomic_write(path, text.replace(old, new, 1))


def main() -> None:
    empty_deck = ROOT / "src/trace_engine_v2/part_empty_deck_search_override.inc"
    replace_once(
        empty_deck,
        """    for (const Card payload : {Card::MegaDragonite, Card::Dragapult,\n                               Card::GoodraVstar, Card::DialgaGX}) {\n""",
        """    // Appletun is a modeled Dragon payload that Ultra Ball may search. The\n    // projected second Ultra Ball must remove it before proving another Pokémon target:\n    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146\n    // https://api.pokemontcg.io/v2/cards/sv8-140\n    // https://github.com/FlareZ123/pokemon-sims/issues/1254\n    for (const Card payload : {Card::MegaDragonite, Card::Dragapult,\n                               Card::GoodraVstar, Card::DialgaGX,\n                               Card::Appletun}) {\n""",
    )

    outlet = ROOT / "src/trace_engine_v2/part_012_override.inc"
    replace_once(
        outlet,
        """        for (const Card payload : {Card::MegaDragonite, Card::Dragapult,\n                                   Card::GoodraVstar, Card::DialgaGX}) {\n""",
        """        // Appletun is a legal Ultra Ball target and modeled Apex Dragon payload:\n        // https://api.pokemontcg.io/v2/cards/swsh12pt5-146\n        // https://api.pokemontcg.io/v2/cards/sv8-140\n        // https://api.pokemontcg.io/v2/cards/swsh12-136\n        // https://github.com/FlareZ123/pokemon-sims/issues/1254\n        for (const Card payload : {Card::MegaDragonite, Card::Dragapult,\n                                   Card::GoodraVstar, Card::DialgaGX,\n                                   Card::Appletun}) {\n""",
    )

    combo = ROOT / "src/trace_engine_v2/part_forretress_ex_combo.inc"
    replace_once(
        combo,
        """                            Card::DialgaGX, Card::MegaDragonite,\n                            Card::Dragapult, Card::GoodraVstar,\n                            Card::Dipplin}) {\n""",
        """                            Card::DialgaGX, Card::MegaDragonite,\n                            Card::Dragapult, Card::GoodraVstar,\n                            Card::Dipplin, Card::Appletun}) {\n      // Ultra Ball searches any Pokémon, including Appletun:\n      // https://api.pokemontcg.io/v2/cards/swsh12pt5-146\n      // https://api.pokemontcg.io/v2/cards/sv8-140\n      // https://github.com/FlareZ123/pokemon-sims/issues/1254\n""",
    )

    test = ROOT / "tests/issue_1254_ultra_ball_appletun_continuation_tests.cpp"
    atomic_write(test, r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static bool second_ultra_has_target(const Engine& engine) {
    return engine.second_ultra_ball_has_post_payload_target();
  }
  static bool payable_outlet(Engine& engine, const Card excluded) {
    return engine.has_payable_payload_outlet_after_costed_search(excluded);
  }
  static Card combo_fallback(const Engine& engine) {
    return engine.fallback_combo_pokemon_after_search_started();
  }
};
}  // namespace sim

namespace {

sim::Engine make_engine(std::mt19937_64& rng) {
  const sim::Scenario scenario{"issue-1254/continuations", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  return sim::Engine(scenario, sim::pineco_recipe(), rng);
}

void test_second_ultra_projects_appletun_removal() {
  std::mt19937_64 rng{12540};
  sim::Engine engine = make_engine(rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1};
  state.deck = {sim::Card::Appletun, sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Ultra Ball searches any Pokémon. Appletun is the projected first payload,
  // and Regidrago V remains as the legal target for the second Ultra Ball:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://github.com/FlareZ123/pokemon-sims/issues/1254
  if (!sim::EngineTestAccess::second_ultra_has_target(engine)) {
    throw std::runtime_error("Second Ultra Ball omitted Appletun from its K1 projection.");
  }
}

void test_costed_search_preserves_appletun_outlet() {
  std::mt19937_64 rng{12541};
  sim::Engine engine = make_engine(rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1};
  state.hand = {sim::Card::QuickBall, sim::Card::Grant,
                sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::Appletun, sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Quick Ball can pay Grant, then the surviving Mysterious Treasure can discard
  // the fetched Appletun and search the remaining Dragon Pokémon:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1254
  if (!sim::EngineTestAccess::payable_outlet(engine, sim::Card::QuickBall)) {
    throw std::runtime_error("Costed-search planner omitted the Appletun continuation.");
  }
}

void test_forretress_ultra_fallback_can_take_appletun() {
  std::mt19937_64 rng{12542};
  sim::Engine engine = make_engine(rng);
  sim::State state;
  state.turn = 2;
  state.deck = {sim::Card::Appletun};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // The Forretress route still uses Ultra Ball's exact any-Pokémon target class:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://github.com/FlareZ123/pokemon-sims/issues/1254
  if (sim::EngineTestAccess::combo_fallback(engine) != sim::Card::Appletun) {
    throw std::runtime_error("Forretress Ultra Ball fallback omitted Appletun.");
  }
}

}  // namespace

int main() {
  test_second_ultra_projects_appletun_removal();
  test_costed_search_preserves_appletun_outlet();
  test_forretress_ultra_fallback_can_take_appletun();
  std::cout << "Issue 1254 Ultra Ball continuation tests passed.\n";
  return 0;
}
''')


if __name__ == "__main__":
    main()
