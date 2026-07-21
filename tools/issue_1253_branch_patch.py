from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PREFERRED = ROOT / "src/trace_engine_v2/part_009a.inc"
FALLBACK = ROOT / "src/trace_engine_v2/part_008b.inc"
TEST = ROOT / "tests/issue_1253_mysterious_treasure_appletun_tests.cpp"


def once(text: str, old: str, new: str) -> str:
    if text.count(old) != 1:
        raise RuntimeError(f"expected one anchor, found {text.count(old)}")
    return text.replace(old, new, 1)


text = PREFERRED.read_text(encoding="utf-8")
text = once(text,
'''    const std::array<Card, 9> targets{Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx,
                                      Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
                                      Card::DialgaGX, Card::TapuLeleGX, Card::Oricorio};''',
'''    // Appletun sv8-140 is a Dragon Pokémon and modeled Apex Dragon payload, so it
    // belongs in the preferred payload search sequence:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://github.com/FlareZ123/pokemon-sims/issues/1253
    const std::array<Card, 10> targets{Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx,
                                       Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
                                       Card::DialgaGX, Card::Appletun, Card::TapuLeleGX,
                                       Card::Oricorio};''')
PREFERRED.write_text(text, encoding="utf-8")

text = FALLBACK.read_text(encoding="utf-8")
text = once(text,
'''    // Dipplin TWM 127 is Dragon and remains a legal fallback: https://api.pokemontcg.io/v2/cards/sv6-127
    // Its legal-search status is separate from the A/S payload predicate in is_payload.
    for (const Card target : {Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx, Card::Oricorio,
                              Card::TapuLeleGX, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
                              Card::DialgaGX, Card::Dipplin}) {''',
'''    // Appletun sv8-140 and Dipplin TWM 127 are Dragon Pokémon and remain legal
    // fallbacks. Dipplin's legal-search status stays separate from is_payload:
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://api.pokemontcg.io/v2/cards/sv6-127
    // https://github.com/FlareZ123/pokemon-sims/issues/1253
    for (const Card target : {Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx, Card::Oricorio,
                              Card::TapuLeleGX, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
                              Card::DialgaGX, Card::Appletun, Card::Dipplin}) {''')
FALLBACK.write_text(text, encoding="utf-8")

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
  static bool play(Engine& engine) { return engine.play_mysterious_treasure(true); }
}; }
namespace {
bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}
void test_preferred_appletun_payload() {
  const sim::Scenario scenario{"issue-1253", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1253};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  recipe.emplace_back(sim::Card::Appletun, 1);
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Grant,
                sim::Card::Serena, sim::Card::RegidragoVstar};
  state.deck = {sim::Card::Appletun, sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Mysterious Treasure searches a Psychic or Dragon Pokémon after discarding one
  // card. Appletun sv8-140 is Dragon and a modeled Apex Dragon payload:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1253
  if (!sim::EngineTestAccess::play(engine)) {
    throw std::runtime_error("Mysterious Treasure did not play.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::Appletun) ||
      contains(after.deck, sim::Card::Appletun) ||
      !contains(after.discard, sim::Card::MysteriousTreasure) ||
      !contains(after.discard, sim::Card::Grant)) {
    throw std::runtime_error("Mysterious Treasure failed to fetch Appletun.");
  }
}
}
int main() {
  try {
    test_preferred_appletun_payload();
    std::cout << "Issue 1253 Mysterious Treasure tests passed\n";
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
