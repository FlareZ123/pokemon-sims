#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_known_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }

  static void set_k0_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = false;
  }

  static bool issue_3173_route_visible(const Engine& engine) {
    return engine.issue_3173_steven_route_available();
  }
};
}  // namespace sim

namespace {

sim::State route_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::Oricorio, turn - 1, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, turn, 1, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::StevensResolve, sim::Card::MysteriousTreasure,
                sim::Card::Gladion, sim::Card::TapuLeleGX};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Crispin,
                sim::Card::LatiasEx, sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite, sim::Card::QuickBall};
  state.prizes = {sim::Card::FieldBlower, sim::Card::Serena,
                  sim::Card::Arven, sim::Card::QuickBall,
                  sim::Card::Grass, sim::Card::Fire};
  state.manual_energy_used = true;
  return state;
}

bool visible(const sim::DciProfile dci, const sim::LockMode lock,
             const bool going_first, const int turn, const int max_turn,
             const bool k1 = true, const bool bench_full = false,
             const bool retreat_used = false, const bool keep_treasure = true) {
  const sim::Scenario scenario{"issue-3173-semantic", dci, lock,
                               going_first, max_turn};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{3173};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = route_state(turn);
  state.retreat_used = retreat_used;
  if (!keep_treasure) {
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                                 sim::Card::MysteriousTreasure),
                     state.hand.end());
  }
  if (bench_full) {
    for (int i = 0; i < 4; ++i) {
      state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, turn});
    }
  }
  if (k1) {
    sim::EngineTestAccess::set_known_state(engine, std::move(state));
  } else {
    sim::EngineTestAccess::set_k0_state(engine, std::move(state));
  }
  return sim::EngineTestAccess::issue_3173_route_visible(engine);
}

void admits_semantic_equivalents() {
  // Both JIT profiles use the same ready-turn payload timing; Mysterious Treasure
  // supplies that discard on the finishing turn. Seat identity and absolute turn
  // are not printed constraints of this sequence.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3173
  if (!visible(sim::DciProfile::StrictJit, sim::LockMode::None, true, 2, 3) ||
      !visible(sim::DciProfile::MatchupFlexJit, sim::LockMode::None, true, 2, 3) ||
      !visible(sim::DciProfile::StrictJit, sim::LockMode::None, false, 2, 3) ||
      !visible(sim::DciProfile::StrictJit, sim::LockMode::None, true, 3, 4)) {
    throw std::runtime_error("A semantically equivalent #3173 route was rejected.");
  }
}

void blocks_real_constraints() {
  // Treasure requires Item legality; Steven/Crispin require Supporter legality;
  // Skyliner is Latias ex's Rule Box Ability; the route also requires K1, Bench
  // space, an unused Retreat action, its held connector, and one future turn.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Official turn, Trainer, Ability, Bench, and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 and lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3173
  if (visible(sim::DciProfile::NoDiscardControl, sim::LockMode::None, true, 2, 3) ||
      visible(sim::DciProfile::StrictJit, sim::LockMode::TurnTwoItem, true, 1, 3) ||
      visible(sim::DciProfile::StrictJit, sim::LockMode::FullSupporter, true, 2, 3) ||
      visible(sim::DciProfile::StrictJit, sim::LockMode::FullRuleBoxAbility, true, 2, 3) ||
      visible(sim::DciProfile::StrictJit, sim::LockMode::None, true, 2, 3, false) ||
      visible(sim::DciProfile::StrictJit, sim::LockMode::None, true, 2, 3, true, true) ||
      visible(sim::DciProfile::StrictJit, sim::LockMode::None, true, 2, 3, true, false, true) ||
      visible(sim::DciProfile::StrictJit, sim::LockMode::None, true, 2, 3, true, false, false, false) ||
      visible(sim::DciProfile::StrictJit, sim::LockMode::None, true, 3, 3)) {
    throw std::runtime_error("A real #3173 route constraint failed to block admission.");
  }
}

}  // namespace

int main() {
  admits_semantic_equivalents();
  blocks_real_constraints();
  return 0;
}
