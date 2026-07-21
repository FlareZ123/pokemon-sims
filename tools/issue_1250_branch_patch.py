from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src/trace_engine_v2/part_pokemon_communication.inc"
TEST = ROOT / "tests/issue_1250_pokemon_communication_forretress_tests.cpp"


def replace_once(text: str, old: str, new: str) -> str:
    if text.count(old) != 1:
        raise RuntimeError(f"expected exactly one source anchor, found {text.count(old)}")
    return text.replace(old, new, 1)


source = SOURCE.read_text(encoding="utf-8")
source = replace_once(
    source,
    '''  static constexpr std::array<Card, 11> pokemon_communication_pokemon() {
    return {Card::RegidragoV, Card::RegidragoVstar, Card::Dragapult,
            Card::MegaDragonite, Card::DialgaGX, Card::GoodraVstar,
            Card::TapuLeleGX, Card::LatiasEx, Card::MawileGX,
            Card::Oricorio, Card::Dipplin};
  }''',
    '''  static constexpr std::array<Card, 14> pokemon_communication_pokemon() {
    // Pokémon Communication may return and search any Pokémon. Keep this list
    // aligned with the complete modeled Pokémon universe:
    // https://api.pokemontcg.io/v2/cards/sm9-152
    // Appletun: https://api.pokemontcg.io/v2/cards/sv8-140
    // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
    // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
    return {Card::RegidragoV, Card::RegidragoVstar, Card::Dragapult,
            Card::MegaDragonite, Card::DialgaGX, Card::GoodraVstar,
            Card::TapuLeleGX, Card::LatiasEx, Card::MawileGX,
            Card::Oricorio, Card::Dipplin, Card::Appletun,
            Card::Pineco, Card::ForretressEx};
  }''',
)
source = replace_once(
    source,
    '''    if (card == Card::RegidragoVstar) {
      return need_vstar() && !has_vstar() ? 900 : 30;
    }''',
    '''    if (card == Card::RegidragoVstar) {
      // A singleton held VSTAR is the immediate evolution half of the same setup
      // graph. Do not return it merely because hand-aware `need_vstar()` is already
      // satisfied while Pokémon Communication is fetching the Pineco line:
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // https://github.com/FlareZ123/pokemon-sims/issues/1250
      if (!has_vstar() &&
          (needs_pineco_connector() || needs_forretress_connector())) {
        return 900;
      }
      return need_vstar() && !has_vstar() ? 900 : 30;
    }''',
)
source = replace_once(
    source,
    '''    if (card == Card::Oricorio) {
      return !in_play(Card::Oricorio) && needs_oricorio_connector() ? 900 : 40;
    }
    if (is_payload(card)) {''',
    '''    if (card == Card::Oricorio) {
      return !in_play(Card::Oricorio) && needs_oricorio_connector() ? 900 : 40;
    }
    if (card == Card::Pineco) {
      return needs_pineco_connector() ? 900 : 45;
    }
    if (card == Card::ForretressEx) {
      return needs_forretress_connector() ? 900 : 45;
    }
    if (card == Card::Appletun &&
        (needs_pineco_connector() || needs_forretress_connector())) {
      // Appletun is the lower-DCI exchange when the singleton Regidrago VSTAR and
      // Pineco line are both immediately live. Pokémon Communication puts it back
      // into the deck, where later Dragon search can recover it:
      // https://api.pokemontcg.io/v2/cards/sm9-152
      // https://api.pokemontcg.io/v2/cards/sv8-140
      // https://github.com/FlareZ123/pokemon-sims/issues/1250
      return 45;
    }
    if (is_payload(card)) {''',
)
source = replace_once(
    source,
    '''    if (need_regi() && bench_space() > 0 && hand_count(Card::RegidragoV) == 0) {
      add_target(Card::RegidragoV);
    }''',
    '''    // The Forretress package delegates its live connector axes to Pokémon
    // Communication before weaker search Items:
    // https://api.pokemontcg.io/v2/cards/sm9-152
    // https://api.pokemontcg.io/v2/cards/sv4pt5-1
    // https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // https://github.com/FlareZ123/pokemon-sims/issues/1250
    if (needs_pineco_connector() && hand_count(Card::Pineco) == 0) {
      add_target(Card::Pineco);
    }
    if (needs_forretress_connector() && hand_count(Card::ForretressEx) == 0) {
      add_target(Card::ForretressEx);
    }
    if (need_regi() && bench_space() > 0 && hand_count(Card::RegidragoV) == 0) {
      add_target(Card::RegidragoV);
    }''',
)
source = source.replace(
    '''      for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                                 Card::GoodraVstar, Card::DialgaGX}) {''',
    '''      for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                                 Card::GoodraVstar, Card::DialgaGX,
                                 Card::Appletun}) {''',
)
if source.count("Card::GoodraVstar, Card::DialgaGX,\n                                 Card::Appletun})") != 2:
    raise RuntimeError("expected both Pokémon Communication payload enumerations")
