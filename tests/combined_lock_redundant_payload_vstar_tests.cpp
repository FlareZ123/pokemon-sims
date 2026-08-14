#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }

  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }

  static bool can_spend_redundant_payload_for_locked_vstar_search(
      const Engine& engine) {
    return engine.can_spend_redundant_payload_for_locked_vstar_search();
  }
};
}  // namespace sim

sim::State locked_vstar_search_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.hand = {
      sim::Card::MegaDragonite,
      sim::Card::Dragapult,
      sim::Card::ProfessorBurnet,
  };
  state.deck = {
      sim::Card::RegidragoVstar,
      sim::Card::Appletun,
  };
  return state;
}

bool route_is_available(const sim::LockMode lock_mode, const std::uint64_t seed) {
  // The scheduled T2 Item-lock predicate is shared by TurnTwoItem and FullCombined:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
  // Mysterious Treasure may discard one redundant Dragon to search Regidrago VSTAR,
  // while Professor Burnet remains a Supporter outlet for the surviving payload axis:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/3782
  const sim::Scenario scenario{
      "issue-3782", sim::DciProfile::StrictJit, lock_mode, false, 3};
  std::mt19937_64 rng(seed);
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, locked_vstar_search_state());
  sim::EngineTestAccess::set_deck_seen(engine);
  return sim::EngineTestAccess::can_spend_redundant_payload_for_locked_vstar_search(
      engine);
}

int main() {
  if (!route_is_available(sim::LockMode::TurnTwoItem, 378201)) {
    throw std::runtime_error(
        "The confirmed #925 redundant-payload route must remain available under TurnTwoItem.");
  }
  if (!route_is_available(sim::LockMode::FullCombined, 378202)) {
    throw std::runtime_error(
        "FullCombined must admit the same scheduled T2 Item-lock redundant-payload route.");
  }
  if (route_is_available(sim::LockMode::FullRuleBoxAbility, 378203)) {
    throw std::runtime_error(
        "Rule Box Ability lock alone must not activate the scheduled T2 Item-lock exception.");
  }
  if (route_is_available(sim::LockMode::None, 378204)) {
    throw std::runtime_error(
        "An unlocked scenario must not activate the scheduled T2 Item-lock exception.");
  }
}
