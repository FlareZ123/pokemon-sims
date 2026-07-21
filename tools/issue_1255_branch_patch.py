from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PREFLIGHT = ROOT / "src/trace_engine_v2/part_empty_deck_search_override.inc"
SEARCH = ROOT / "src/trace_engine_v2/part_search_item_overrides.inc"
TEST = ROOT / "tests/issue_1255_evolution_incense_appletun_tests.cpp"


def once(text: str, old: str, new: str) -> str:
    if text.count(old) != 1:
        raise RuntimeError(f"expected one anchor, found {text.count(old)}")
    return text.replace(old, new, 1)


text = PREFLIGHT.read_text(encoding="utf-8")
text = once(text,
'''    const bool evolution_payload_searchable = might_be_unseen(Card::MegaDragonite) ||
        might_be_unseen(Card::Dragapult) || might_be_unseen(Card::GoodraVstar);''',
'''    // Appletun sv8-140 is a Stage 1 Evolution Pokémon and modeled payload:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://github.com/FlareZ123/pokemon-sims/issues/1255
    const bool evolution_payload_searchable = might_be_unseen(Card::MegaDragonite) ||
        might_be_unseen(Card::Dragapult) || might_be_unseen(Card::GoodraVstar) ||
        might_be_unseen(Card::Appletun);''')
PREFLIGHT.write_text(text, encoding="utf-8")

text = SEARCH.read_text(encoding="utf-8")
text = once(text,
'''    // Hisuian Goodra VSTAR https://api.pokemontcg.io/v2/cards/swsh11-136
    if (!found && want_payload) {
      for (const Card payload : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar}) {''',
'''    // Hisuian Goodra VSTAR https://api.pokemontcg.io/v2/cards/swsh11-136
    // Appletun sv8-140 https://api.pokemontcg.io/v2/cards/sv8-140
    // https://github.com/FlareZ123/pokemon-sims/issues/1255
    if (!found && want_payload) {
      for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                                 Card::GoodraVstar, Card::Appletun}) {''')
text = once(text,
'''    // Dipplin https://api.pokemontcg.io/v2/cards/sv6-127
    if (!found) {
      for (const Card fallback : {Card::RegidragoVstar, Card::MegaDragonite,
                                  Card::Dragapult, Card::GoodraVstar, Card::Dipplin}) {''',
'''    // Appletun https://api.pokemontcg.io/v2/cards/sv8-140
    // Dipplin https://api.pokemontcg.io/v2/cards/sv6-127
    // Forretress ex https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // https://github.com/FlareZ123/pokemon-sims/issues/1255
    if (!found) {
      for (const Card fallback : {Card::RegidragoVstar, Card::MegaDragonite,
                                  Card::Dragapult, Card::GoodraVstar,
                                  Card::Appletun, Card::Dipplin, Card::ForretressEx}) {''')
SEARCH.write_text(text, encoding="utf-8")

TEST.write_text(r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"
#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>
namespace sim { struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play(Engine& engine) { return engine.play_evolution_incense(true); }
}; }
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
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1};
  state.hand = {sim::Card::EvolutionIncense, sim::Card::Serena};
  state.deck = {sim::Card::Appletun, sim::Card::Grass};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Evolution Incense searches an Evolution Pokémon. Appletun is Stage 1 and
  // Serena is the live same-turn payload outlet:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh12-164
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
}
int main() {
  try {
    test_appletun_payload();
    std::cout << "Issue 1255 Evolution Incense tests passed\n";
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
text = report.read_text(encoding="utf-8")
text, count = re.subn(r"Simulator policy digest: `[0-9a-f]{64}`\.",
                      f"Simulator policy digest: `{digest}`.", text)
if count != 1:
    raise RuntimeError("expected one report provenance line")
report.write_text(text, encoding="utf-8")
print(digest)
