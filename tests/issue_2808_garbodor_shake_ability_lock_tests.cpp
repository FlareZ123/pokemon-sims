#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static bool garbodor_locked(const Engine& engine) { return engine.garbodor_abilities_locked(); }
  static bool ability_available(const Engine& engine, Card card) {
    return engine.ability_available_for_pokemon_garbodor(card);
  }
  static bool play_field_blower(Engine& engine) { return engine.play_field_blower(); }
  static bool arven_blower_route_live(Engine& engine) {
    return engine.arven_garbodor_field_blower_route_live();
  }
  static bool play_arven(Engine& engine) { return engine.play_arven(); }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
  static void begin_turn(Engine& engine, int turn) { engine.begin_turn(turn); }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {
void expect(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
void pass(const char* name) { std::cout << "PASS issue-2808: " << name << '\n'; }

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  return sim::Engine(scenario, recipe, rng);
}

sim::State minimal_state(int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::RegidragoVstar,
                sim::Card::MegaDragonite, sim::Card::Crispin};
  return state;
}

void registry_and_timing() {
  const auto scenarios = sim::all_scenarios_with_garbodor();
  const auto has = [&scenarios](const std::string& label) {
    return std::any_of(scenarios.begin(), scenarios.end(), [&label](const sim::Scenario& s) {
      return s.label == label;
    });
  };
  expect(has("garbodor-shake-ability-lock/go-first"), "missing go-first Garbodor row");
  expect(has("garbodor-shake-ability-lock/go-second"), "missing go-second Garbodor row");

  // Boost Shake establishes Garbodor during the opponent's first turn:
  // https://api.pokemontcg.io/v2/cards/swsh7-142
  // https://api.pokemontcg.io/v2/cards/xy9-57
  std::mt19937_64 rng1{280801};
  const sim::Scenario first{"garbodor-shake-ability-lock/go-first",
                            sim::DciProfile::StrictJit, sim::LockMode::None, true, 5};
  sim::Engine first_engine = make_engine(first, rng1);
  auto first_state = minimal_state(1);
  sim::EngineTestAccess::set_state(first_engine, first_state);
  expect(!sim::EngineTestAccess::garbodor_locked(first_engine), "go-first T1 must be unlocked");
  expect(sim::EngineTestAccess::ability_available(first_engine, sim::Card::Oricorio),
         "Oricorio must be available go-first T1");
  first_state.turn = 2;
  sim::EngineTestAccess::set_state(first_engine, first_state);
  expect(sim::EngineTestAccess::garbodor_locked(first_engine), "go-first T2 must be locked");
  expect(!sim::EngineTestAccess::ability_available(first_engine, sim::Card::Oricorio),
         "Garbotoxin must suppress non-Rule-Box Pokemon Abilities");
  expect(!sim::EngineTestAccess::ability_available(first_engine, sim::Card::CrobatV),
         "Garbotoxin must suppress Pokemon V Abilities");

  std::mt19937_64 rng2{280802};
  const sim::Scenario second{"garbodor-shake-ability-lock/go-second",
                             sim::DciProfile::StrictJit, sim::LockMode::None, false, 5};
  sim::Engine second_engine = make_engine(second, rng2);
  sim::EngineTestAccess::set_state(second_engine, minimal_state(1));
  expect(sim::EngineTestAccess::garbodor_locked(second_engine), "go-second T1 must be locked");
  pass("registry and Boost Shake seat timing");
}

void field_blower_relief_and_relock() {
  std::mt19937_64 rng{280803};
  const sim::Scenario scenario{"garbodor-shake-ability-lock/go-second",
                               sim::DciProfile::StrictJit, sim::LockMode::None, false, 5};
  sim::Engine engine = make_engine(scenario, rng);
  auto state = minimal_state(1);
  state.hand = {sim::Card::FieldBlower, sim::Card::CrobatV};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  // Field Blower removes Garbodor's Tool; maximum-pressure modeling re-Tools next turn:
  // https://api.pokemontcg.io/v2/cards/sm2-125
  // https://api.pokemontcg.io/v2/cards/xy9-57
  // Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
  expect(sim::EngineTestAccess::play_field_blower(engine), "Field Blower must unlock live Dark Asset");
  expect(sim::EngineTestAccess::ability_available(engine, sim::Card::CrobatV),
         "Abilities must be available after Field Blower this turn");
  expect(sim::EngineTestAccess::state(engine).path_lock_removed, "temporary unlock marker missing");
  sim::EngineTestAccess::begin_turn(engine, 2);
  expect(!sim::EngineTestAccess::state(engine).path_lock_removed, "temporary unlock must reset");
  expect(sim::EngineTestAccess::garbodor_locked(engine), "Garbodor must relock next turn");
  pass("Field Blower current-turn relief and next-turn relock");
}

