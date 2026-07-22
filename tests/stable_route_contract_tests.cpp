#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool future_turn_wonder_tag_route_has_live_target(Engine& engine) {
    return engine.future_turn_wonder_tag_route_has_live_target();
  }
  static bool needs_tapu_connector(Engine& engine) {
    return engine.needs_tapu_connector();
  }
  static bool pay_tapu_retreat_to_ready_benched_vstar(Engine& engine) {
    return engine.pay_tapu_retreat_to_ready_benched_vstar();
  }
  static bool ability_available_for_pokemon(const Engine& engine, const Card card) {
    return engine.ability_available_for_pokemon(card);
  }
};

}  // namespace sim

namespace {

const sim::DeckRecipe& test_recipe() {
  // Engine stores the recipe by reference, so this owner must outlive every test Engine:
  // https://eel.is/c++draft/class.temporary#6.10
  // https://github.com/FlareZ123/pokemon-sims/issues/907
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  return recipe;
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng) {
  return sim::Engine(scenario, test_recipe(), rng);
}

void test_exact_future_wonder_tag_crispin_contract() {
  using namespace sim;
  const Scenario scenario{"issue-875-future-crispin", DciProfile::NoDiscardControl,
                          LockMode::None, true, 4};
  std::mt19937_64 rng(87501);
  Engine engine = make_engine(scenario, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 1;
  state.active = Pokemon{Card::LatiasEx, 0, 0, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoV, 0, 0, 0, Tool::None}};
  state.hand = {Card::RegidragoVstar, Card::Grass, Card::Grass};
  state.deck = {Card::TapuLeleGX, Card::Crispin, Card::Grass, Card::Fire};
  EngineTestAccess::set_deck_seen(engine);

  // Going first prevents a turn-one Supporter play, while Wonder Tag may search
  // Crispin and hold it. The two held Grass Energy plus the exact K1 Grass/Fire
  // deck pair let the current manual attachment, next-turn Crispin attachment, and
  // next-turn manual attachment complete GGF without Vital Dance or Star Alchemy:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/809
  // https://github.com/FlareZ123/pokemon-sims/issues/875
  assert(EngineTestAccess::future_turn_wonder_tag_route_has_live_target(engine));
  assert(EngineTestAccess::needs_tapu_connector(engine));
}

void test_future_crispin_contract_requires_the_exact_energy_plan() {
  using namespace sim;
  const Scenario scenario{"issue-875-future-crispin-control",
                          DciProfile::NoDiscardControl, LockMode::None, true, 4};
  std::mt19937_64 rng(87502);
  Engine engine = make_engine(scenario, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 1;
  state.active = Pokemon{Card::LatiasEx, 0, 0, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoV, 0, 0, 0, Tool::None}};
  state.hand = {Card::RegidragoVstar, Card::Grass, Card::Grass};
  state.deck = {Card::TapuLeleGX, Card::Crispin, Card::Grass};
  EngineTestAccess::set_deck_seen(engine);

  // K1 proves that no Fire Energy remains. Crispin therefore cannot complete the
  // different-type GGF plan, and the future Wonder Tag route must stay unavailable:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  // https://github.com/FlareZ123/pokemon-sims/issues/875
  assert(!EngineTestAccess::future_turn_wonder_tag_route_has_live_target(engine));
}

void test_exact_active_tapu_paid_retreat_contract() {
  using namespace sim;
  const Scenario scenario{"issue-875-paid-retreat", DciProfile::NoDiscardControl,
                          LockMode::None, true, 4};
  std::mt19937_64 rng(87503);
  Engine engine = make_engine(scenario, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 4;
  state.active = Pokemon{Card::TapuLeleGX, 0, 0, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None}};
  state.hand = {Card::Fire};
  state.discard = {Card::MegaDragonite};

  // Tapu Lele-GX has a one-Colorless Retreat Cost. With GGF and a Dragon payload
  // already complete, the unused manual attachment may put Fire on the Active,
  // discard it as the retreat payment, and promote the ready Benched VSTAR:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/802
  // https://github.com/FlareZ123/pokemon-sims/issues/875
  assert(EngineTestAccess::pay_tapu_retreat_to_ready_benched_vstar(engine));
  assert(state.active.has_value());
  assert(state.active->card == Card::RegidragoVstar);
  assert(state.active->grass == 2);
  assert(state.active->fire == 1);
  assert(state.manual_energy_used);
  assert(state.retreat_used);
  assert(contains(state.discard, Card::Fire));
}

void test_paid_retreat_contract_preserves_an_incomplete_payload_axis() {
  using namespace sim;
  const Scenario scenario{"issue-875-paid-retreat-control",
                          DciProfile::NoDiscardControl, LockMode::None, true, 4};
  std::mt19937_64 rng(87504);
  Engine engine = make_engine(scenario, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 4;
  state.active = Pokemon{Card::TapuLeleGX, 0, 0, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None}};
  state.hand = {Card::Fire};

  // The paid retreat helper is reserved for the state where Active position is the
  // only unresolved axis. With no Dragon payload in discard, it must preserve the
  // manual attachment and the Active Pokémon:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/875
  assert(!EngineTestAccess::pay_tapu_retreat_to_ready_benched_vstar(engine));
  assert(state.active.has_value());
  assert(state.active->card == Card::TapuLeleGX);
  assert(!state.manual_energy_used);
  assert(!state.retreat_used);
  assert(contains(state.hand, Card::Fire));
}

void test_rule_box_lock_contract_uses_card_classification() {
  using namespace sim;
  const Scenario scenario{"issue-875-rulebox-lock", DciProfile::StrictJit,
                          LockMode::FullRuleBoxAbility, false, 4};
  std::mt19937_64 rng(87505);
  const Engine engine = make_engine(scenario, rng);

  // The modeled full Rule Box Ability lock suppresses Wonder Tag and Skyliner because
  // Tapu Lele-GX and Latias ex have Rule Boxes. Oricorio GRI 55 has no Rule Box, so
  // Vital Dance remains available. This deterministic classification contract replaces
  // an unsupported shuffled-seed T3 readiness deadline:
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md
  // https://github.com/FlareZ123/pokemon-sims/issues/875
  assert(!EngineTestAccess::ability_available_for_pokemon(engine, Card::TapuLeleGX));
  assert(!EngineTestAccess::ability_available_for_pokemon(engine, Card::LatiasEx));
  assert(EngineTestAccess::ability_available_for_pokemon(engine, Card::Oricorio));
}

}  // namespace

int main() {
  test_exact_future_wonder_tag_crispin_contract();
  test_future_crispin_contract_requires_the_exact_energy_plan();
  test_exact_active_tapu_paid_retreat_contract();
  test_paid_retreat_contract_preserves_an_incomplete_payload_axis();
  test_rule_box_lock_contract_uses_card_classification();
  return 0;
}