source = replace_once(
    source,
    '''    if (need_regi() && bench_space() > 0 && present(Card::RegidragoV)) {
      return Card::RegidragoV;
    }''',
    '''    if (needs_pineco_connector() && present(Card::Pineco)) {
      return Card::Pineco;
    }
    if (needs_forretress_connector() && present(Card::ForretressEx)) {
      return Card::ForretressEx;
    }
    if (need_regi() && bench_space() > 0 && present(Card::RegidragoV)) {
      return Card::RegidragoV;
    }''',
)
SOURCE.write_text(source, encoding="utf-8")

TEST.write_text(r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool needs_pineco_connector(const Engine& engine) {
    return engine.needs_pineco_connector();
  }
  static bool needs_forretress_connector(const Engine& engine) {
    return engine.needs_forretress_connector();
  }
  static bool run_forretress_combo_search_step(Engine& engine) {
    return engine.run_forretress_combo_search_step(false);
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::DeckRecipe variant_recipe() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  recipe.emplace_back(sim::Card::PokemonCommunication, 1);
  recipe.emplace_back(sim::Card::Pineco, 1);
  recipe.emplace_back(sim::Card::ForretressEx, 1);
  recipe.emplace_back(sim::Card::Appletun, 1);
  return recipe;
}

void expect_exchange(const bool pineco_target) {
  const sim::Scenario scenario{pineco_target ? "issue-1250-pineco" : "issue-1250-forretress",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{pineco_target ? 1250U : 12501U};
  const sim::DeckRecipe recipe = variant_recipe();
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  if (!pineco_target) state.bench = {sim::Pokemon{sim::Card::Pineco, 1}};
  state.hand = {sim::Card::PokemonCommunication, sim::Card::Appletun,
                sim::Card::RegidragoVstar};
  const sim::Card target = pineco_target ? sim::Card::Pineco : sim::Card::ForretressEx;
  state.deck = {target, sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Pokémon Communication accepts any Pokémon from hand and searches any
  // Pokémon. Appletun is a legal exchange while the missing Pineco line is the
  // live connector axis; the singleton held Regidrago VSTAR remains protected:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1250
  const bool axis_live = pineco_target
      ? sim::EngineTestAccess::needs_pineco_connector(engine)
      : sim::EngineTestAccess::needs_forretress_connector(engine);
  if (!axis_live || !sim::EngineTestAccess::run_forretress_combo_search_step(engine)) {
    throw std::runtime_error("Pokémon Communication did not complete the live Forretress connector route.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, target) || contains(after.hand, sim::Card::Appletun) ||
      !contains(after.deck, sim::Card::Appletun) ||
      !contains(after.hand, sim::Card::RegidragoVstar) ||
      !contains(after.discard, sim::Card::PokemonCommunication)) {
    throw std::runtime_error("The Appletun exchange resolved incorrectly.");
  }
}

}  // namespace

int main() {
  try {
    expect_exchange(true);
    expect_exchange(false);
    std::cout << "Issue 1250 Pokémon Communication tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
''', encoding="utf-8")

from scripts.baseline_provenance import simulator_policy_source_digest

digest = simulator_policy_source_digest(ROOT)
for relative in ("results/baseline_manifest.json", "results/multi_deck_manifest.json"):
    path = ROOT / relative
    data = json.loads(path.read_text(encoding="utf-8"))
    data["simulator_policy_source_sha256"] = digest
    path.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")

report = ROOT / "docs/MULTI_DECK_REPORT.md"
report_text = report.read_text(encoding="utf-8")
report_text, count = re.subn(
    r"Simulator policy digest: `[0-9a-f]{64}`\.",
    f"Simulator policy digest: `{digest}`.",
    report_text,
)
if count != 1:
    raise RuntimeError("expected one report provenance line")
report.write_text(report_text, encoding="utf-8")
print(digest)
