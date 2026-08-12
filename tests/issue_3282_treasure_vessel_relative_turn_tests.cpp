#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool known = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = known;
  }

  static bool available(const Engine& engine) {
    return engine.issue_1872_treasure_vessel_payload_route_available();
  }

  static bool complete(Engine& engine) {
    return engine.complete_issue_1872_treasure_vessel_payload_route();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

void erase_one(std::vector<sim::Card>& cards, const sim::Card card) {
  const auto it = std::find(cards.begin(), cards.end(), card);
  if (it != cards.end()) cards.erase(it);
}

sim::State route_state(const int turn) {
  sim::State state;
  state.turn = turn;
  // Pokemon field order is card, entered_turn, grass, fire, tool, double_dragon:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/game_state_types.inc
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None, 0};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::EarthenVessel,
                sim::Card::ChaoticSwell};
  state.deck = {sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite};
  state.prizes = {sim::Card::QuickBall, sim::Card::Arven,
                  sim::Card::Serena, sim::Card::Crispin,
                  sim::Card::FieldBlower, sim::Card::ForestSealStone};
  return state;
}

bool available(const int turn, const sim::DciProfile dci,
               const sim::LockMode lock = sim::LockMode::None,
               const bool known = true,
               const bool manual_energy_used = false,
               const bool going_first = true) {
  const sim::Scenario scenario{"issue-3282", dci, lock, going_first, 5};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3282000 + static_cast<unsigned>(turn));
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = route_state(turn);
  state.manual_energy_used = manual_energy_used;
  sim::EngineTestAccess::set_state(engine, std::move(state), known);
  return sim::EngineTestAccess::available(engine);
}

void test_equivalent_turn_and_seat_admission() {
  // The route is entirely current-turn. Mysterious Treasure and Earthen Vessel have
  // no absolute-turn or seat restriction, while the repository selects the earliest
  // complete legal connector route from the observable state.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Official Item/search/discard/attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Earliest-route and same-ready-turn JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3282
  expect(available(2, sim::DciProfile::StrictJit),
         "equivalent legal T2 Treasure-Vessel route was hidden");
  expect(available(3, sim::DciProfile::StrictJit),
         "equivalent legal T3 Treasure-Vessel route was hidden");
  expect(available(4, sim::DciProfile::StrictJit),
         "historical T4 Treasure-Vessel witness stopped being legal");
  expect(available(3, sim::DciProfile::StrictJit, sim::LockMode::None,
                   true, false, false),
         "equivalent going-second Treasure-Vessel route was hidden");
  expect(available(3, sim::DciProfile::MatchupFlexJit),
         "MatchupFlex same-ready-turn JIT route was hidden");
}

void test_route_executes_on_equivalent_turn() {
  // Connector domination matters here: Treasure creates the Dragon that Vessel
  // immediately spends as its discard cost, while Vessel supplies the final Grass.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // DCI/connector policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3282
  const sim::Scenario scenario{"issue-3282-complete", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3282099);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, route_state(3));
  expect(sim::EngineTestAccess::complete(engine),
         "equivalent T3 route did not complete GGF plus same-turn payload");
}

void test_semantic_boundaries() {
  // Removing historical turn identity must preserve physical legality. K0 lacks
  // deterministic targets, persistent Item lock blocks both Items, a used manual
  // attachment cannot finish GGF, and NoDiscardControl has different payload timing.
  // Knowledge/JIT/lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3282
  expect(!available(3, sim::DciProfile::NoDiscardControl),
         "NoDiscardControl entered the same-ready-turn JIT route");
  expect(!available(3, sim::DciProfile::StrictJit,
                    sim::LockMode::TurnTwoItem),
         "persistent Item lock admitted the two-Item route");
  expect(!available(3, sim::DciProfile::StrictJit,
                    sim::LockMode::None, false),
         "K0 admitted a deterministic deck-dependent route");
  expect(!available(3, sim::DciProfile::StrictJit,
                    sim::LockMode::None, true, true),
         "used manual attachment admitted the final-Grass route");
}

void test_resource_boundaries() {
  // Current-turn resources remain mandatory after removing the absolute turn gate.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3282
  const sim::Scenario scenario{"issue-3282-resources", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3282100);

  for (const sim::Card missing : {sim::Card::MysteriousTreasure,
                                  sim::Card::EarthenVessel,
                                  sim::Card::ChaoticSwell}) {
    sim::Engine engine(scenario, recipe, rng);
    sim::State state = route_state(3);
    erase_one(state.hand, missing);
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::available(engine),
           "route admitted a missing held route resource");
  }

  {
    sim::Engine engine(scenario, recipe, rng);
    sim::State state = route_state(3);
    erase_one(state.deck, sim::Card::Grass);
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::available(engine),
           "route admitted without searchable Grass");
  }

  {
    sim::Engine engine(scenario, recipe, rng);
    sim::State state = route_state(3);
    erase_one(state.deck, sim::Card::MegaDragonite);
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::available(engine),
           "route admitted without a searchable Dragon payload");
  }
}

}  // namespace

int main() {
  test_equivalent_turn_and_seat_admission();
  test_route_executes_on_equivalent_turn();
  test_semantic_boundaries();
  test_resource_boundaries();
  return 0;
}
