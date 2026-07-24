#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen,
                        const bool prizes_revealed) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool use_exploding_energy_for_setup(Engine& engine) {
    return engine.use_exploding_energy_for_setup();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool deck_seen(const Engine& engine) { return engine.deck_seen_; }
  static bool used_exploding_energy(const Engine& engine) {
    return engine.outcome_.used_exploding_energy;
  }
};
}  // namespace sim

namespace {

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

const sim::DeckRecipe& pineco_recipe() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  if (deck == nullptr) {
    throw std::runtime_error("The registered Pineco recipe is unavailable.");
  }
  return deck->recipe;
}

struct Fixture {
  explicit Fixture(const std::string& label)
      : scenario{label, sim::DciProfile::StrictJit, sim::LockMode::None,
                 false, 4},
        rng{1460},
        trace{true, {}},
        engine{scenario, pineco_recipe(), rng, &trace} {}

  sim::Scenario scenario;
  std::mt19937_64 rng;
  sim::TraceLog trace;
  sim::Engine engine;
};

sim::State zero_grass_state(const bool source_active) {
  sim::State state;
  state.turn = 2;
  if (source_active) {
    state.active = sim::Pokemon{sim::Card::ForretressEx, 0, 0, 1};
    state.bench = {sim::Pokemon{sim::Card::RegidragoV, 0, 0, 1}};
  } else {
    state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 1};
    state.bench = {sim::Pokemon{sim::Card::ForretressEx, 0, 0, 1}};
  }
  state.deck = {sim::Card::Fire, sim::Card::Crispin};
  state.prizes = {sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                  sim::Card::Grass, sim::Card::Grass, sim::Card::Grass};
  return state;
}

void verify_resolved_zero_target_search(const bool source_active) {
  Fixture fixture(source_active ? "issue-1460-active-source"
                                : "issue-1460-benched-source");
  sim::EngineTestAccess::set_state(
      fixture.engine, zero_grass_state(source_active), false, false);

  // The K0 policy may announce Exploding Energy from fixed-list public-zone
  // accounting. When the hidden search then finds zero valid Basic Grass targets,
  // Forretress ex still resolves the printed shuffle and self-Knock-Out consequence:
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Core Ability, deck-search, shuffle, Knock Out, Evolution-stack, and promotion procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Knowledge-state contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1460
  if (!sim::EngineTestAccess::use_exploding_energy_for_setup(fixture.engine)) {
    throw std::runtime_error(
        "A legal K0 zero-target Exploding Energy search did not resolve.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!sim::EngineTestAccess::deck_seen(fixture.engine) ||
      !sim::EngineTestAccess::used_exploding_energy(fixture.engine) ||
      !after.active || after.active->card != sim::Card::RegidragoV ||
      !after.bench.empty() ||
      count(after.discard, sim::Card::ForretressEx) != 1 ||
      count(after.discard, sim::Card::Pineco) != 1) {
    throw std::runtime_error(
        "The zero-target search failed to retain K1 while resolving the self-Knock-Out stack and promotion.");
  }
}

void verify_known_zero_declines_announcement() {
  Fixture fixture("issue-1460-k1-control");
  sim::EngineTestAccess::set_state(
      fixture.engine, zero_grass_state(false), true, true);

  // Once K1 already proves that no Basic Grass Energy remains in the deck, policy
  // declines to announce the setup Ability and leaves the board unchanged:
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Knowledge-state contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1460
  if (sim::EngineTestAccess::use_exploding_energy_for_setup(fixture.engine)) {
    throw std::runtime_error(
        "K1 policy announced Exploding Energy with a known zero-target deck.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.active || after.active->card != sim::Card::RegidragoV ||
      after.bench.size() != 1U ||
      after.bench.front().card != sim::Card::ForretressEx ||
      sim::EngineTestAccess::used_exploding_energy(fixture.engine) ||
      !after.discard.empty()) {
    throw std::runtime_error(
        "The K1 zero-target control mutated the board or outcome.");
  }
}

}  // namespace

int main() {
  verify_resolved_zero_target_search(false);
  verify_resolved_zero_target_search(true);
  verify_known_zero_declines_announcement();
}
