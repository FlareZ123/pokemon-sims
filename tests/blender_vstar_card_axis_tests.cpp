#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen,
                        const bool prizes_revealed = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_brilliant_blender(Engine& engine) {
    return engine.play_brilliant_blender();
  }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{718};
  sim::Engine engine;

  explicit Fixture(
      const sim::DciProfile dci = sim::DciProfile::StrictJit,
      const sim::LockMode locks = sim::LockMode::None)
      : scenario{"blender-vstar-card-axis", dci, locks, true, 5},
        engine{scenario, recipe, rng} {}
};

sim::State ready_regi_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::BrilliantBlender};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dipplin};
  return state;
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect_rejected(sim::State state, const bool deck_seen,
                     const std::string_view label) {
  Fixture fixture;
  const auto hand_before = state.hand;
  const auto deck_before = state.deck;
  const auto discard_before = state.discard;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state), deck_seen);

  if (sim::EngineTestAccess::play_brilliant_blender(fixture.engine)) {
    throw std::runtime_error(std::string(label) +
                             ": Blender must be held.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (after.hand != hand_before || after.deck != deck_before ||
      after.discard != discard_before) {
    throw std::runtime_error(std::string(label) +
                             ": rejection must preserve every zone.");
  }
}

void expect_played(Fixture& fixture, sim::State state, const bool deck_seen,
                   const bool prizes_revealed, const std::string_view label) {
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state), deck_seen,
                                   prizes_revealed);
  if (!sim::EngineTestAccess::play_brilliant_blender(fixture.engine)) {
    throw std::runtime_error(std::string(label) +
                             ": live route must preserve the Blender play.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (contains(after.hand, sim::Card::BrilliantBlender) ||
      !contains(after.discard, sim::Card::BrilliantBlender) ||
      !contains(after.discard, sim::Card::MegaDragonite)) {
    throw std::runtime_error(std::string(label) +
                             ": Blender must resolve its printed discard route.");
  }
}

void test_strict_jit_holds_blender_without_vstar_card_route() {
  sim::State state = ready_regi_state();
  state.deck.push_back(sim::Card::RegidragoVstar);

  // A prior-turn GGF Regidrago V satisfies evolution timing, but no held VSTAR,
  // payable search connector, Tool route, Prize bridge, or recovery line can
  // obtain the evolution card before turn end. The one-use ACE SPEC cannot
  // contribute to a later strict-JIT turn and must remain in hand:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  // https://github.com/FlareZ123/pokemon-sims/issues/718
  expect_rejected(std::move(state), true, "no VSTAR card route");
}

void test_held_vstar_preserves_blender() {
  Fixture fixture;
  sim::State state = ready_regi_state();
  state.hand.push_back(sim::Card::RegidragoVstar);

  // The held evolution card can evolve the prior-turn Active Regidrago V after
  // Blender, so the Dragon entering discard belongs to this turn's legal Apex
  // Dragon route:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/718
  expect_played(fixture, std::move(state), true, false, "held VSTAR");
}

void test_payable_search_item_preserves_blender() {
  Fixture fixture;
  sim::State state = ready_regi_state();
  state.hand.push_back(sim::Card::MysteriousTreasure);
  state.hand.push_back(sim::Card::Dipplin);
  state.deck.push_back(sim::Card::RegidragoVstar);

  // Mysterious Treasure retains a distinct one-card cost after Blender leaves
  // hand and can search the Dragon-type Regidrago VSTAR:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/718
  expect_played(fixture, std::move(state), true, false,
                "payable Mysterious Treasure");
}

void test_forest_seal_stone_preserves_blender() {
  Fixture fixture;
  sim::State state = ready_regi_state();
  state.hand.push_back(sim::Card::ForestSealStone);
  state.deck.push_back(sim::Card::RegidragoVstar);

  // Forest Seal Stone can attach to the Active Pokémon V, and Star Alchemy can
  // search the VSTAR card without consuming the Supporter slot:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/718
  expect_played(fixture, std::move(state), true, false,
                "Forest Seal Stone");
}

