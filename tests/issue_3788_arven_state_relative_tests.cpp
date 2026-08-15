#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>

namespace sim {

struct EngineTestAccess {};

}  // namespace sim

namespace {

using sim::Card;
using sim::DciProfile;
using sim::DeckRecipe;
using sim::Engine;
using sim::LockMode;
using sim::Pokemon;
using sim::Scenario;

void require(const bool condition, const std::string_view message) {
  if (!condition) throw std::runtime_error(std::string(message));
}

DeckRecipe route_recipe() {
  return {
      {Card::RegidragoV, 4},
      {Card::QuickBall, 1},
      {Card::MysteriousTreasure, 1},
      {Card::Arven, 1},
      {Card::MegaDragonite, 2},
      {Card::GoodraVstar, 1},
  };
}

Engine make_engine(const int turn, const bool going_first,
                   const LockMode locks = LockMode::None) {
  static std::mt19937_64 rng{3788};
  Scenario scenario{
      .label = "issue-3788",
      .dci = DciProfile::StrictJit,
      .locks = locks,
      .going_first = going_first,
      .max_turn = 4,
  };
  Engine engine{scenario, route_recipe(), rng};
  engine.state_.turn = turn;
  engine.state_.hand = {
      Card::Arven,
      Card::MegaDragonite,
      Card::MegaDragonite,
      Card::GoodraVstar,
  };
  engine.state_.deck = {Card::RegidragoV, Card::QuickBall};
  return engine;
}

void test_original_t1_going_second_witness_remains_live() {
  auto engine = make_engine(1, false);
  require(engine.issue_1605_arven_crobat_route_available(),
          "The original T1 going-second Arven witness must remain live.");
}

void test_equivalent_later_turns_are_state_relative() {
  auto second_player_t2 = make_engine(2, false);
  require(second_player_t2.issue_1605_arven_crobat_route_available(),
          "Equivalent T2 going-second state must admit the Arven connector.");

  auto first_player_t2 = make_engine(2, true);
  require(first_player_t2.issue_1605_arven_crobat_route_available(),
          "Equivalent T2 going-first state must admit the Arven connector once Supporters are legal.");
}

void test_first_player_first_turn_supporter_rule_still_blocks_route() {
  auto engine = make_engine(1, true);
  // The first player cannot play a Supporter on that player's first turn:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  require(!engine.issue_1605_arven_crobat_route_available(),
          "T1 going-first must remain blocked by Supporter legality.");
}

void test_semantic_negative_controls() {
  auto supporter_used = make_engine(2, true);
  supporter_used.state_.supporter_used = true;
  require(!supporter_used.issue_1605_arven_crobat_route_available(),
          "A used Supporter slot must block Arven admission.");

  auto item_locked = make_engine(2, true, LockMode::FullItem);
  // Quick Ball and Mysterious Treasure are Items whose search routes remain unavailable through Item lock:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm6-113
  require(!item_locked.issue_1605_arven_crobat_route_available(),
          "Item lock must block the one-discard search route.");

  auto full_bench = make_engine(2, true);
  full_bench.state_.bench.assign(5, Pokemon{Card::TapuLeleGX, 1});
  require(!full_bench.issue_1605_arven_crobat_route_available(),
          "A full Bench must block the missing-Regidrago setup axis.");

  auto missing_target = make_engine(2, true);
  missing_target.deck_seen_ = true;
  missing_target.state_.deck = {Card::QuickBall};
  // Regidrago V is the Basic searched by the live setup route:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  require(!missing_target.issue_1605_arven_crobat_route_available(),
          "A known deck with no Regidrago V target must block the route.");

  auto insufficient_payload = make_engine(2, true);
  insufficient_payload.state_.hand = {
      Card::Arven,
      Card::MegaDragonite,
      Card::MegaDragonite,
  };
  // Duplicate-payload DCI must preserve another distinct modeled Dragon identity:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed generic predecessor: https://github.com/FlareZ123/pokemon-sims/issues/3467
  // Confirmed residual bug: https://github.com/FlareZ123/pokemon-sims/issues/3788
  require(!insufficient_payload.issue_1605_arven_crobat_route_available(),
          "Duplicate payload fuel without a surviving distinct identity must remain protected.");
}

}  // namespace

int main() {
  try {
    test_original_t1_going_second_witness_remains_live();
    test_equivalent_later_turns_are_state_relative();
    test_first_player_first_turn_supporter_rule_still_blocks_route();
    test_semantic_negative_controls();
    std::cout << "Issue #3788 Arven state-relative tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
