#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1 = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = k1;
  }
  static bool play_vessel(Engine& engine) {
    return engine.play_earthen_vessel(false);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State strict_gg_state() {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::EarthenVessel, sim::Card::Dragapult,
                sim::Card::Dipplin};
  state.deck = {sim::Card::Fire, sim::Card::Grass,
                sim::Card::QuickBall};
  return state;
}

void exact_public_gg_state_spends_dragon_and_searches_fire() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-2231 fixture is unavailable.");

  std::mt19937_64 rng(2231);
  sim::Engine engine(*scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(engine, strict_gg_state());

  // At public K1 with Active Regidrago VSTAR at GG, Earthen Vessel can search
  // Fire and the unused manual attachment can complete GGF. Dragapult ex is DCI 1
  // for this exact Item cost because the same payment establishes the required
  // current-turn Apex Dragon payload, while Dipplin remains ordinary fodder.
  // This GG orientation is distinct from merged #2224's GF -> Grass route.
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, discard, search, and Energy attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, dynamic DCI/JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bugs defining the two Energy orientations: https://github.com/FlareZ123/pokemon-sims/issues/2224 https://github.com/FlareZ123/pokemon-sims/issues/2231
  expect(sim::EngineTestAccess::play_vessel(engine),
         "The exact public GG Earthen Vessel route was rejected.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.discarded_this_turn, sim::Card::Dragapult),
         "The GG route did not use Dragapult ex as Vessel's payload cost.");
  expect(!contains(after.discarded_this_turn, sim::Card::Dipplin),
         "The GG route discarded Dipplin instead of the payload.");
  expect(contains(after.hand, sim::Card::Fire),
         "The GG route did not search the missing Fire Energy.");
}

void k0_control_does_not_spend_protected_payload() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-2231 K0 fixture is unavailable.");

  std::mt19937_64 rng(2231);
  sim::Engine engine(*scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(engine, strict_gg_state(), false);
  sim::EngineTestAccess::play_vessel(engine);

  // K1 is not retroactive. Before any legal inspection the policy may not spend
  // a protected Dragon based on hidden Fire availability in the deck.
  // K0/K1 rule: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k1-after-a-legal-deck-or-prize-inspection
  // Future-card-oracle prohibition: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(!contains(after.discarded_this_turn, sim::Card::Dragapult),
         "K0 incorrectly spent the protected Dragon payload.");
}
}  // namespace

int main() {
  try {
    exact_public_gg_state_spends_dragon_and_searches_fire();
    k0_control_does_not_spend_protected_payload();
    std::cout << "Issue 2231 strict GG Vessel payload tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}