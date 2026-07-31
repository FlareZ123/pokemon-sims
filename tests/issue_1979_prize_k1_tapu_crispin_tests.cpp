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
    return engine.issue_1875_quick_ball_tapu_crispin_route_available();
  }
  static bool complete_route(Engine& engine) {
    return engine.complete_issue_1875_quick_ball_tapu_crispin_route();
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
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 0};
  state.hand = {sim::Card::Fire, sim::Card::QuickBall,
                sim::Card::Dragapult};
  state.deck = {sim::Card::RegidragoV, sim::Card::Grass,
                sim::Card::Fire, sim::Card::Crispin,
                sim::Card::TapuLeleGX};
  return state;
}

sim::Engine make_engine(std::mt19937_64& rng) {
  static const sim::Scenario scenario{
      "issue-1979", sim::DciProfile::StrictJit,
      sim::LockMode::None, true, 5};
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  return sim::Engine(scenario, recipe, rng);
}

bool admitted(const bool deck_seen, const bool prizes_revealed,
              const std::uint64_t seed) {
  std::mt19937_64 rng{seed};
  sim::Engine engine = make_engine(rng);
  sim::EngineTestAccess::set_state(engine, route_state(), deck_seen,
                                   prizes_revealed);
  return sim::EngineTestAccess::route_available(engine);
}

void test_k1_provenance_equivalence() {
  // Full-Prize inspection establishes exact K1 composition without exposing deck
  // order. Every Tapu-Crispin physical route condition remains checked separately:
  // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
  // Official Prize, Item, Bench, Ability, Supporter, attachment, and search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1979
  expect(admitted(true, false, 197900),
         "Deck-search K1 must admit the complete Tapu-Crispin route.");
  expect(admitted(false, true, 197901),
         "Prize-inspection K1 must admit the same complete route.");
  expect(!admitted(false, false, 197902),
         "True K0 must keep the composition-dependent route blocked.");
}

void test_prize_k1_completes_the_full_turn() {
  std::mt19937_64 rng{197903};
  sim::Engine engine = make_engine(rng);
  sim::EngineTestAccess::set_state(engine, route_state(), false, true);

  // The source-bound Prize-only K1 state must execute the whole legal connector
  // chain, including Quick Ball's payload cost, Wonder Tag, and Crispin's final
  // Grass attachment, instead of merely passing the route-admission predicate:
  // Route specification: https://github.com/FlareZ123/pokemon-sims/issues/1979
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::complete_route(engine),
         "Prize-inspection K1 must complete the full Tapu-Crispin turn.");
}
}  // namespace

int main() {
  test_k1_provenance_equivalence();
  test_prize_k1_completes_the_full_turn();
}
