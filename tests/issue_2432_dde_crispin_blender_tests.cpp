#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = false;
  }
  static bool route_available(Engine& engine) {
    return engine.issue_1873_blender_crispin_preempts_gladion();
  }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
  static bool play_blender(Engine& engine) {
    return engine.play_brilliant_blender();
  }
  static bool pays_apex(const Engine& engine, const Pokemon& pokemon) {
    return engine.pays_apex_energy_cost(pokemon);
  }
  static bool payload_ready(const Engine& engine) {
    return engine.payload_ready();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"issue-2432", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2432};
  sim::Engine engine{scenario, recipe, rng};
};

void require(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state(const int grass, const int fire, const int dde) {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, grass, fire,
                              sim::Tool::None, dde};
  state.manual_energy_used = true;
  state.hand = {sim::Card::Crispin, sim::Card::Gladion,
                sim::Card::BrilliantBlender};
  state.deck = {sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::GoodraVstar};
  state.prizes = {sim::Card::MysteriousTreasure, sim::Card::RegidragoV,
                  sim::Card::HisuianHeavyBall, sim::Card::ProfessorBurnet,
                  sim::Card::Appletun, sim::Card::DialgaGX};
  return state;
}

void test_dde_grass_shadow_preempts_gladion() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, route_state(0, 0, 1));

  // One attached DDE supplies two units of every Energy type to this Dragon.
  // Crispin therefore needs only one Basic Energy and currently attaches Grass;
  // Blender supplies the current-turn Dragon payload. The semantic GGF proof must
  // preserve this complete Supporter route instead of letting Gladion consume it.
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Supporter, Item, search, discard, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2432
  require(sim::EngineTestAccess::route_available(fixture.engine),
          "DDE plus one-Basic Crispin-Blender shadow did not preempt Gladion.");

  sim::EngineTestAccess::choose_supporter(fixture.engine);
  const sim::State& after_crispin = sim::EngineTestAccess::state(fixture.engine);
  require(after_crispin.supporter_used,
          "Crispin did not consume the Supporter action.");
  require(after_crispin.active && after_crispin.active->double_dragon == 1 &&
              after_crispin.active->grass == 1 && after_crispin.active->fire == 0,
          "Crispin did not produce the expected DDE plus Grass state.");
  require(sim::EngineTestAccess::pays_apex(fixture.engine, *after_crispin.active),
          "DDE plus Grass was not recognized as Apex Dragon payment.");
  require(std::find(after_crispin.hand.begin(), after_crispin.hand.end(),
                    sim::Card::Gladion) != after_crispin.hand.end(),
          "The DDE-aware route failed to preserve Gladion.");
  require(sim::EngineTestAccess::play_blender(fixture.engine),
          "Brilliant Blender did not resolve after Crispin.");
  require(sim::EngineTestAccess::payload_ready(fixture.engine),
          "Brilliant Blender did not establish the strict-JIT payload.");
}

void test_dde_fire_semantic_symmetry() {
  Fixture fixture;
  const sim::Pokemon dde_fire{sim::Card::RegidragoVstar, 1, 0, 1,
                              sim::Tool::None, 1};

  // The route predicate delegates to the shared Apex-payment rule. Preserve the
  // symmetric DDE + Fire payment explicitly so future legal Crispin ordering
  // cannot regress to raw Basic counters.
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // DDE enhancement contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2432
  require(sim::EngineTestAccess::pays_apex(fixture.engine, dde_fire),
          "DDE plus Fire was not recognized as Apex Dragon payment.");
}

void test_original_basic_energy_route_remains_live() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, route_state(1, 1, 0));

  // The original issue-1873 Basic-Energy Supporter-contention route remains a
  // positive control while readiness delegates to the semantic predicate.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Original confirmed route: https://github.com/FlareZ123/pokemon-sims/issues/1873
  // DDE regression: https://github.com/FlareZ123/pokemon-sims/issues/2432
  require(sim::EngineTestAccess::route_available(fixture.engine),
          "The original Basic-Energy Crispin-Blender route regressed.");
}

}  // namespace

int main() {
  test_dde_grass_shadow_preempts_gladion();
  test_dde_fire_semantic_symmetry();
  test_original_basic_energy_route_remains_live();
  return 0;
}
