#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen,
                        const bool prizes_revealed) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }

  static const State& state(const Engine& engine) { return engine.state_; }

  static bool prizes_known(const Engine& engine) {
    return engine.prizes_known();
  }

  static bool post_payload_target(const Engine& engine) {
    return engine.second_ultra_ball_has_post_payload_target();
  }

  static bool play_ultra_ball(Engine& engine) {
    return engine.play_ultra_ball(false);
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State two_ultra_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1};
  state.hand = {sim::Card::UltraBall, sim::Card::UltraBall,
                sim::Card::Dipplin, sim::Card::Dipplin,
                sim::Card::Dipplin};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Grass};
  state.prizes = {sim::Card::Fire, sim::Card::Grass,
                  sim::Card::Guzma, sim::Card::Channeler,
                  sim::Card::Lusamine, sim::Card::HisuianHeavyBall};
  return state;
}

sim::Engine make_engine(std::mt19937_64& rng) {
  static const sim::Scenario scenario{"issue-2063-prize-k1",
                                      sim::DciProfile::StrictJit,
                                      sim::LockMode::None, false, 4};
  return sim::Engine(scenario, sim::baseline_recipe(), rng);
}

void test_both_k1_provenances_reject_the_false_second_search() {
  std::mt19937_64 deck_rng{20630};
  sim::Engine deck_engine = make_engine(deck_rng);
  const sim::State expected = two_ultra_state();
  sim::EngineTestAccess::set_state(deck_engine, expected, true, false);
  expect(sim::EngineTestAccess::prizes_known(deck_engine),
         "Deck inspection did not establish K1.");
  expect(!sim::EngineTestAccess::post_payload_target(deck_engine),
         "Deck-search K1 invented a second Ultra Ball Pokémon target.");
  expect(!sim::EngineTestAccess::play_ultra_ball(deck_engine),
         "Deck-search K1 spent the first Ultra Ball on a dead continuation.");
  expect(sim::EngineTestAccess::state(deck_engine).hand == expected.hand &&
             sim::EngineTestAccess::state(deck_engine).deck == expected.deck &&
             sim::EngineTestAccess::state(deck_engine).discard.empty(),
         "The rejected deck-search K1 route changed a card zone.");

  std::mt19937_64 prize_rng{20631};
  sim::Engine prize_engine = make_engine(prize_rng);
  sim::EngineTestAccess::set_state(prize_engine, expected, false, true);

  // Full Prize inspection establishes the same exact fixed-list K1 as a deck
  // inspection. After the first Ultra Ball removes the sole Pokémon, only Grass
  // remains, so the promised second Ultra Ball has no legal Pokémon target and
  // the stronger action preserves both Items and every discard-cost card:
  // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Trainer no-effect ruling: https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  // K1 and resource-preservation specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2063
  expect(sim::EngineTestAccess::prizes_known(prize_engine),
         "Complete Prize inspection did not establish K1.");
  expect(!sim::EngineTestAccess::post_payload_target(prize_engine),
         "Prize-inspection K1 invented a second Ultra Ball Pokémon target.");
  expect(!sim::EngineTestAccess::play_ultra_ball(prize_engine),
         "Prize-inspection K1 spent the first Ultra Ball on a dead continuation.");
  expect(sim::EngineTestAccess::state(prize_engine).hand == expected.hand &&
             sim::EngineTestAccess::state(prize_engine).deck == expected.deck &&
             sim::EngineTestAccess::state(prize_engine).discard.empty(),
         "The rejected Prize-inspection K1 route changed a card zone.");
}

void test_true_k0_preserves_hidden_target_uncertainty() {
  std::mt19937_64 rng{20632};
  sim::Engine engine = make_engine(rng);
  sim::EngineTestAccess::set_state(engine, two_ultra_state(), false, false);

  // K0 cannot inspect exact deck identities before a legal effect resolves, so a
  // second Pokémon target remains plausible under the fixed-list model:
  // Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // Hidden-information specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2063
  expect(!sim::EngineTestAccess::prizes_known(engine),
         "The true-K0 control unexpectedly had exact hidden-zone knowledge.");
  expect(sim::EngineTestAccess::post_payload_target(engine),
         "The K0 projection used exact hidden deck identities.");
}
}  // namespace

int main() {
  test_both_k1_provenances_reject_the_false_second_search();
  test_true_k0_preserves_hidden_target_uncertainty();
}
