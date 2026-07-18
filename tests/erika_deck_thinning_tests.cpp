#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool play_blender(Engine& engine) { return engine.play_brilliant_blender(); }
  static bool play_burnet(Engine& engine) { return engine.play_professor_burnet(); }
};
}  // namespace sim

namespace {
bool contains(const std::vector<sim::Card>& cards, sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(const std::string& label, sim::DciProfile dci, std::uint64_t seed)
      : scenario{label, dci, sim::LockMode::None, false, 4},
        recipe(sim::baseline_recipe()),
        rng(seed),
        engine(scenario, recipe, rng) {}
};

sim::State blender_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1, sim::Tool::None};
  state.hand = {sim::Card::BrilliantBlender, sim::Card::TateLiza, sim::Card::Grass};
  state.deck = {sim::Card::MegaDragonite, sim::Card::ErikasInvitation,
                sim::Card::Arven, sim::Card::QuickBall,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

sim::State burnet_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0, sim::Tool::None};
  state.hand = {sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::MegaDragonite, sim::Card::ErikasInvitation,
                sim::Card::Arven, sim::Card::QuickBall,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

void require_erika_thinned(const sim::State& state, const char* action) {
  if (contains(state.deck, sim::Card::ErikasInvitation) ||
      !contains(state.discard, sim::Card::ErikasInvitation)) {
    throw std::runtime_error(std::string(action) + " should discard Erika.");
  }
  if (!contains(state.deck, sim::Card::Arven) ||
      !contains(state.deck, sim::Card::QuickBall)) {
    throw std::runtime_error(std::string(action) + " should preserve live connectors.");
  }
}

void test_blender() {
  Fixture fixture{"issue-881-blender", sim::DciProfile::NoDiscardControl, 88101};
  sim::EngineTestAccess::set_state(fixture.engine, blender_state());
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Brilliant Blender may discard up to five deck cards. Spare selections before a
  // legal Tate & Liza refresh remove the opponent-dependent inert Erika singleton:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/sv3pt5-160
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/881
  if (!sim::EngineTestAccess::play_blender(fixture.engine)) {
    throw std::runtime_error("Brilliant Blender should play.");
  }
  require_erika_thinned(sim::EngineTestAccess::state(fixture.engine), "Blender");
}

void test_burnet() {
  Fixture fixture{"issue-881-burnet", sim::DciProfile::NoDiscardControl, 88102};
  sim::EngineTestAccess::set_state(fixture.engine, burnet_state());
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Burnet's second legal discard changes Celestial Roar's Energy hit from 90% to
  // 100% in this exact state while preserving live connectors:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/sv3pt5-160
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/881
  if (!sim::EngineTestAccess::play_burnet(fixture.engine)) {
    throw std::runtime_error("Professor Burnet should play.");
  }
  require_erika_thinned(sim::EngineTestAccess::state(fixture.engine), "Burnet");
}

void test_profile_boundaries() {
  for (sim::DciProfile dci :
       {sim::DciProfile::StrictJit, sim::DciProfile::MatchupFlexJit}) {
    Fixture fixture{"issue-881-control", dci, 88103};
    sim::EngineTestAccess::set_state(fixture.engine, blender_state());
    sim::EngineTestAccess::set_deck_seen(fixture.engine);

    // This thinning remains exclusive to no-discard-control:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // https://github.com/FlareZ123/pokemon-sims/issues/881
    if (!sim::EngineTestAccess::play_blender(fixture.engine)) {
      throw std::runtime_error("Blender should remain legal in the control.");
    }
    const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
    if (!contains(after.deck, sim::Card::ErikasInvitation) ||
        contains(after.discard, sim::Card::ErikasInvitation)) {
      throw std::runtime_error("Other DCI profiles must retain Erika.");
    }
  }
}
}  // namespace

int main() {
  try {
    test_blender();
    test_burnet();
    test_profile_boundaries();
    std::cout << "Erika deck-thinning tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
