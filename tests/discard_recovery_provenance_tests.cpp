#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static bool recover_discard_to_hand(Engine& engine, const Card card) {
    return engine.recover_discard_to_hand(card);
  }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};
}  // namespace sim

namespace {
struct Fixture {
  sim::Scenario scenario{"issue-934-discard-recovery-provenance",
                         sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{93401};
  sim::Engine engine{scenario, recipe, rng};
};

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

void older_duplicate_preserves_current_turn_payload() {
  Fixture fixture;
  sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  state.turn = 2;
  state.discard = {sim::Card::MegaDragonite, sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};

  // Legacy Star can recover the older duplicate while the copy discarded this turn
  // remains available for strict-JIT Apex Dragon readiness:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  // https://github.com/FlareZ123/pokemon-sims/issues/934
  if (!sim::EngineTestAccess::recover_discard_to_hand(
          fixture.engine, sim::Card::MegaDragonite)) {
    throw std::runtime_error("older duplicate recovery was rejected");
  }
  if (count(state.discard, sim::Card::MegaDragonite) != 1 ||
      count(state.discarded_this_turn, sim::Card::MegaDragonite) != 1 ||
      count(state.hand, sim::Card::MegaDragonite) != 1 ||
      !sim::EngineTestAccess::payload_ready(fixture.engine)) {
    throw std::runtime_error(
        "older duplicate recovery erased the remaining current-turn payload");
  }
}

void sole_current_turn_copy_clears_provenance() {
  Fixture fixture;
  sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  state.turn = 2;
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};

  if (!sim::EngineTestAccess::recover_discard_to_hand(
          fixture.engine, sim::Card::MegaDragonite)) {
    throw std::runtime_error("sole current-turn recovery was rejected");
  }
  if (!state.discard.empty() || !state.discarded_this_turn.empty() ||
      sim::EngineTestAccess::payload_ready(fixture.engine)) {
    throw std::runtime_error(
        "sole recovered current-turn payload retained strict-JIT provenance");
  }
}

void all_current_turn_duplicates_decrement_one_marker() {
  Fixture fixture;
  sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  state.turn = 2;
  state.discard = {sim::Card::MegaDragonite, sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite,
                               sim::Card::MegaDragonite};

  if (!sim::EngineTestAccess::recover_discard_to_hand(
          fixture.engine, sim::Card::MegaDragonite)) {
    throw std::runtime_error("current-turn duplicate recovery was rejected");
  }
  if (count(state.discard, sim::Card::MegaDragonite) != 1 ||
      count(state.discarded_this_turn, sim::Card::MegaDragonite) != 1 ||
      !sim::EngineTestAccess::payload_ready(fixture.engine)) {
    throw std::runtime_error(
        "all-current-turn duplicate recovery did not remove exactly one marker");
  }
}
}  // namespace

int main() {
  older_duplicate_preserves_current_turn_payload();
  sole_current_turn_copy_clears_provenance();
  all_current_turn_duplicates_decrement_one_marker();
  return 0;
}