void item_lock_still_blocks_blower() {
  std::mt19937_64 rng{280804};
  const sim::Scenario scenario{"garbodor-shake-ability-lock/go-second",
                               sim::DciProfile::StrictJit, sim::LockMode::FullItem, false, 5};
  sim::Engine engine = make_engine(scenario, rng);
  auto state = minimal_state(1);
  state.hand = {sim::Card::FieldBlower, sim::Card::CrobatV};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(!sim::EngineTestAccess::play_field_blower(engine), "real Item lock must block Field Blower");
  pass("Item-lock interaction");
}

void arven_supporter_contention() {
  const sim::Scenario scenario{"garbodor-shake-ability-lock/go-second",
                               sim::DciProfile::StrictJit, sim::LockMode::None, false, 5};

  std::mt19937_64 crobat_rng{280805};
  sim::Engine crobat_engine = make_engine(scenario, crobat_rng);
  auto crobat_state = minimal_state(1);
  crobat_state.hand = {sim::Card::Arven, sim::Card::CrobatV};
  crobat_state.deck = {sim::Card::FieldBlower, sim::Card::ForestSealStone,
                       sim::Card::RegidragoVstar, sim::Card::Grass,
                       sim::Card::Fire, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(crobat_engine, std::move(crobat_state));
  expect(sim::EngineTestAccess::arven_blower_route_live(crobat_engine),
         "Arven should fetch Blower for a live non-Supporter Ability route");
  expect(sim::EngineTestAccess::play_arven(crobat_engine), "Arven live route should play");
  expect(std::find(sim::EngineTestAccess::state(crobat_engine).hand.begin(),
                   sim::EngineTestAccess::state(crobat_engine).hand.end(), sim::Card::FieldBlower) !=
             sim::EngineTestAccess::state(crobat_engine).hand.end(),
         "Arven must search Field Blower for live Dark Asset");

  std::mt19937_64 tapu_rng{280806};
  sim::Engine tapu_engine = make_engine(scenario, tapu_rng);
  auto tapu_state = minimal_state(1);
  tapu_state.hand = {sim::Card::Arven, sim::Card::TapuLeleGX};
  tapu_state.deck = {sim::Card::FieldBlower, sim::Card::ForestSealStone,
                     sim::Card::RegidragoVstar, sim::Card::Grass,
                     sim::Card::Fire, sim::Card::Crispin};
  sim::EngineTestAccess::set_state(tapu_engine, std::move(tapu_state));
  // Arven consumes the Supporter action, so Wonder Tag cannot be the sole payoff:
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  expect(!sim::EngineTestAccess::arven_blower_route_live(tapu_engine),
         "Wonder Tag alone must not justify Arven fetching Field Blower");
  pass("Arven Supporter contention");
}

void forest_seal_stone_ruling() {
  std::mt19937_64 rng{280807};
  const sim::Scenario scenario{"garbodor-shake-ability-lock/go-second",
                               sim::DciProfile::StrictJit, sim::LockMode::None, false, 5};
  sim::Engine engine = make_engine(scenario, rng);
  auto state = minimal_state(1);
  state.active->tool = sim::Tool::ForestSealStone;
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite, sim::Card::MysteriousTreasure};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  // Star Alchemy is the Tool's granted Ability under the repository's existing ruling:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
  expect(sim::EngineTestAccess::garbodor_locked(engine), "fixture must be Garbotoxin-locked");
  expect(sim::EngineTestAccess::use_fss(engine), "Forest Seal Stone must remain usable");
  pass("Forest Seal Stone Tool-Ability ruling");
}
}  // namespace

int main() {
  registry_and_timing();
  field_blower_relief_and_relock();
  item_lock_still_blocks_blower();
  arven_supporter_contention();
  forest_seal_stone_ruling();
  std::cout << "PASS issue-2808: all focused Garbodor lock tests\n";
  return 0;
}
