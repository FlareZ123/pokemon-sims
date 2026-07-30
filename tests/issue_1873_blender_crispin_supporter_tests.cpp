#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool known = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = known;
    engine.prizes_revealed_ = known;
  }
  static bool route_available(Engine& engine) {
    return engine.issue_1873_blender_crispin_preempts_gladion();
  }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
  static bool play_blender(Engine& engine) {
    return engine.play_brilliant_blender();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1873};
  sim::Engine engine;

  Fixture(const sim::DciProfile dci = sim::DciProfile::StrictJit,
          const sim::LockMode locks = sim::LockMode::None)
      : scenario{"issue-1873", dci, locks, false, 5},
        engine{scenario, recipe, rng} {}
};

sim::State exact_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.manual_energy_used = true;
  state.hand = {sim::Card::Crispin, sim::Card::Gladion,
                sim::Card::BrilliantBlender, sim::Card::EarthenVessel,
                sim::Card::Fire};
  state.deck = {sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::GoodraVstar};
  state.prizes = {sim::Card::MysteriousTreasure, sim::Card::RegidragoV,
                  sim::Card::HisuianHeavyBall, sim::Card::ProfessorBurnet,
                  sim::Card::Appletun, sim::Card::DialgaGX};
  return state;
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_exact_route_uses_crispin_then_blender() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, exact_state());
  expect(sim::EngineTestAccess::route_available(fixture.engine),
         "The exact public K1 Blender-Crispin route must be available.");

  // Crispin must consume the Supporter play before Gladion because it completes
  // the final Grass, while the protected ACE SPEC supplies the same-turn payload:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // https://github.com/FlareZ123/pokemon-sims/issues/1873
  sim::EngineTestAccess::choose_supporter(fixture.engine);
  const sim::State& after_crispin = sim::EngineTestAccess::state(fixture.engine);
  expect(after_crispin.supporter_used,
         "Crispin must consume the available Supporter action.");
  expect(after_crispin.active && after_crispin.active->grass >= 2 &&
             after_crispin.active->fire >= 1,
         "Crispin must complete GGF in the exact state.");
  expect(std::find(after_crispin.hand.begin(), after_crispin.hand.end(),
                   sim::Card::Gladion) != after_crispin.hand.end(),
         "The stronger route must preserve Gladion.");
  expect(sim::EngineTestAccess::play_blender(fixture.engine),
         "The protected Brilliant Blender must remain playable.");
  expect(!sim::EngineTestAccess::state(fixture.engine)
              .discarded_this_turn.empty(),
         "Brilliant Blender must establish a same-turn discard payload.");
}

void test_k0_rejects_route() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, exact_state(), false);
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The K1-specific override must reject K0.");
}

void test_item_lock_rejects_route() {
  Fixture fixture(sim::DciProfile::StrictJit, sim::LockMode::FullItem);
  sim::EngineTestAccess::set_state(fixture.engine, exact_state());
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "Item lock must block the Brilliant Blender route.");
}

void test_supporter_lock_rejects_route() {
  Fixture fixture(sim::DciProfile::StrictJit, sim::LockMode::FullSupporter);
  sim::EngineTestAccess::set_state(fixture.engine, exact_state());
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "Supporter lock must block Crispin.");
}

void test_one_type_crispin_after_attachment_rejects_route() {
  sim::State state = exact_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::Fire),
                   state.deck.end());
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "One searchable Energy type cannot attach after the manual action is spent.");
}

void test_missing_payload_rejects_route() {
  sim::State state = exact_state();
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route_available(fixture.engine),
         "The override must require a legal Blender payload target.");
}

void test_missing_connector_rejects_route() {
  sim::State no_blender = exact_state();
  no_blender.hand.erase(
      std::find(no_blender.hand.begin(), no_blender.hand.end(),
                sim::Card::BrilliantBlender));
  Fixture blender_fixture;
  sim::EngineTestAccess::set_state(blender_fixture.engine,
                                   std::move(no_blender));
  expect(!sim::EngineTestAccess::route_available(blender_fixture.engine),
         "The override must require held Brilliant Blender.");

  sim::State no_crispin = exact_state();
  no_crispin.hand.erase(std::find(no_crispin.hand.begin(), no_crispin.hand.end(),
                                  sim::Card::Crispin));
  Fixture crispin_fixture;
  sim::EngineTestAccess::set_state(crispin_fixture.engine,
                                   std::move(no_crispin));
  expect(!sim::EngineTestAccess::route_available(crispin_fixture.engine),
         "The override must require held Crispin.");
}

void test_spent_supporter_and_non_strict_reject_route() {
  sim::State state = exact_state();
  state.supporter_used = true;
  Fixture spent_fixture;
  sim::EngineTestAccess::set_state(spent_fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::route_available(spent_fixture.engine),
         "A spent Supporter action must reject the route.");

  Fixture no_control_fixture(sim::DciProfile::NoDiscardControl);
  sim::EngineTestAccess::set_state(no_control_fixture.engine, exact_state());
  expect(!sim::EngineTestAccess::route_available(no_control_fixture.engine),
         "The override must remain JIT-specific.");
}

}  // namespace

int main() {
  test_exact_route_uses_crispin_then_blender();
  test_k0_rejects_route();
  test_item_lock_rejects_route();
  test_supporter_lock_rejects_route();
  test_one_type_crispin_after_attachment_rejects_route();
  test_missing_payload_rejects_route();
  test_missing_connector_rejects_route();
  test_spent_supporter_and_non_strict_reject_route();
  return 0;
}
