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
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"Expected one source anchor in {path}, found {count}")
    atomic_write(path, text.replace(old, new, 1))


def main() -> None:
    empty_deck = ROOT / "src/trace_engine_v2/part_empty_deck_search_override.inc"
    replace_once(
        empty_deck,
        """    for (const Card payload : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar}) {
""",
        """    // Evolution Incense can search Appletun because it is a Stage 1 Pokémon:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://github.com/FlareZ123/pokemon-sims/issues/1255
    for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                               Card::GoodraVstar, Card::Appletun}) {
""",
    )
    replace_once(
        empty_deck,
        """    const bool evolution_payload_searchable = might_be_unseen(Card::MegaDragonite) ||
        might_be_unseen(Card::Dragapult) || might_be_unseen(Card::GoodraVstar);
""",
        """    // Appletun sv8-140 is a Stage 1 Evolution Pokémon and modeled payload:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://github.com/FlareZ123/pokemon-sims/issues/1255
    const bool evolution_payload_searchable = might_be_unseen(Card::MegaDragonite) ||
        might_be_unseen(Card::Dragapult) || might_be_unseen(Card::GoodraVstar) ||
        might_be_unseen(Card::Appletun);
""",
    )

    search_items = ROOT / "src/trace_engine_v2/part_search_item_overrides.inc"
    replace_once(
        search_items,
        """    if (!found && want_payload) {
      for (const Card payload : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar}) {
""",
        """    // Appletun is a Stage 1 Evolution Pokémon and modeled Apex Dragon payload:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/1255
    if (!found && want_payload) {
      for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                                 Card::GoodraVstar, Card::Appletun}) {
""",
    )
    replace_once(
        search_items,
        """      for (const Card fallback : {Card::RegidragoVstar, Card::MegaDragonite,
                                  Card::Dragapult, Card::GoodraVstar, Card::Dipplin}) {
""",
        """      // Evolution Incense searches every modeled Evolution Pokémon, including
      // Appletun and Forretress ex:
      // https://api.pokemontcg.io/v2/cards/swsh1-163
      // https://api.pokemontcg.io/v2/cards/sv8-140
      // https://api.pokemontcg.io/v2/cards/sv4pt5-2
      // https://github.com/FlareZ123/pokemon-sims/issues/1255
      for (const Card fallback : {Card::RegidragoVstar, Card::MegaDragonite,
                                  Card::Dragapult, Card::GoodraVstar,
                                  Card::Appletun, Card::Dipplin,
                                  Card::ForretressEx}) {
""",
    )

    legacy = ROOT / "src/trace_engine_v2/part_013_legacy_star_override.inc"
    replace_once(
        legacy,
        """        for (const Card candidate :
             {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar}) {
""",
        """        // Evolution Incense can fetch Appletun after Legacy Star recovers the
        // two-card search-and-discard bridge:
        // https://api.pokemontcg.io/v2/cards/swsh12-136
        // https://api.pokemontcg.io/v2/cards/swsh1-163
        // https://api.pokemontcg.io/v2/cards/sv8-140
        // https://github.com/FlareZ123/pokemon-sims/issues/1255
        for (const Card candidate :
             {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
              Card::Appletun}) {
""",
    )

    combo = ROOT / "src/trace_engine_v2/part_forretress_ex_combo.inc"
    replace_once(
        combo,
        """    for (const Card fallback : {Card::RegidragoVstar,
                                Card::MegaDragonite, Card::Dragapult,
                                Card::GoodraVstar}) {
""",
        """    // Evolution Incense searches any Evolution Pokémon, including Appletun:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://github.com/FlareZ123/pokemon-sims/issues/1255
    for (const Card fallback : {Card::RegidragoVstar,
                                Card::MegaDragonite, Card::Dragapult,
                                Card::GoodraVstar, Card::Appletun}) {
""",
    )

    primary = ROOT / "tests/issue_1255_evolution_incense_appletun_tests.cpp"
    atomic_write(primary, r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play(Engine& engine) { return engine.play_evolution_incense(true); }
};
}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_appletun_payload() {
  const sim::Scenario scenario{"issue-1255", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1255};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  recipe.emplace_back(sim::Card::Appletun, 1);
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::EvolutionIncense, sim::Card::Serena};
  state.deck = {sim::Card::Appletun, sim::Card::Grass};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Evolution Incense searches an Evolution Pokémon. Appletun is Stage 1 and
  // Serena is the live same-turn payload outlet:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1255
  if (!sim::EngineTestAccess::play(engine)) {
    throw std::runtime_error("Evolution Incense did not play.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::Appletun) ||
      contains(after.deck, sim::Card::Appletun) ||
      !contains(after.discard, sim::Card::EvolutionIncense)) {
    throw std::runtime_error("Evolution Incense failed the Appletun route.");
  }
}

}  // namespace

