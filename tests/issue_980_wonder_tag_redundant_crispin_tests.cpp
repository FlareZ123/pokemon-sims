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
  static bool needs_tapu_connector(Engine& engine) {
    return engine.needs_tapu_connector();
  }
};

}  // namespace sim

namespace {

sim::State complete_held_route_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0,
                              sim::Tool::ForestSealStone};
  state.bench = {
      sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::RegidragoV, 2, 0, 0, sim::Tool::None},
  };
  state.hand = {
      sim::Card::MysteriousTreasure,
      sim::Card::Fire,
      sim::Card::TapuLeleGX,
      sim::Card::ProfessorTuro,
      sim::Card::GoodraVstar,
  };
  state.deck = {
      sim::Card::RegidragoVstar,
      sim::Card::Crispin,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::LatiasEx,
  };
  return state;
}

bool needs_tapu(sim::State state) {
  const sim::Scenario scenario{"issue-980", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{980};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return sim::EngineTestAccess::needs_tapu_connector(engine);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

bool benched(const sim::State& state, const sim::Card card) {
  return std::any_of(state.bench.begin(), state.bench.end(),
                     [card](const sim::Pokemon& pokemon) {
                       return pokemon.card == card;
                     });
}

void test_seed_331_full_route_skips_wonder_tag() {
  const sim::Scenario scenario{"strict-jit/go-first", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 3};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{331};
  sim::Engine engine(scenario, recipe, rng);
  const sim::TrialOutcome outcome = engine.run();
  const sim::State& state = engine.state();

  // Seed 331 reaches the same earliest T3 readiness through held Fire and
  // Mysterious Treasure, while Tapu Lele-GX and Crispin retain their discrete value:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh11-136
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/980
  if (outcome.first_ready_turn != 3 || benched(state, sim::Card::TapuLeleGX) ||
      !contains(state.hand, sim::Card::TapuLeleGX) ||
      contains(state.hand, sim::Card::Crispin) ||
      !contains(state.discard, sim::Card::MysteriousTreasure) ||
      !contains(state.discard, sim::Card::GoodraVstar)) {
    throw std::runtime_error(
        "Seed 331 must preserve Tapu and Crispin while retaining T3 readiness.");
  }
}

void test_complete_held_route_suppresses_wonder_tag() {
  // Fire plus the unused manual attachment completes GGF. Mysterious Treasure
  // discards Hisuian Goodra VSTAR and searches Regidrago VSTAR, so Wonder Tag's
  // Crispin target is unused and Tapu Lele-GX should remain in hand:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh11-136
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/980
  if (needs_tapu(complete_held_route_state())) {
    throw std::runtime_error(
        "The complete held route must suppress redundant Wonder Tag into Crispin.");
  }
}

void test_missing_held_energy_preserves_wonder_tag() {
  sim::State state = complete_held_route_state();
  std::erase(state.hand, sim::Card::Fire);
  if (!needs_tapu(std::move(state))) {
    throw std::runtime_error(
        "Wonder Tag must remain live when Crispin is required for Energy.");
  }
}

void test_used_manual_attachment_preserves_wonder_tag() {
  sim::State state = complete_held_route_state();
  state.manual_energy_used = true;
  if (!needs_tapu(std::move(state))) {
    throw std::runtime_error(
        "Wonder Tag must remain live after the manual attachment is spent.");
  }
}

void test_missing_treasure_target_preserves_wonder_tag() {
  sim::State state = complete_held_route_state();
  std::erase(state.deck, sim::Card::RegidragoVstar);
  if (!needs_tapu(std::move(state))) {
    throw std::runtime_error(
        "Wonder Tag must remain live when Treasure cannot search the VSTAR.");
  }
}

void test_missing_payload_cost_preserves_wonder_tag() {
  sim::State state = complete_held_route_state();
  std::erase(state.hand, sim::Card::GoodraVstar);
  if (!needs_tapu(std::move(state))) {
    throw std::runtime_error(
        "Wonder Tag must remain live without a Dragon payload cost.");
  }
}

void test_fresh_active_regidrago_preserves_wonder_tag() {
  sim::State state = complete_held_route_state();
  state.active->entered_turn = state.turn;
  if (!needs_tapu(std::move(state))) {
    throw std::runtime_error(
        "Wonder Tag must remain live when the Active Regidrago cannot evolve.");
  }
}

}  // namespace

int main() {
  try {
    test_seed_331_full_route_skips_wonder_tag();
    test_complete_held_route_suppresses_wonder_tag();
    test_missing_held_energy_preserves_wonder_tag();
    test_used_manual_attachment_preserves_wonder_tag();
    test_missing_treasure_target_preserves_wonder_tag();
    test_missing_payload_cost_preserves_wonder_tag();
    test_fresh_active_regidrago_preserves_wonder_tag();
    std::cout << "Issue 980 Wonder Tag tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
