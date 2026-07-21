from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src/trace_engine_v2/part_prize_payload_outlet_override.inc"
TEST = ROOT / "tests/issue_1252_heavy_ball_pineco_tests.cpp"


def replace_once(text: str, old: str, new: str) -> str:
    if text.count(old) != 1:
        raise RuntimeError(f"expected one source anchor, found {text.count(old)}")
    return text.replace(old, new, 1)


source = SOURCE.read_text(encoding="utf-8")
source = replace_once(
    source,
    '''    // Hisuian Heavy Ball reveals the Prize cards before its optional Basic recovery.
    // The revealed information therefore permits an exact K1 continuation check:
    // https://api.pokemontcg.io/v2/cards/swsh10-146
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    remove_one(state_.hand, Card::HisuianHeavyBall);
    prizes_revealed_ = true;
''',
    '''    // Hisuian Heavy Ball reveals the Prize cards before its optional Basic recovery.
    // Preserve the live Pineco connector decision made at K0, then use the exact
    // revealed Prize contents to choose that Basic when it is present:
    // https://api.pokemontcg.io/v2/cards/swsh10-146
    // https://api.pokemontcg.io/v2/cards/sv4pt5-1
    // https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    // https://github.com/FlareZ123/pokemon-sims/issues/1252
    const bool recover_prized_pineco = needs_pineco_connector();
    remove_one(state_.hand, Card::HisuianHeavyBall);
    prizes_revealed_ = true;
''',
)
source = replace_once(
    source,
    '''    if (need_regi() && can_bench_recovered_basic && prized(Card::RegidragoV)) {
      selected = Card::RegidragoV;
    } else if (oricorio_preserves_burnet && prized(Card::Oricorio)) {
''',
    '''    if (recover_prized_pineco && can_bench_recovered_basic && prized(Card::Pineco)) {
      selected = Card::Pineco;
    } else if (need_regi() && can_bench_recovered_basic && prized(Card::RegidragoV)) {
      selected = Card::RegidragoV;
    } else if (oricorio_preserves_burnet && prized(Card::Oricorio)) {
''',
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
  static bool play_heavy_ball(Engine& engine) { return engine.play_heavy_ball(); }
  static bool needs_pineco_connector(const Engine& engine) {
    return engine.needs_pineco_connector();
  }
};
}

namespace {
bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_prized_pineco_is_recovered_for_live_connector() {
  const sim::Scenario scenario{"issue-1252", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1252};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  recipe.emplace_back(sim::Card::Pineco, 1);
  recipe.emplace_back(sim::Card::ForretressEx, 1);
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::HisuianHeavyBall, sim::Card::RegidragoVstar};
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Fire};
  state.prizes = {sim::Card::Pineco, sim::Card::Grass, sim::Card::Fire,
                  sim::Card::Dipplin, sim::Card::Powerglass, sim::Card::FieldBlower};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Hisuian Heavy Ball may recover a revealed Basic Pokémon from the Prize cards.
  // Pineco is a Basic and is the missing live connector for Forretress ex:
  // https://api.pokemontcg.io/v2/cards/swsh10-146
  // https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1252
  if (!sim::EngineTestAccess::needs_pineco_connector(engine) ||
      !sim::EngineTestAccess::play_heavy_ball(engine)) {
    throw std::runtime_error("The live prized-Pineco Heavy Ball route did not resolve.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::Pineco) ||
      contains(after.prizes, sim::Card::Pineco) ||
      !contains(after.prizes, sim::Card::HisuianHeavyBall) ||
      contains(after.discard, sim::Card::HisuianHeavyBall)) {
    throw std::runtime_error("Hisuian Heavy Ball did not exchange for prized Pineco.");
  }
}
}

int main() {
  try {
    test_prized_pineco_is_recovered_for_live_connector();
    std::cout << "Issue 1252 Hisuian Heavy Ball tests passed\n";
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
