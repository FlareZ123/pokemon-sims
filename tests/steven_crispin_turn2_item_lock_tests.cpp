#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static void set_deck_seen(Engine& engine, const bool value) { engine.deck_seen_ = value; }
  static bool should_play_steven(const Engine& engine) { return engine.should_play_steven(); }
  static Card choose_supporter_after_search_started(const Engine& engine) {
    return engine.choose_supporter_after_search_started();
  }
};

}  // namespace sim

namespace {

sim::State exact_k1_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 0},
                 sim::Pokemon{sim::Card::TapuLeleGX, 1}};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::RegidragoV,
                sim::Card::Powerglass, sim::Card::Guzma,
                sim::Card::ProfessorTuro};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Crispin,
                sim::Card::StevensResolve, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, sim::State state,
                        std::mt19937_64& rng) {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine, true);
  return engine;
}

void test_crispin_preempts_steven_for_exact_route() {
  const sim::Scenario scenario{"issue-774", sim::DciProfile::StrictJit,
                               sim::LockMode::TurnTwoItem, false, 4};
  std::mt19937_64 rng{774};
  sim::Engine engine = make_engine(scenario, exact_k1_state(), rng);

  // The held Mysterious Treasure can discard the redundant Regidrago V and search
  // VSTAR. Crispin then attaches one Energy and puts the other into hand while the
  // manual attachment remains, whereas Steven's Resolve ends the turn:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://github.com/FlareZ123/pokemon-sims/issues/774
  if (sim::EngineTestAccess::should_play_steven(engine)) {
    throw std::runtime_error("Steven should yield to the proven Crispin-first route.");
  }
  if (sim::EngineTestAccess::choose_supporter_after_search_started(engine) !=
      sim::Card::Crispin) {
    throw std::runtime_error("Wonder Tag should select Crispin for the faster route.");
  }
}

void test_unpayable_mysterious_keeps_steven() {
  const sim::Scenario scenario{"issue-774-unpayable", sim::DciProfile::StrictJit,
                               sim::LockMode::TurnTwoItem, false, 4};
  std::mt19937_64 rng{775};
  sim::State state = exact_k1_state();
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Powerglass,
                sim::Card::Guzma, sim::Card::ProfessorTuro};
  sim::Engine engine = make_engine(scenario, std::move(state), rng);
  if (!sim::EngineTestAccess::should_play_steven(engine)) {
    throw std::runtime_error("An unpayable Mysterious Treasure must not suppress Steven.");
  }
}

void test_prized_vstar_keeps_steven() {
  const sim::Scenario scenario{"issue-774-prized-vstar", sim::DciProfile::StrictJit,
                               sim::LockMode::TurnTwoItem, false, 4};
  std::mt19937_64 rng{776};
  sim::State state = exact_k1_state();
  state.deck.erase(state.deck.begin());
  state.prizes.push_back(sim::Card::RegidragoVstar);
  sim::Engine engine = make_engine(scenario, std::move(state), rng);
  if (!sim::EngineTestAccess::should_play_steven(engine)) {
    throw std::runtime_error("A known-prized VSTAR must preserve Steven's broader search.");
  }
}

void test_other_lock_and_active_states_keep_steven() {
  {
    const sim::Scenario scenario{"issue-774-full-item", sim::DciProfile::StrictJit,
                                 sim::LockMode::FullItem, false, 4};
    std::mt19937_64 rng{777};
    sim::Engine engine = make_engine(scenario, exact_k1_state(), rng);
    if (!sim::EngineTestAccess::should_play_steven(engine)) {
      throw std::runtime_error("Full Item lock must keep Steven because Treasure is locked.");
    }
  }
  {
    const sim::Scenario scenario{"issue-774-basic-active", sim::DciProfile::StrictJit,
                                 sim::LockMode::TurnTwoItem, false, 4};
    std::mt19937_64 rng{778};
    sim::State state = exact_k1_state();
    state.active = sim::Pokemon{sim::Card::Oricorio, 0};
    sim::Engine engine = make_engine(scenario, std::move(state), rng);
    if (!sim::EngineTestAccess::should_play_steven(engine)) {
      throw std::runtime_error("Crispin should not displace Steven without an Active Regidrago V.");
    }
  }
}

}  // namespace

int main() {
  test_crispin_preempts_steven_for_exact_route();
  test_unpayable_mysterious_keeps_steven();
  test_prized_vstar_keeps_steven();
  test_other_lock_and_active_states_keep_steven();
  return 0;
}
