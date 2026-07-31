#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen,
                        const bool prizes_revealed) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool route_available(const Engine& engine) {
    return engine.quick_ball_seed23_latias_route_ready();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state() {
  sim::State state;
  state.turn = 3;
  state.vstar_power_used = true;
  state.supporter_used = true;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1};
  state.bench = {sim::Pokemon{sim::Card::TapuLeleGX, 2},
                 sim::Pokemon{sim::Card::RegidragoV, 3, 2, 0,
                              sim::Tool::ForestSealStone}};
  state.hand = {sim::Card::TateLiza, sim::Card::QuickBall,
                sim::Card::RegidragoVstar, sim::Card::ProfessorBurnet,
                sim::Card::Fire};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::LatiasEx,
                sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::RegidragoV};
  state.discard = {sim::Card::StevensResolve, sim::Card::Crispin};
  return state;
}

bool admitted(const bool deck_seen, const bool prizes_revealed,
              const std::uint64_t seed) {
  const sim::Scenario scenario{"issue-1981", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{seed};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, route_state(), deck_seen,
                                   prizes_revealed);
  return sim::EngineTestAccess::route_available(engine);
}

void test_k1_provenance_equivalence() {
  // Full-Prize inspection establishes exact K1 composition while the selector still
  // checks Latias, payload, Energy, evolution, Ability, Bench, retreat, and horizon:
  // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Prize, Item, Bench, Ability, evolution, retreat, Supporter, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1981
  expect(admitted(true, false, 198100),
         "Deck-search K1 must admit the complete seed-23 Latias route.");
  expect(admitted(false, true, 198101),
         "Prize-inspection K1 must admit the same complete route.");
  expect(!admitted(false, false, 198102),
         "True K0 must keep the composition-dependent route blocked.");
}

void test_registered_seed_23_route_remains_t4() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  if (!scenario) throw std::runtime_error("Missing strict-jit/go-first scenario.");
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{23};
  sim::Engine engine(*scenario, recipe, rng);
  const sim::TrialOutcome outcome = engine.run();

  // Existing source-bound route witness: https://github.com/FlareZ123/pokemon-sims/issues/1403
  // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  expect(outcome.first_ready_turn == 4,
         "Seed 23 must retain its verified T4 ready turn.");
}
}  // namespace

int main() {
  test_k1_provenance_equivalence();
  test_registered_seed_23_route_remains_t4();
}
