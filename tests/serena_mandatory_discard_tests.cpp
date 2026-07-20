#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static bool play_serena(Engine& engine, const bool allow_zero_draw_payload_completion = false) {
    return engine.play_serena(allow_zero_draw_payload_completion);
  }
  static bool play_mysterious_treasure(Engine& engine, const bool permit_payload) {
    return engine.play_mysterious_treasure(permit_payload);
  }
};

}  // namespace sim

namespace {

std::vector<sim::Pokemon> full_bench() {
  return {{sim::Card::MawileGX, 2}, {sim::Card::DialgaGX, 2}, {sim::Card::LatiasEx, 2},
          {sim::Card::Oricorio, 2}, {sim::Card::TapuLeleGX, 2}};
}

void test_serena_requires_a_discard() {
  using namespace sim;
  const Scenario scenario{"serena-mandatory-discard", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(164);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::None};
  state.hand = {Card::Serena};
  state.deck = {Card::RegidragoVstar, Card::Grass, Card::Fire, Card::Dipplin, Card::QuickBall};

  // Exact Serena draw text says "You must discard at least 1 card"; zero is illegal: https://api.pokemontcg.io/v2/cards/swsh12-164
  assert(!EngineTestAccess::play_serena(engine));
  assert(!state.supporter_used);
  assert(std::count(state.hand.begin(), state.hand.end(), Card::Serena) == 1);
  assert(state.discard.empty());
}


void test_strict_serena_preserves_payload_before_energy_axis_can_finish() {
  using namespace sim;
  const Scenario scenario{"serena-complete-axis-jit-gate", DciProfile::StrictJit, LockMode::None, true, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(1099);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 0, 1, Tool::ForestSealStone};
  state.hand = {Card::LatiasEx, Card::MegaDragonite, Card::Powerglass, Card::Serena,
                Card::MysteriousTreasure, Card::RegidragoVstar, Card::StevensResolve};
  state.deck = {Card::Fire, Card::Grass};

  // Serena may discard 1 to 3 cards, but strict JIT cannot count a Dragon before
  // the same turn can finish Apex Dragon's GGF cost. Active Regidrago VSTAR also
  // makes Skyliner irrelevant because Latias ex affects only Basic Pokémon:
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://github.com/FlareZ123/pokemon-sims/issues/1099
  assert(EngineTestAccess::play_serena(engine));
  assert(std::count(state.discard.begin(), state.discard.end(), Card::LatiasEx) == 1);
  assert(std::count(state.hand.begin(), state.hand.end(), Card::MegaDragonite) == 1);
  assert(std::count(state.discard.begin(), state.discard.end(), Card::MegaDragonite) == 0);
  assert(std::count(state.hand.begin(), state.hand.end(), Card::Grass) == 1);
}

void test_serena_still_discards_final_strict_jit_payload() {
  using namespace sim;
  const Scenario scenario{"serena-final-payload-control", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(1100);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.hand = {Card::Serena, Card::MegaDragonite, Card::Powerglass, Card::FieldBlower,
                Card::ChaoticSwell, Card::PathToPeak};
  state.deck.clear();

  // The dedicated final-axis branch must preserve Serena's legal zero-draw payload
  // completion when Active Regidrago VSTAR already has GGF:
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/882
  // https://github.com/FlareZ123/pokemon-sims/issues/1099
  assert(EngineTestAccess::play_serena(engine, true));
  assert(std::count(state.discard.begin(), state.discard.end(), Card::MegaDragonite) == 1);
  assert(std::count(state.discarded_this_turn.begin(), state.discarded_this_turn.end(),
                    Card::MegaDragonite) == 1);
  assert(state.supporter_used);
}

void test_evolution_incense_is_not_a_hand_payload_discard_route() {
  using namespace sim;
  const Scenario scenario{"evolution-incense-not-discard", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(201);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.bench = full_bench();
  state.hand = {Card::MysteriousTreasure, Card::Dipplin, Card::EvolutionIncense};
  state.deck = {Card::MegaDragonite};

  // A Bench has at most five Pokémon, so this state removes unrelated Basic-Pokémon connector routes: https://www.pokemon.com/us/pokemon-tcg/rules
  // Mysterious Treasure can fetch a Dragon after a hand discard, while Evolution Incense has no discard effect: https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/swsh1-163
  assert(!EngineTestAccess::play_mysterious_treasure(engine, true));
  assert(std::count(state.hand.begin(), state.hand.end(), Card::MysteriousTreasure) == 1);
  assert(std::count(state.deck.begin(), state.deck.end(), Card::MegaDragonite) == 1);
}

void test_single_ultra_ball_without_two_follow_up_cards_is_not_a_payload_outlet() {
  using namespace sim;
  const Scenario scenario{"single-ultra-not-payable-after-payload-search", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(203);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.bench = full_bench();
  state.hand = {Card::MysteriousTreasure, Card::Dipplin, Card::UltraBall};
  state.deck = {Card::MegaDragonite};

  // Mysterious Treasure spends itself and one hand card before fetching the payload: https://api.pokemontcg.io/v2/cards/sm6-113
  // Ultra Ball still needs two other cards from hand, so payload plus one Ultra Ball is not a playable continuation: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  assert(!EngineTestAccess::play_mysterious_treasure(engine, true));
  assert(std::count(state.hand.begin(), state.hand.end(), Card::MysteriousTreasure) == 1);
  assert(std::count(state.deck.begin(), state.deck.end(), Card::MegaDragonite) == 1);
}

void test_spent_supporter_slot_disables_serena_payload_continuation() {
  using namespace sim;
  const Scenario scenario{"spent-serena-payload-outlet", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(204);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.supporter_used = true;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.bench = full_bench();
  state.hand = {Card::MysteriousTreasure, Card::Dipplin, Card::Serena};
  state.deck = {Card::MegaDragonite};

  // A full Bench removes unrelated Basic-Pokémon connector routes: https://www.pokemon.com/us/pokemon-tcg/rules
  // Serena cannot be used after the turn's Supporter play: https://api.pokemontcg.io/v2/cards/swsh12-164 https://www.pokemon.com/us/pokemon-tcg/rules
  assert(!EngineTestAccess::play_mysterious_treasure(engine, true));
  assert(std::count(state.hand.begin(), state.hand.end(), Card::MysteriousTreasure) == 1);
  assert(std::count(state.deck.begin(), state.deck.end(), Card::MegaDragonite) == 1);
}

}  // namespace

int main() {
  test_serena_requires_a_discard();
  test_strict_serena_preserves_payload_before_energy_axis_can_finish();
  test_serena_still_discards_final_strict_jit_payload();
  test_evolution_incense_is_not_a_hand_payload_discard_route();
  test_single_ultra_ball_without_two_follow_up_cards_is_not_a_payload_outlet();
  test_spent_supporter_slot_disables_serena_payload_continuation();
  return 0;
}
