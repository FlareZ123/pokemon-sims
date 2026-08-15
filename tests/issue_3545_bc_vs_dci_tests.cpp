#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static std::optional<Card> discard_candidate(
      const Engine& engine, const std::optional<Card> excluded = std::nullopt) {
    return engine.choose_discard(false, false, true, excluded, false);
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Engine make_engine(const sim::LockMode lock, std::mt19937_64& rng) {
  const sim::Scenario scenario{"issue-3545-dci", sim::DciProfile::StrictJit,
                               lock, false, 4};
  return sim::Engine{scenario, sim::baseline_recipe(), rng};
}

sim::State neutral_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1};
  state.discard.push_back(sim::Card::MegaDragonite);
  state.discarded_this_turn.push_back(sim::Card::MegaDragonite);
  return state;
}

void test_duplicate_items_are_fallback_dci() {
  // A second physical copy remains legal discard fuel after preserving one copy for
  // the live Item route. These fallbacks run only after all established DCI choices.
  // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
  // VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  {
    std::mt19937_64 rng{3545};
    sim::Engine engine = make_engine(sim::LockMode::None, rng);
    sim::State state = neutral_state();
    state.hand = {sim::Card::BattleCompressor, sim::Card::BattleCompressor};
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(sim::EngineTestAccess::discard_candidate(engine) == sim::Card::BattleCompressor,
           "duplicate BC was not available as fallback DCI");
  }
  {
    std::mt19937_64 rng{3546};
    sim::Engine engine = make_engine(sim::LockMode::None, rng);
    sim::State state = neutral_state();
    state.hand = {sim::Card::VsSeeker, sim::Card::VsSeeker};
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(sim::EngineTestAccess::discard_candidate(engine) == sim::Card::VsSeeker,
           "duplicate VS Seeker was not available as fallback DCI");
  }
}

void test_persistently_locked_singletons_become_dci() {
  // TurnTwoItem is persistent from T2 onward in the repository model. A singleton BC
  // or VS Seeker has no remaining play value once that lock is active, so it may pay
  // another legal discard cost without consuming a future Item route.
  // Persistent lock: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
  // Battle Compressor / VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-92 https://api.pokemontcg.io/v2/cards/xy4-109
  {
    std::mt19937_64 rng{3547};
    sim::Engine engine = make_engine(sim::LockMode::TurnTwoItem, rng);
    sim::State state = neutral_state();
    state.hand = {sim::Card::BattleCompressor};
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(sim::EngineTestAccess::discard_candidate(engine) == sim::Card::BattleCompressor,
           "locked singleton BC retained nonexistent future Item value");
  }
  {
    std::mt19937_64 rng{3548};
    sim::Engine engine = make_engine(sim::LockMode::TurnTwoItem, rng);
    sim::State state = neutral_state();
    state.hand = {sim::Card::VsSeeker};
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(sim::EngineTestAccess::discard_candidate(engine) == sim::Card::VsSeeker,
           "locked singleton VS Seeker retained nonexistent future Item value");
  }
}
}  // namespace

int main() {
  try {
    test_duplicate_items_are_fallback_dci();
    test_persistently_locked_singletons_become_dci();
    std::cout << "Issue 3545 BC/VS DCI tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
