#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen,
                        const bool prizes_revealed) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }

  static bool prizes_known(const Engine& engine) {
    return engine.prizes_known();
  }

  static bool route_available(const Engine& engine) {
    return engine.issue_1745_steven_latias_t3_route_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State complete_route_state() {
  sim::State state;
  state.turn = 2;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1,
                              sim::Tool::ForestSealStone}};
  state.hand = {sim::Card::EarthenVessel, sim::Card::QuickBall,
                sim::Card::Fire, sim::Card::StevensResolve,
                sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::LatiasEx,
                sim::Card::Grass, sim::Card::Arven, sim::Card::Pineco};
  state.prizes = {sim::Card::Guzma, sim::Card::Channeler,
                  sim::Card::Lusamine, sim::Card::Dipplin,
                  sim::Card::Fire, sim::Card::Grass};
  state.discard = {sim::Card::MegaDragonite, sim::Card::EarthenVessel,
                   sim::Card::Arven};
  return state;
}

sim::Engine make_engine(std::mt19937_64& rng) {
  const sim::Scenario scenario{"issue-2061-prize-k1",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 5};
  const sim::CrobatModelingDeck* deck =
      sim::crobat_modeling_deck_by_id("crobat1-erika");
  if (deck == nullptr) throw std::runtime_error("Modeling recipe is unavailable.");
  return sim::Engine(scenario, deck->recipe, rng);
}

void test_both_k1_provenances_and_true_k0() {
  std::mt19937_64 deck_rng{20610};
  sim::Engine deck_engine = make_engine(deck_rng);
  sim::EngineTestAccess::set_state(
      deck_engine, complete_route_state(), true, false);
  expect(sim::EngineTestAccess::prizes_known(deck_engine),
         "Deck inspection did not establish K1.");
  expect(sim::EngineTestAccess::route_available(deck_engine),
         "The deck-search K1 issue-1745 route was rejected.");

  std::mt19937_64 prize_rng{20611};
  sim::Engine prize_engine = make_engine(prize_rng);
  sim::EngineTestAccess::set_state(
      prize_engine, complete_route_state(), false, true);

  // A complete Prize inspection and a legal deck inspection reveal complementary
  // fixed-list zones and therefore establish the same K1. Steven can search the
  // known Regidrago VSTAR, Latias ex, and Grass package, then the prior-turn
  // Regidrago evolves, receives the manual attachment, and uses Skyliner to promote:
  // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Official Prize, Supporter, search, evolution, attachment, Bench, Ability, and Retreat procedure: https://assets.pokemon.com/assets/cms2-en-uk/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Original route: https://github.com/FlareZ123/pokemon-sims/issues/1745
  // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2061
  expect(sim::EngineTestAccess::prizes_known(prize_engine),
         "Complete Prize inspection did not establish K1.");
  expect(sim::EngineTestAccess::route_available(prize_engine),
         "The Prize-inspection K1 issue-1745 route was rejected.");

  std::mt19937_64 k0_rng{20612};
  sim::Engine k0_engine = make_engine(k0_rng);
  sim::EngineTestAccess::set_state(
      k0_engine, complete_route_state(), false, false);
  expect(!sim::EngineTestAccess::prizes_known(k0_engine),
         "The true-K0 control unexpectedly had exact hidden-zone knowledge.");
  expect(!sim::EngineTestAccess::route_available(k0_engine),
         "The issue-1745 route used exact deck identities before K1.");
}
}  // namespace

int main() {
  test_both_k1_provenances_and_true_k0();
}
