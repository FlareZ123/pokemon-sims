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
  static bool complete(Engine& engine) {
    return engine.complete_issue_1724_crobat_stadium_compression();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State exact_public_state() {
  sim::State state;
  state.turn = 5;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::ForestSealStone};
  state.bench = {
      sim::Pokemon{sim::Card::LatiasEx, 1, 0, 0, sim::Tool::None}};
  state.hand = {sim::Card::PathToPeak, sim::Card::Powerglass,
                sim::Card::ChaoticSwell, sim::Card::Lusamine,
                sim::Card::Crispin, sim::Card::TeamYellsCheer,
                sim::Card::CrobatV};
  state.deck = {sim::Card::ProfessorBurnet, sim::Card::BrilliantBlender,
                sim::Card::Dragapult, sim::Card::Grass};
  state.discard = {sim::Card::RoseannesBackup, sim::Card::QuickBall};
  state.vstar_power_used = true;
  return state;
}

bool compression_resolves(const bool deck_seen, const bool prizes_revealed,
                          const std::uint64_t seed) {
  sim::Scenario scenario{"issue-1994-k1", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe(sim::kDeckRecipe.begin(), sim::kDeckRecipe.end());
  std::mt19937_64 rng{seed};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, exact_public_state(), deck_seen,
                                   prizes_revealed);
  return sim::EngineTestAccess::complete(engine);
}

void test_k1_provenance_equivalence() {
  // A legal deck inspection and a complete legal Prize inspection establish the
  // same fixed-list composition knowledge for this exact route. True K0 remains
  // blocked, while every existing Stadium, Bench, Ability, lock, hand-size,
  // deterministic-connector, and setup-axis boundary remains in production:
  // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
  // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Future-card-oracle boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
  // Official Prize, Stadium, Bench, Ability, search, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1994
  expect(compression_resolves(true, false, 199400),
         "Deck-search K1 must admit the issue-1724 compression route.");
  expect(compression_resolves(false, true, 199401),
         "Prize-inspection K1 must admit the equivalent compression route.");
  expect(!compression_resolves(false, false, 199402),
         "True K0 must keep the composition-dependent route blocked.");
}

}  // namespace

int main() {
  test_k1_provenance_equivalence();
}
