#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool known = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = known;
  }

  static bool available(const Engine& engine) {
    return engine.issue_1745_steven_latias_t3_route_available();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.manual_energy_used = true;
  // The Active only needs to remain a Basic for Latias ex's Skyliner pivot.
  // Tapu Lele-GX Basic identity: https://api.pokemontcg.io/v2/cards/sm2-60
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, turn, 0, 0,
                              sim::Tool::None, 0};
  // This Regidrago V entered on an earlier turn and is exactly one Grass short
  // of Apex Dragon's GGF cost. Regidrago V/VSTAR:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, turn - 1, 1, 1,
                              sim::Tool::None, 0}};
  state.hand = {sim::Card::StevensResolve};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::LatiasEx,
                sim::Card::Grass, sim::Card::QuickBall};
  // NoDiscardControl requires the Dragon payload to be banked before this route.
  // Repository DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  state.discard = {sim::Card::MegaDragonite};
  state.prizes = {sim::Card::Fire, sim::Card::Arven, sim::Card::Serena,
                  sim::Card::Crispin, sim::Card::FieldBlower,
                  sim::Card::ForestSealStone};
  return state;
}

bool available(const int turn, const bool going_first,
               const int max_turn = 6, const bool known = true,
               const sim::LockMode lock = sim::LockMode::None,
               const bool supporter_used = false) {
  const sim::Scenario scenario{"issue-3276", sim::DciProfile::NoDiscardControl,
                               lock, going_first, max_turn};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3276000 + static_cast<unsigned>(turn));
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = route_state(turn);
  state.supporter_used = supporter_used;
  sim::EngineTestAccess::set_state(engine, std::move(state), known);
  return sim::EngineTestAccess::available(engine);
}

void test_seat_and_relative_turn_parity() {
  // Steven's Resolve and the next-turn evolution/attachment/Skyliner sequence have
  // no seat or absolute-turn text. Current Supporter legality, evolution age, K1,
  // board space, Ability availability, and the one-turn horizon remain explicit.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Earliest-route/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3276
  expect(available(2, false), "historical go-second T2 witness disappeared");
  expect(available(2, true), "equivalent go-first T2 state remained hidden");
  expect(available(3, false), "equivalent later go-second state remained hidden");
  expect(available(3, true), "equivalent later go-first state remained hidden");
}

void test_semantic_boundaries() {
  // Exhausted horizon, K0, a real lock, and an already-spent Supporter remain blockers.
  // Official Supporter/evolution/Ability procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Knowledge/lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3276
  expect(!available(3, true, 3), "route crossed an exhausted next-turn horizon");
  expect(!available(3, true, 6, false), "K0 admitted deterministic Steven targets");
  expect(!available(3, true, 6, true, sim::LockMode::FullSupporter),
         "Supporter lock admitted Steven's Resolve");
  expect(!available(3, true, 6, true, sim::LockMode::None, true),
         "second Supporter in one turn was admitted");
}

}  // namespace

int main() {
  test_seat_and_relative_turn_parity();
  test_semantic_boundaries();
  return 0;
}