int main() {
  test_appletun_payload();
  std::cout << "Issue 1255 Evolution Incense tests passed.\n";
  return 0;
}
''')

    continuation = ROOT / "tests/issue_1255_evolution_incense_appletun_continuation_tests.cpp"
    atomic_write(continuation, r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool live_incense_outlet(Engine& engine) {
    return engine.evolution_incense_has_live_post_payload_outlet();
  }
  static bool use_legacy_star(Engine& engine) { return engine.use_legacy_star(); }
  static bool play_forretress_incense(Engine& engine) {
    return engine.play_evolution_incense_for_forretress();
  }
};
}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::DeckRecipe continuation_recipe() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  recipe.emplace_back(sim::Card::Pineco, 1);
  recipe.emplace_back(sim::Card::ForretressEx, 1);
  return recipe;
}

struct Fixture {
  sim::Scenario scenario{"issue-1255/continuations", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{continuation_recipe()};
  std::mt19937_64 rng{12550};
  sim::Engine engine{scenario, recipe, rng};
};

sim::Pokemon ready_vstar() {
  return sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
}

void test_post_incense_outlet_projects_appletun() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = ready_vstar();
  state.hand = {sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::Appletun, sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Evolution Incense removes Appletun, then Mysterious Treasure retains the
  // remaining Dragon target while discarding Appletun for Apex Dragon:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1255
  if (!sim::EngineTestAccess::live_incense_outlet(fixture.engine)) {
    throw std::runtime_error("Post-Incense outlet planner omitted Appletun.");
  }
}

void test_legacy_star_recovers_appletun_incense_bridge() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = ready_vstar();
  state.discard = {sim::Card::EvolutionIncense, sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::Appletun, sim::Card::RegidragoV,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Legacy Star discards the seven top Grass Energy, then may recover Evolution
  // Incense plus Mysterious Treasure for the legal Appletun payload bridge:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://github.com/FlareZ123/pokemon-sims/issues/1255
  if (!sim::EngineTestAccess::use_legacy_star(fixture.engine)) {
    throw std::runtime_error("Legacy Star rejected the Appletun Incense bridge.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::EvolutionIncense) ||
      !contains(after.hand, sim::Card::MysteriousTreasure)) {
    throw std::runtime_error("Legacy Star failed to recover the Appletun bridge.");
  }
}

void test_forretress_incense_fallback_takes_appletun() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = ready_vstar();
  state.bench = {sim::Pokemon{sim::Card::Pineco, 1, 0, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::EvolutionIncense};
  state.deck = {sim::Card::Appletun};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // The Forretress route uses Evolution Incense's exact Evolution target class:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://github.com/FlareZ123/pokemon-sims/issues/1255
  if (!sim::EngineTestAccess::play_forretress_incense(fixture.engine)) {
    throw std::runtime_error("Forretress Incense route rejected Appletun fallback.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::Appletun) ||
      contains(after.deck, sim::Card::Appletun)) {
    throw std::runtime_error("Forretress Incense fallback omitted Appletun.");
  }
}

}  // namespace

int main() {
  test_post_incense_outlet_projects_appletun();
  test_legacy_star_recovers_appletun_incense_bridge();
  test_forretress_incense_fallback_takes_appletun();
  std::cout << "Issue 1255 Evolution Incense continuation tests passed.\n";
  return 0;
}
''')

    (ROOT / ".github/workflows/apply-issue-1255-v2.yml").unlink(missing_ok=True)
    Path(__file__).unlink(missing_ok=True)


if __name__ == "__main__":
    main()
