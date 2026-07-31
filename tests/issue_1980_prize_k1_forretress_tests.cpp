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
    return engine.issue_1744_quick_ball_forretress_route_available();
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
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1};
  state.bench = {sim::Pokemon{sim::Card::Pineco, 1}};
  state.hand = {sim::Card::SecretBox, sim::Card::ForretressEx,
                sim::Card::QuickBall, sim::Card::Dragapult};
  state.deck = {sim::Card::Grass, sim::Card::RegidragoV,
                sim::Card::Fire, sim::Card::Pineco};
  return state;
}

sim::Engine make_engine(std::mt19937_64& rng) {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  if (deck == nullptr) throw std::runtime_error("Pineco recipe is unavailable.");
  static const sim::Scenario scenario{
      "issue-1980", sim::DciProfile::MatchupFlexJit,
      sim::LockMode::None, true, 4};
  return sim::Engine(scenario, deck->recipe, rng);
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
  // Hisuian Heavy Ball reveals the complete Prize set. The fixed recipe and public
  // zones then establish the same K1 deck composition used by the route selector:
  // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Official Prize, Item, evolution, Ability, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1980
  expect(admitted(true, false, 198000),
         "Deck-search K1 must admit the complete Forretress route.");
  expect(admitted(false, true, 198001),
         "Prize-inspection K1 must admit the same complete route.");
  expect(!admitted(false, false, 198002),
         "True K0 must keep the composition-dependent route blocked.");
}
}  // namespace

int main() {
  test_k1_provenance_equivalence();
}
