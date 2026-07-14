#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool retreat_to_benched_vstar_with_latias(Engine& engine) {
    return engine.retreat_to_benched_vstar_with_latias();
  }
  static bool attach_manual(Engine& engine) { return engine.attach_manual(); }
  static bool play_tate_switch(Engine& engine) { return engine.play_tate_switch(); }
  static bool play_brilliant_blender(Engine& engine) {
    return engine.play_brilliant_blender();
  }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

const sim::Scenario& test_scenario() {
  static const sim::Scenario scenario{"vstar-promotion-selection", sim::DciProfile::StrictJit,
                                      sim::LockMode::None, false, 4};
  return scenario;
}

const sim::DeckRecipe& test_recipe() {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  return recipe;
}

void test_latias_promotes_complete_vstar_over_first_match() {
  std::mt19937_64 rng{4021};
  sim::Engine engine(test_scenario(), test_recipe(), rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1, sim::Tool::None},
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None},
      sim::Pokemon{sim::Card::LatiasEx, 1},
  };
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Skyliner removes the Basic Active's Retreat Cost, while Apex Dragon requires GGF:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::retreat_to_benched_vstar_with_latias(engine),
         "Latias should promote a Benched Regidrago VSTAR.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->card == sim::Card::RegidragoVstar &&
             after.active->grass == 2 && after.active->fire == 1,
         "Latias must promote the complete GGF VSTAR instead of the first incomplete VSTAR.");
  expect(after.retreat_used, "The Latias route must consume the once-per-turn retreat.");
}

void test_tate_promotes_complete_vstar_over_first_match() {
  std::mt19937_64 rng{4022};
  sim::Engine engine(test_scenario(), test_recipe(), rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1, sim::Tool::None},
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None},
  };
  state.hand = {sim::Card::TateLiza};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Tate & Liza may switch any Benched Pokémon Active. Spending the Supporter should
  // select the Regidrago VSTAR that already pays Apex Dragon's GGF cost:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::play_tate_switch(engine),
         "Tate & Liza should promote a Benched Regidrago VSTAR.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->card == sim::Card::RegidragoVstar &&
             after.active->grass == 2 && after.active->fire == 1,
         "Tate & Liza must promote the complete GGF VSTAR instead of the first incomplete VSTAR.");
  expect(after.supporter_used, "The Tate route must consume the Supporter play.");
}

void test_incomplete_fallback_uses_greatest_ggf_progress() {
  std::mt19937_64 rng{4023};
  sim::Engine engine(test_scenario(), test_recipe(), rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1, sim::Tool::None},
  };
  state.hand = {sim::Card::TateLiza};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Apex Dragon's printed GGF cost defines deterministic progress for incomplete candidates:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::play_tate_switch(engine),
         "Tate & Liza should still select an incomplete fallback when no VSTAR is complete.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->grass == 1 && after.active->fire == 1,
         "The fallback must choose the incomplete VSTAR with greater progress toward GGF.");
}

void test_tate_blender_attachment_uses_current_promotion_target() {
  std::mt19937_64 rng{416};
  sim::Engine engine(test_scenario(), test_recipe(), rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 2, 0, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 0, 1, sim::Tool::None},
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1, sim::Tool::None},
  };
  state.hand = {sim::Card::Grass, sim::Card::TateLiza, sim::Card::BrilliantBlender};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // The once-per-turn Energy attachment must advance the same best-powered Benched
  // Regidrago VSTAR that Tate & Liza will promote. Brilliant Blender can then create
  // the current-turn payload and Apex Dragon can attack for GGF:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  expect(sim::EngineTestAccess::attach_manual(engine),
         "The Tate-Blender route should attach the held Grass Energy.");
  const sim::State& after_attachment = sim::EngineTestAccess::state(engine);
  expect(after_attachment.bench[0].grass == 0 && after_attachment.bench[0].fire == 1,
         "The earlier weaker VSTAR must not receive the coordinated attachment.");
  expect(after_attachment.bench[1].grass == 2 && after_attachment.bench[1].fire == 1,
         "The current promotion target must receive Grass and reach GGF.");

  expect(sim::EngineTestAccess::play_tate_switch(engine),
         "Tate & Liza should promote the newly completed VSTAR.");
  expect(sim::EngineTestAccess::play_brilliant_blender(engine),
         "Brilliant Blender should discard the strict-JIT payload.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->card == sim::Card::RegidragoVstar &&
             after.active->grass == 2 && after.active->fire == 1,
         "Tate must promote the VSTAR completed by the coordinated attachment.");
  expect(sim::EngineTestAccess::payload_ready(engine),
         "The shared attachment and promotion target must reach the ready state.");
}

}  // namespace

int main() {
  try {
    test_latias_promotes_complete_vstar_over_first_match();
    test_tate_promotes_complete_vstar_over_first_match();
    test_incomplete_fallback_uses_greatest_ggf_progress();
    test_tate_blender_attachment_uses_current_promotion_target();
    std::cout << "VSTAR promotion selection tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
