#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <optional>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = false;
  }

  static std::optional<Card> route_cost(
      const Engine& engine, const Card search_item) {
    return engine.issue_2264_latias_burnet_replaced_supporter_cost(search_item);
  }

  static std::optional<Card> choose_cost(
      const Engine& engine, const Card search_item) {
    return engine.choose_discard(false, true, true, search_item);
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State latias_burnet_finish_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoVstar, 2, 2, 1, sim::Tool::None},
  };
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Crispin,
                sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::LatiasEx, sim::Card::MegaDragonite,
                sim::Card::Dragapult, sim::Card::Grass, sim::Card::Fire};
  return state;
}

void test_strict_treasure_admits_route_replaced_crispin() {
  sim::Scenario scenario{"issue-2264/strict", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{2264};
  sim::Engine engine{scenario, recipe, rng};
  sim::EngineTestAccess::set_state(engine, latias_burnet_finish_state());

  // The exact K1 state has GGF complete on the Benched VSTAR, a Basic Active,
  // searchable Latias ex, unused retreat, held Professor Burnet, and a known
  // deck-resident Dragon payload. Mysterious Treasure can therefore spend held
  // Crispin as its one-card cost, search Latias ex, use Skyliner for free retreat,
  // and leave Burnet to create the required current-turn Apex Dragon payload.
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Item, Supporter, Bench, Ability, and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, dynamic DCI, strict-JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed refinement and source-bound seed-33 witness: https://github.com/FlareZ123/pokemon-sims/issues/2264#issuecomment-5215391405
  expect(sim::EngineTestAccess::route_cost(engine, sim::Card::MysteriousTreasure) ==
             sim::Card::Crispin,
         "#2264 strict route did not admit route-replaced Crispin.");
  expect(sim::EngineTestAccess::choose_cost(engine, sim::Card::MysteriousTreasure) ==
             sim::Card::Crispin,
         "#2264 strict route was not integrated into the discard selector.");
}

void test_matchup_flex_quick_ball_admits_route_replaced_crispin() {
  sim::Scenario scenario{"issue-2264/flex", sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{22640};
  sim::Engine engine{scenario, recipe, rng};
  sim::State state = latias_burnet_finish_state();
  state.turn = 4;
  state.hand.erase(state.hand.begin());
  state.hand.push_back(sim::Card::QuickBall);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Quick Ball has the same one-card cost before its Basic search. With every
  // completion gate already public, preserving Crispin cannot improve the direct
  // Latias ex plus Burnet finish represented by the confirmed seed-220 trace.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Dynamic DCI and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed refinement and source-bound seed-220 witness: https://github.com/FlareZ123/pokemon-sims/issues/2264#issuecomment-5215077864
  expect(sim::EngineTestAccess::route_cost(engine, sim::Card::QuickBall) ==
             sim::Card::Crispin,
         "#2264 matchup-flex route did not admit route-replaced Crispin.");
}

void test_route_preserves_supporter_when_any_completion_gate_is_missing() {
  sim::Scenario scenario{"issue-2264/strict", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{22641};
  sim::Engine engine{scenario, recipe, rng};

  sim::State state = latias_burnet_finish_state();
  state.bench.front().grass = 1;
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::route_cost(engine, sim::Card::MysteriousTreasure),
         "#2264 spent a live Supporter before GGF was complete.");

  state = latias_burnet_finish_state();
  state.deck.erase(state.deck.begin());
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::route_cost(engine, sim::Card::MysteriousTreasure),
         "#2264 spent a live Supporter without known Latias ex in deck.");

  state = latias_burnet_finish_state();
  sim::EngineTestAccess::set_state(engine, std::move(state), false);
  expect(!sim::EngineTestAccess::route_cost(engine, sim::Card::MysteriousTreasure),
         "#2264 used K1-only discard logic while K0.");
}

}  // namespace

int main() {
  test_strict_treasure_admits_route_replaced_crispin();
  test_matchup_flex_quick_ball_admits_route_replaced_crispin();
  test_route_preserves_supporter_when_any_completion_gate_is_missing();
  return 0;
}
