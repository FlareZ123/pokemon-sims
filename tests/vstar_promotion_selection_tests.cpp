#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
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
  static bool play_tate_switch(Engine& engine) { return engine.play_tate_switch(); }
  static bool bench_from_hand(Engine& engine, const Card card, const bool resolve_entry) {
    return engine.bench_from_hand(card, resolve_entry);
  }
  static void play_basics_from_hand(Engine& engine) { engine.play_basics_from_hand(); }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
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

void test_tapu_tate_connector_requires_ready_promotion_target() {
  {
    std::mt19937_64 rng{4061};
    sim::Engine engine(test_scenario(), test_recipe(), rng);
    sim::State state;
    state.turn = 2;
    state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
    state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1, sim::Tool::None}};
    state.hand = {sim::Card::TapuLeleGX};
    state.deck = {sim::Card::TateLiza};
    state.discard = {sim::Card::MegaDragonite};
    state.discarded_this_turn = {sim::Card::MegaDragonite};
    sim::EngineTestAccess::set_state(engine, std::move(state));

    // Wonder Tag can search Tate & Liza, but Tate only switches Pokémon. The promoted
    // Regidrago VSTAR still cannot pay Apex Dragon's GGF cost, so preserve Tapu and the
    // final Bench slot instead of spending them on an incomplete route:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    sim::EngineTestAccess::play_basics_from_hand(engine);
    const sim::State& after = sim::EngineTestAccess::state(engine);
    expect(contains(after.hand, sim::Card::TapuLeleGX),
           "Tapu Lele-GX must remain in hand when Tate would promote an incomplete VSTAR.");
    expect(after.bench.size() == 1U && !contains(after.hand, sim::Card::TateLiza),
           "The incomplete Tate route must preserve the Bench slot and leave Tate in deck.");
    expect(contains(after.deck, sim::Card::TateLiza),
           "Wonder Tag must not spend Tate on an Energy-incomplete promotion target.");
  }

  {
    std::mt19937_64 rng{4062};
    sim::Engine engine(test_scenario(), test_recipe(), rng);
    sim::State state;
    state.turn = 2;
    state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
    state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None}};
    state.hand = {sim::Card::TapuLeleGX};
    state.deck = {sim::Card::TateLiza};
    state.discard = {sim::Card::MegaDragonite};
    state.discarded_this_turn = {sim::Card::MegaDragonite};
    sim::EngineTestAccess::set_state(engine, std::move(state));

    // A complete GGF Benched VSTAR makes the Wonder Tag into Tate route live. Tapu may
    // take the Bench slot, Tate may switch that attacker Active, and the strict-JIT
    // payload remains ready for Apex Dragon:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    sim::EngineTestAccess::play_basics_from_hand(engine);
    expect(contains(sim::EngineTestAccess::state(engine).hand, sim::Card::TateLiza),
           "A GGF promotion target should still Bench Tapu Lele-GX and fetch Tate & Liza.");
    expect(sim::EngineTestAccess::play_tate_switch(engine),
           "The fetched Tate & Liza should promote the complete Benched VSTAR.");
    const sim::State& after = sim::EngineTestAccess::state(engine);
    expect(after.active && after.active->card == sim::Card::RegidragoVstar &&
               after.active->grass == 2 && after.active->fire == 1 &&
               sim::EngineTestAccess::payload_ready(engine),
           "The complete Wonder Tag route must reach the modeled ready state.");
  }
}

void test_wonder_tag_skips_tate_for_incomplete_target() {
  std::mt19937_64 rng{4063};
  sim::Engine engine(test_scenario(), test_recipe(), rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1, sim::Tool::None}};
  state.hand = {sim::Card::TapuLeleGX};
  state.deck = {sim::Card::TateLiza, sim::Card::Serena};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Even when Tapu is Benched by another legal route, Wonder Tag should not select
  // Tate for an incomplete promotion target while another Supporter remains available:
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::bench_from_hand(engine, sim::Card::TapuLeleGX, true),
         "The focused selector test should legally Bench Tapu Lele-GX.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::Serena) && contains(after.deck, sim::Card::TateLiza),
         "Wonder Tag must skip Tate for an incomplete VSTAR and select the live fallback Supporter.");
}

}  // namespace

int main() {
  try {
    test_latias_promotes_complete_vstar_over_first_match();
    test_tate_promotes_complete_vstar_over_first_match();
    test_incomplete_fallback_uses_greatest_ggf_progress();
    test_tapu_tate_connector_requires_ready_promotion_target();
    test_wonder_tag_skips_tate_for_incomplete_target();
    std::cout << "VSTAR promotion selection tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