void test_arven_follow_up_preserves_blender() {
  Fixture fixture;
  sim::State state = ready_regi_state();
  state.hand.push_back(sim::Card::Arven);
  state.deck.push_back(sim::Card::EvolutionIncense);
  state.deck.push_back(sim::Card::RegidragoVstar);

  // Arven can search Evolution Incense as its Item, and Evolution Incense can
  // obtain Regidrago VSTAR during the same turn after Blender resolves:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/718
  expect_played(fixture, std::move(state), true, false, "Arven follow-up");
}

void test_gladion_follow_up_preserves_blender() {
  Fixture fixture;
  sim::State state = ready_regi_state();
  state.hand.push_back(sim::Card::Gladion);
  state.prizes = {sim::Card::RegidragoVstar, sim::Card::Grass,
                  sim::Card::Fire};

  // With Prize information public, Gladion can exchange itself for the known
  // prized VSTAR after Blender, preserving the same-turn evolution route:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/718
  expect_played(fixture, std::move(state), true, true,
                "known-prize Gladion follow-up");
}

void test_k0_preserves_plausible_search_route_without_hidden_oracle() {
  Fixture fixture;
  sim::State state = ready_regi_state();
  state.hand.push_back(sim::Card::EvolutionIncense);
  state.prizes = {sim::Card::RegidragoVstar};

  // Before a legal deck inspection, fixed-list VSTAR copies remain publicly
  // plausible search targets. The preflight must not read the physical Prize or
  // deck identity to suppress Evolution Incense's K0 route:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  // https://github.com/FlareZ123/pokemon-sims/issues/718
  expect_played(fixture, std::move(state), false, false,
                "K0 plausible Evolution Incense");
}

void test_rule_box_lock_keeps_forest_seal_stone_route() {
  Fixture fixture{sim::DciProfile::StrictJit,
                  sim::LockMode::FullRuleBoxAbility};
  sim::State state = ready_regi_state();
  state.hand.push_back(sim::Card::ForestSealStone);
  state.deck.push_back(sim::Card::RegidragoVstar);

  // Rule Box Ability lock suppresses the Pokémon's own Ability. Star Alchemy is
  // Forest Seal Stone's Ability, so the attached Tool still searches the VSTAR:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
  // https://github.com/FlareZ123/pokemon-sims/issues/718
  expect_played(fixture, std::move(state), true, false,
                "Rule Box lock Forest Seal Stone");
}

void test_team_yell_recovery_preserves_blender() {
  Fixture fixture;
  sim::State state = ready_regi_state();
  state.turn = 3;
  state.hand.push_back(sim::Card::TeamYellsCheer);
  state.hand.push_back(sim::Card::EvolutionIncense);
  state.discard.push_back(sim::Card::RegidragoVstar);

  // Team Yell's Cheer can restore a discarded VSTAR to the deck, and the held
  // Evolution Incense can search that restored Evolution Pokémon afterward:
  // https://api.pokemontcg.io/v2/cards/swsh9-149
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/718
  expect_played(fixture, std::move(state), true, false,
                "Team Yell recovery");
}

void test_no_discard_control_keeps_payload_banking() {
  Fixture fixture{sim::DciProfile::NoDiscardControl};
  sim::State state = ready_regi_state();

  // No-discard-control intentionally permits earlier payload banking; the new
  // route gate is limited to the strict-JIT timing contract:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/718
  expect_played(fixture, std::move(state), true, false,
                "no-discard-control banking");
}

}  // namespace

int main() {
  try {
    test_strict_jit_holds_blender_without_vstar_card_route();
    test_held_vstar_preserves_blender();
    test_payable_search_item_preserves_blender();
    test_forest_seal_stone_preserves_blender();
    test_arven_follow_up_preserves_blender();
    test_gladion_follow_up_preserves_blender();
    test_k0_preserves_plausible_search_route_without_hidden_oracle();
    test_rule_box_lock_keeps_forest_seal_stone_route();
    test_team_yell_recovery_preserves_blender();
    test_no_discard_control_keeps_payload_banking();
    std::cout << "Blender VSTAR card-axis tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
