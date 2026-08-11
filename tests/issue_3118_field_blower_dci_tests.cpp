#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
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

  static std::optional<Card> direct_vessel_cost(Engine& engine) {
    return engine.issue_2437_direct_vessel_cost();
  }

  static bool play_direct_vessel_route(Engine& engine) {
    return engine.play_issue_2437_direct_dde_vessel_energy_route();
  }

  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {
void require(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::Scenario scenario(const char* label, const sim::LockMode locks) {
  return sim::Scenario{label, sim::DciProfile::StrictJit, locks, false, 5};
}

sim::State dde_vessel_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 0, 0,
                              sim::Tool::None, 1};
  state.hand = {sim::Card::EarthenVessel, sim::Card::FieldBlower};
  state.deck = {sim::Card::Grass};
  state.prizes = {sim::Card::QuickBall, sim::Card::RegidragoV,
                  sim::Card::RegidragoVstar, sim::Card::Gladion,
                  sim::Card::Crispin, sim::Card::MysteriousTreasure};
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{3118};
  sim::Engine engine;

  Fixture(const char* label, const sim::LockMode locks)
      : scenario_value(scenario(label, locks)),
        engine(scenario_value, recipe, rng) {}
};

void supporter_lock_allows_dead_field_blower_cost() {
  Fixture fixture{"strict-jit-supporter-lock/go-second",
                  sim::LockMode::FullSupporter};
  sim::EngineTestAccess::set_state(fixture.engine, dde_vessel_state());

  // Supporter lock does not disable Item play, Earthen Vessel's printed discard
  // and search, or the later manual Energy attachment. With no Path, Garbodor,
  // or spent-FSS/Powerglass target, Field Blower is DCI-safe route-replaced fodder:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced Item/search/discard/attachment procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3118
  require(sim::EngineTestAccess::direct_vessel_cost(fixture.engine) ==
              sim::Card::FieldBlower,
          "Supporter lock rejected dead Field Blower as the direct Vessel cost.");
  require(sim::EngineTestAccess::play_direct_vessel_route(fixture.engine),
          "Supporter lock blocked the legal DDE direct-Vessel route.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  require(contains(after.discard, sim::Card::FieldBlower),
          "Direct Vessel did not spend the dead Field Blower.");
  require(contains(after.hand, sim::Card::Grass),
          "Direct Vessel did not search the one-attachment Apex completion.");
}

void path_value_preserves_field_blower() {
  Fixture fixture{"strict-jit-rulebox-ability-lock/go-second",
                  sim::LockMode::FullRuleBoxAbility};
  sim::State state = dde_vessel_state();
  state.vstar_power_used = false;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // An unresolved in-play VSTAR gives Path removal current setup value because
  // Field Blower restores the Rule Box Legacy Star Ability. That live discrete
  // value keeps Field Blower out of Vessel's DCI fallback:
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Scenario-lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3118
  require(sim::EngineTestAccess::direct_vessel_cost(fixture.engine) !=
              sim::Card::FieldBlower,
          "A live Path-removal route was discarded as Vessel fodder.");
}

void garbodor_value_preserves_field_blower() {
  Fixture fixture{"garbodor-shake-ability-lock/go-second",
                  sim::LockMode::FullRuleBoxAbility};
  sim::State state = dde_vessel_state();
  state.vstar_power_used = false;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Garbotoxin suppresses the in-play Legacy Star route until Field Blower removes
  // Garbodor's Tool, so the same card retains current setup value and DCI stays low:
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
  // Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3118
  require(sim::EngineTestAccess::direct_vessel_cost(fixture.engine) !=
              sim::Card::FieldBlower,
          "A live Garbodor-removal route was discarded as Vessel fodder.");
}

void spent_fss_powerglass_value_preserves_field_blower() {
  Fixture fixture{"strict-jit-supporter-lock/go-second",
                  sim::LockMode::FullSupporter};
  sim::State state = dde_vessel_state();
  state.active->tool = sim::Tool::ForestSealStone;
  state.vstar_power_used = true;
  state.hand.push_back(sim::Card::Powerglass);
  state.discard.push_back(sim::Card::Grass);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // A spent Forest Seal Stone can occupy the Active Tool slot while Powerglass can
  // recover a missing Basic from the discard pile. Field Blower must be preserved
  // when removing that Tool unlocks the live Energy route:
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Dynamic DCI / decision priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3118
  require(sim::EngineTestAccess::direct_vessel_cost(fixture.engine) !=
              sim::Card::FieldBlower,
          "A live spent-FSS/Powerglass route was discarded as Vessel fodder.");
}
}  // namespace

int main() {
  try {
    supporter_lock_allows_dead_field_blower_cost();
    path_value_preserves_field_blower();
    garbodor_value_preserves_field_blower();
    spent_fss_powerglass_value_preserves_field_blower();
    std::cout << "Issue 3118 Field Blower DCI tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
