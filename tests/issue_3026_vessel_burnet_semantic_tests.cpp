#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool route_visible(const Engine& engine) {
    return engine.issue_1646_vessel_burnet_finish_visible();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::EarthenVessel,
                sim::Card::QuickBall};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::Dragapult,
                sim::Card::RegidragoV};
  state.prizes = {sim::Card::ForestSealStone, sim::Card::FieldBlower,
                  sim::Card::Oricorio, sim::Card::MegaDragonite,
                  sim::Card::Grass, sim::Card::QuickBall};
  return state;
}

bool visible_for(const sim::DciProfile dci, const sim::LockMode lock,
                 sim::State state, const int max_turn = 5) {
  const sim::Scenario scenario{"issue-3026", dci, lock, true, max_turn};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3026);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return sim::EngineTestAccess::route_visible(engine);
}

void test_shared_jit_rulebox_and_t2_admission() {
  // Earthen Vessel is an Item and Professor Burnet is a Supporter, so a Rule Box
  // Ability lock leaves these Trainer actions legal. StrictJit and MatchupFlexJit
  // share the repository's same-ready-turn payload requirement:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced legality procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Same-turn JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3026
  expect(visible_for(sim::DciProfile::MatchupFlexJit,
                     sim::LockMode::FullRuleBoxAbility, route_state(4)),
         "MatchupFlexJit Rule Box Ability lock hid the legal Vessel-Burnet route");

  // Neither printed Trainer effect has a T3 minimum. Once the current public/K1
  // state satisfies the route prerequisites, earliest-route policy admits T2:
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3026
  expect(visible_for(sim::DciProfile::StrictJit, sim::LockMode::None,
                     route_state(2)),
         "The legal T2 Vessel-Burnet route was suppressed by the historical turn witness");
}

void test_semantic_boundaries() {
  // NoDiscardControl is outside the same-turn JIT contract:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/3026
  expect(!visible_for(sim::DciProfile::NoDiscardControl, sim::LockMode::None,
                      route_state(3)),
         "NoDiscardControl incorrectly entered the same-turn-JIT Burnet route");

  // Earthen Vessel remains illegal through Item lock:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#full-item-lock
  // https://github.com/FlareZ123/pokemon-sims/issues/3026
  expect(!visible_for(sim::DciProfile::StrictJit, sim::LockMode::FullItem,
                      route_state(3)),
         "Item lock incorrectly admitted the Vessel-Burnet route");

  // Professor Burnet is a Supporter, so Supporter lock blocks the route:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // https://github.com/FlareZ123/pokemon-sims/issues/3026
  expect(!visible_for(sim::DciProfile::StrictJit, sim::LockMode::FullSupporter,
                      route_state(3)),
         "Supporter lock incorrectly admitted the Vessel-Burnet route");

  sim::State manual_spent = route_state(3);
  manual_spent.manual_energy_used = true;
  expect(!visible_for(sim::DciProfile::StrictJit, sim::LockMode::None,
                      std::move(manual_spent)),
         "The route ignored the spent manual Energy attachment");

  sim::State no_energy = route_state(3);
  no_energy.deck = {sim::Card::Fire, sim::Card::Dragapult,
                    sim::Card::RegidragoV};
  expect(!visible_for(sim::DciProfile::StrictJit, sim::LockMode::None,
                      std::move(no_energy)),
         "The route admitted a state without the completing Basic Energy");

  sim::State no_payload = route_state(3);
  no_payload.deck = {sim::Card::Grass, sim::Card::Fire,
                     sim::Card::RegidragoV};
  expect(!visible_for(sim::DciProfile::StrictJit, sim::LockMode::None,
                      std::move(no_payload)),
         "The route admitted a state without a searchable Dragon payload");

  expect(!visible_for(sim::DciProfile::StrictJit, sim::LockMode::None,
                      route_state(4), 3),
         "The route ignored the configured simulation horizon");
}

}  // namespace

int main() {
  test_shared_jit_rulebox_and_t2_admission();
  test_semantic_boundaries();
  return 0;
}
