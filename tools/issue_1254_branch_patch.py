from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SEARCH = ROOT / "src/trace_engine_v2/part_search_item_overrides.inc"
FALLBACK = ROOT / "src/trace_engine_v2/part_009a.inc"
TEST = ROOT / "tests/issue_1254_ultra_ball_appletun_tests.cpp"


def once(text: str, old: str, new: str) -> str:
    if text.count(old) != 1:
        raise RuntimeError(f"expected one anchor, found {text.count(old)}")
    return text.replace(old, new, 1)


text = SEARCH.read_text(encoding="utf-8")
text = once(text,
'''    const bool payload_might_be_searchable = might_be_unseen(Card::MegaDragonite) ||
                                               might_be_unseen(Card::Dragapult) ||
                                               might_be_unseen(Card::GoodraVstar) ||
                                               might_be_unseen(Card::DialgaGX);''',
'''    // Ultra Ball may search any Pokémon. Appletun sv8-140 is both modeled and a
    // strict-JIT payload, so it belongs in the K0 payload-availability gate:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://github.com/FlareZ123/pokemon-sims/issues/1254
    const bool payload_might_be_searchable = might_be_unseen(Card::MegaDragonite) ||
                                               might_be_unseen(Card::Dragapult) ||
                                               might_be_unseen(Card::GoodraVstar) ||
                                               might_be_unseen(Card::DialgaGX) ||
                                               might_be_unseen(Card::Appletun);''')
text = once(text,
'''    const std::array<Card, 9> targets{Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx, Card::Oricorio,
                                      Card::TapuLeleGX, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
                                      Card::DialgaGX};''',
'''    // Appletun is a legal Pokémon target and modeled payload:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://github.com/FlareZ123/pokemon-sims/issues/1254
    const std::array<Card, 10> targets{Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx, Card::Oricorio,
                                       Card::TapuLeleGX, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
                                       Card::DialgaGX, Card::Appletun};''')
SEARCH.write_text(text, encoding="utf-8")

text = FALLBACK.read_text(encoding="utf-8")
text = once(text,
'''  Card fallback_ultra_ball_target_after_search_started() const {
    for (const Card target : {Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx, Card::Oricorio,
                              Card::TapuLeleGX, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
                              Card::DialgaGX, Card::MawileGX, Card::Dipplin}) {''',
'''  Card fallback_ultra_ball_target_after_search_started() const {
    // Ultra Ball searches any Pokémon, so the K1 fallback covers the complete
    // modeled universe, including the Appletun and Pineco package:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://api.pokemontcg.io/v2/cards/sv4pt5-1
    // https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // https://github.com/FlareZ123/pokemon-sims/issues/1254
    for (const Card target : {Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx, Card::Oricorio,
                              Card::TapuLeleGX, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
                              Card::DialgaGX, Card::MawileGX, Card::Dipplin, Card::Appletun,
                              Card::Pineco, Card::ForretressEx}) {''')
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
  static bool play(Engine& engine) { return engine.play_ultra_ball(true); }
}; }
namespace {
bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}
void test_appletun_payload() {
  const sim::Scenario scenario{"issue-1254", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1254};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  recipe.emplace_back(sim::Card::Appletun, 1);
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1};
  state.hand = {sim::Card::UltraBall, sim::Card::Grant, sim::Card::WishfulBaton,
                sim::Card::Serena, sim::Card::RegidragoVstar};
  state.deck = {sim::Card::Appletun, sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Ultra Ball discards two other cards and searches any Pokémon. Appletun is a
  // legal modeled payload, while Grant and Wishful Baton are the low-DCI costs:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1254
  if (!sim::EngineTestAccess::play(engine)) {
    throw std::runtime_error("Ultra Ball did not play.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::Appletun) ||
      contains(after.deck, sim::Card::Appletun) ||
      !contains(after.discard, sim::Card::UltraBall) ||
      !contains(after.discard, sim::Card::Grant) ||
      !contains(after.discard, sim::Card::WishfulBaton)) {
    throw std::runtime_error("Ultra Ball failed the Appletun route.");
  }
}
}
int main() {
  try {
    test_appletun_payload();
    std::cout << "Issue 1254 Ultra Ball tests passed\n";
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
