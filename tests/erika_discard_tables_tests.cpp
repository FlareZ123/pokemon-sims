#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <optional>
#include <random>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static std::optional<Card> choose_discard(
      Engine& engine, const bool permit_payload, const bool flex_fodder,
      const bool allow_heavy_ball = true,
      const std::optional<Card> excluded_from_cost = std::nullopt) {
    return engine.choose_discard(permit_payload, flex_fodder, allow_heavy_ball,
                                 excluded_from_cost);
  }
  static bool play_serena(Engine& engine) { return engine.play_serena(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_matchup_flex_erika_pays_one_discard_item() {
  using namespace sim;
  const Scenario scenario{"issue-852-erika-item-cost", DciProfile::MatchupFlexJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(85201);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.hand = {Card::QuickBall, Card::ErikasInvitation, Card::RegidragoVstar};

  // Quick Ball may discard Erika's Invitation as the required other hand card.
  // Erika's opponent-dependent effect is deliberately inert in this goldfish model,
  // so the matchup-flex DCI table treats it as the low-impact Mawile-GX replacement:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv3pt5-160
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CARD_AUDIT.md#erikas-invitation
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/852
  const std::optional<Card> cost = EngineTestAccess::choose_discard(
      engine, false, true, true, Card::QuickBall);
  assert(cost.has_value());
  assert(*cost == Card::ErikasInvitation);
}

void test_strict_jit_keeps_erika_protected() {
  using namespace sim;
  const Scenario scenario{"issue-852-strict-control", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(85202);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.hand = {Card::QuickBall, Card::ErikasInvitation, Card::RegidragoVstar};

  // The issue is confined to matchup-flex DCI. Strict JIT does not promote this
  // singleton Supporter into generic discard fodder:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/852
  const std::optional<Card> cost = EngineTestAccess::choose_discard(
      engine, false, true, true, Card::QuickBall);
  assert(!cost.has_value());
}

void test_serena_cycles_erika_after_first_discard() {
  using namespace sim;
  const Scenario scenario{"issue-852-serena-cycle", DciProfile::MatchupFlexJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(85203);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.hand = {Card::Serena, Card::Dipplin, Card::ErikasInvitation};
  state.deck = {Card::Grass, Card::Fire, Card::QuickBall, Card::Arven, Card::Crispin};

  // Serena requires at least one discard and permits up to three. After Dipplin
  // supplies the first admitted discard, Erika is a legal low-impact optional cycle
  // before drawing until five cards remain in hand:
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://api.pokemontcg.io/v2/cards/sv3pt5-160
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CARD_AUDIT.md#erikas-invitation
  // https://github.com/FlareZ123/pokemon-sims/issues/852
  assert(EngineTestAccess::play_serena(engine));
  assert(state.supporter_used);
  assert(contains(state.discard, Card::Serena));
  assert(contains(state.discard, Card::Dipplin));
  assert(contains(state.discard, Card::ErikasInvitation));
}

}  // namespace

int main() {
  test_matchup_flex_erika_pays_one_discard_item();
  test_strict_jit_keeps_erika_protected();
  test_serena_cycles_erika_after_first_discard();
  return 0;
}
