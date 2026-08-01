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
    return engine.issue_1772_steven_t3_package_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State exact_t2_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 0, 1,
                              sim::Tool::None}};
  state.hand = {sim::Card::StevensResolve, sim::Card::Gladion,
                sim::Card::EarthenVessel, sim::Card::RegidragoVstar,
                sim::Card::Crispin};
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Fire, sim::Card::LatiasEx,
                sim::Card::Dragapult, sim::Card::Arven};
  state.prizes = {sim::Card::BrilliantBlender};
  return state;
}

sim::Engine make_engine(std::mt19937_64& rng) {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  if (deck == nullptr) throw std::runtime_error("Registered shell is unavailable.");
  static const sim::Scenario scenario{"issue-2066-prize-k1",
                                      sim::DciProfile::MatchupFlexJit,
                                      sim::LockMode::None, true, 5};
  return sim::Engine(scenario, deck->recipe, rng);
}

void test_both_k1_provenances_admit_the_same_package() {
  std::mt19937_64 deck_rng{20660};
  sim::Engine deck_engine = make_engine(deck_rng);
  sim::EngineTestAccess::set_state(deck_engine, exact_t2_state(), true, false);
  expect(sim::EngineTestAccess::prizes_known(deck_engine),
         "Deck inspection did not establish K1.");
  expect(sim::EngineTestAccess::route_available(deck_engine),
         "Deck-search K1 rejected the issue-1772 Steven package.");

  std::mt19937_64 prize_rng{20661};
  sim::Engine prize_engine = make_engine(prize_rng);
  sim::EngineTestAccess::set_state(prize_engine, exact_t2_state(), false, true);

  // Full Prize inspection proves the same fixed-list inventory as a resolved
  // deck search. Steven can bank Grass, Latias ex, and Dragapult ex; Crispin and
  // Earthen Vessel then finish the two Energy attachments and strict-JIT payload:
  // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago V and VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // K1, strict-JIT, and earliest-route specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2066
  expect(sim::EngineTestAccess::prizes_known(prize_engine),
         "Complete Prize inspection did not establish K1.");
  expect(sim::EngineTestAccess::route_available(prize_engine),
         "Prize-inspection K1 rejected the issue-1772 Steven package.");
}

void test_true_k0_remains_rejected() {
  std::mt19937_64 rng{20662};
  sim::Engine engine = make_engine(rng);
  sim::EngineTestAccess::set_state(engine, exact_t2_state(), false, false);

  // K0 cannot use exact hidden-zone identities before a legal inspection:
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2066
  expect(!sim::EngineTestAccess::prizes_known(engine),
         "The true-K0 control unexpectedly had exact hidden-zone knowledge.");
  expect(!sim::EngineTestAccess::route_available(engine),
         "The issue-1772 Steven package was admitted at true K0.");
}
}  // namespace

int main() {
  test_both_k1_provenances_admit_the_same_package();
  test_true_k0_remains_rejected();
}
