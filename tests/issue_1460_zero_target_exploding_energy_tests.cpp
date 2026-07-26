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
  static bool resolve_exploding_energy(
      Engine& engine, const std::vector<std::size_t>& destinations) {
    return engine.resolve_exploding_energy(destinations);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool deck_seen(const Engine& engine) { return engine.deck_seen_; }
  static bool used_exploding_energy(const Engine& engine) {
    return engine.outcome_.used_exploding_energy;
  }
};
}  // namespace sim

namespace {

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
        rng{1587},
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

void verify_k0_empty_search_retains_board(const bool source_active) {
  Fixture fixture(source_active ? "issue-1587-active-source"
                      : "issue-1587-benched-source");
  sim::EngineTestAccess::set_state(
      fixture.engine, zero_grass_state(source_active), false, false);

  // The K0 policy may begin a hidden-zone search from public copy accounting.
  // Once the search reveals zero Basic Grass Energy, the February 2026 ruling
  // leaves no legal 1-through-5 Ability selection. K1 remains acquired while
  // Forretress ex, its stack, and the board remain unchanged:
  // Official February 2026 ruling: https://professorprogram.pokemon.com/news/11473085
  // Forretress ex / Exploding Energy: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Knowledge-state contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#knowledge-states
  // Confirmed superseding bug: https://github.com/FlareZ123/pokemon-sims/issues/1587
  if (sim::EngineTestAccess::use_exploding_energy_for_setup(fixture.engine)) {
    throw std::runtime_error(
        "A K0 empty Exploding Energy search resolved an illegal zero-card selection.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  const sim::Card expected_active = source_active
      ? sim::Card::ForretressEx : sim::Card::RegidragoV;
  const sim::Card expected_bench = source_active
      ? sim::Card::RegidragoV : sim::Card::ForretressEx;
  if (!sim::EngineTestAccess::deck_seen(fixture.engine) ||
      sim::EngineTestAccess::used_exploding_energy(fixture.engine) ||
      !after.active || after.active->card != expected_active ||
      after.bench.size() != 1U ||
      after.bench.front().card != expected_bench ||
      !after.discard.empty()) {
    throw std::runtime_error(
        "The K0 empty search failed to retain K1 without resolving Exploding Energy.");
  }
}

void verify_resolver_rejects_empty_destinations() {
  Fixture fixture("issue-1587-empty-resolver");
  sim::State state = zero_grass_state(false);
  state.deck.push_back(sim::Card::Grass);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state), true, true);

  // The low-level resolver also enforces the Ability's 1-through-5 minimum:
  // Official February 2026 ruling: https://professorprogram.pokemon.com/news/11473085
  // Forretress ex / Exploding Energy: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1587
  if (sim::EngineTestAccess::resolve_exploding_energy(fixture.engine, {})) {
    throw std::runtime_error(
        "The Exploding Energy resolver accepted an empty destination list.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.active || after.active->card != sim::Card::RegidragoV ||
      after.bench.size() != 1U ||
      after.bench.front().card != sim::Card::ForretressEx ||
      sim::EngineTestAccess::used_exploding_energy(fixture.engine) ||
      !after.discard.empty()) {
    throw std::runtime_error(
        "Rejecting the empty resolver input mutated the board or outcome.");
  }
}

void verify_known_zero_declines_announcement() {
  Fixture fixture("issue-1587-k1-control");
  sim::EngineTestAccess::set_state(
      fixture.engine, zero_grass_state(false), true, true);

  // K1 already proves that no legal Basic Grass Energy selection exists, so
  // policy declines to announce the Ability and leaves every zone unchanged:
  // Official February 2026 ruling: https://professorprogram.pokemon.com/news/11473085
  // Forretress ex / Exploding Energy: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Knowledge-state contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1587
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
  verify_k0_empty_search_retains_board(false);
  verify_k0_empty_search_retains_board(true);
  verify_resolver_rejects_empty_destinations();
  verify_known_zero_declines_announcement();
}
