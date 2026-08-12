#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = false;
    engine.prizes_revealed_ = false;
  }
  static bool available(const Engine& engine) {
    return engine.issue_1711_k0_treasure_turo_crispin_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, turn - 1, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::ProfessorTuro,
                sim::Card::RegidragoVstar, sim::Card::EarthenVessel,
                sim::Card::Grass, sim::Card::MegaDragonite,
                sim::Card::MegaDragonite, sim::Card::Appletun};
  state.deck = {sim::Card::RegidragoV, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Fire,
                sim::Card::Dragapult};
  return state;
}

bool available(const sim::DciProfile dci, const sim::LockMode lock,
               const bool going_first, const int turn, const int max_turn,
               sim::State state) {
  const sim::Scenario selected{"issue-3267", dci, lock, going_first, max_turn};
  const sim::DeckRecipe recipe = sim::pineco_recipe();
  std::mt19937_64 rng(3267);
  sim::Engine engine(selected, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return sim::EngineTestAccess::available(engine);
}

void test_semantic_profile_seat_and_turn_admission() {
  // Treasure establishes K1, Turo replays Tapu/Wonder Tag for Crispin, and the newly
  // Benched Regidrago evolves on the following turn. Supporter legality and the
  // one-turn horizon govern seat/turn admission rather than the original witness.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // K0/JIT/priority policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Original route / semantic follow-up: https://github.com/FlareZ123/pokemon-sims/issues/1711 https://github.com/FlareZ123/pokemon-sims/issues/3267
  expect(available(sim::DciProfile::StrictJit, sim::LockMode::None,
                   false, 2, 3, route_state(2)),
         "Original strict/go-second/T2 route regressed");
  expect(available(sim::DciProfile::MatchupFlexJit, sim::LockMode::None,
                   false, 2, 3, route_state(2)),
         "MatchupFlex equivalent route was rejected");
  expect(available(sim::DciProfile::StrictJit, sim::LockMode::None,
                   true, 2, 3, route_state(2)),
         "Going-first T2 route was rejected after Supporters become legal");
  expect(available(sim::DciProfile::StrictJit, sim::LockMode::None,
                   false, 1, 2, route_state(1)),
         "Going-second T1 route was rejected despite a legal next-turn evolution");
}

void test_real_legality_boundaries() {
  expect(!available(sim::DciProfile::NoDiscardControl, sim::LockMode::None,
                    false, 2, 3, route_state(2)),
         "NoDiscardControl entered the same-ready-turn route");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::FullItem,
                    false, 2, 3, route_state(2)),
         "Item lock admitted Treasure/Vessel route");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::FullSupporter,
                    false, 2, 3, route_state(2)),
         "Supporter lock admitted Turo route");
  expect(!available(sim::DciProfile::StrictJit,
                    sim::LockMode::FullRuleBoxAbility,
                    false, 2, 3, route_state(2)),
         "Rule Box Ability lock admitted Wonder Tag route");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::TurnTwoItem,
                    false, 1, 2, route_state(1)),
         "Projected T2 Item lock admitted next-turn Vessel route");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::None,
                    false, 2, 2, route_state(2)),
         "Expired continuation horizon admitted route");

  sim::State no_redundant_payload = route_state(2);
  const auto it = std::find(no_redundant_payload.hand.begin(),
                            no_redundant_payload.hand.end(), sim::Card::Appletun);
  no_redundant_payload.hand.erase(it);
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::None,
                    false, 2, 3, std::move(no_redundant_payload)),
         "Missing dynamically redundant payload cost admitted route");
}
}  // namespace

int main() {
  test_semantic_profile_seat_and_turn_admission();
  test_real_legality_boundaries();
  return 0;
}
