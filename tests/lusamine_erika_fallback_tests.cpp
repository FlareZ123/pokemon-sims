#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static std::vector<Card> lusamine_recovery_targets(const Engine& engine) {
    return engine.lusamine_recovery_targets();
  }
  static bool play_lusamine(Engine& engine) { return engine.play_lusamine(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::Engine make_engine(std::mt19937_64& rng) {
  static const sim::Scenario scenario{"issue-854-lusamine-erika",
                                      sim::DciProfile::StrictJit,
                                      sim::LockMode::None, false, 4};
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  return sim::Engine(scenario, recipe, rng);
}

void configure_payload_state(sim::Engine& engine, const bool include_erika) {
  using namespace sim;
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.hand = {Card::Lusamine};
  state.discard = {Card::ProfessorBurnet};
  if (include_erika) state.discard.push_back(Card::ErikasInvitation);
  state.deck = {Card::MegaDragonite, Card::Grass, Card::Fire};
}

void test_burnet_and_erika_form_legal_two_card_recovery() {
  using namespace sim;
  std::mt19937_64 rng(85401);
  Engine engine = make_engine(rng);
  configure_payload_state(engine, true);
  State& state = EngineTestAccess::state(engine);

  // Lusamine requires exactly two Supporter and/or Stadium cards. Professor Burnet
  // advances the unresolved payload axis, while Erika's Invitation is the legal
  // low-impact mandatory second Supporter recovered from the public discard pile:
  // https://api.pokemontcg.io/v2/cards/sm4-96
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sv3pt5-160
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CARD_AUDIT.md#erikas-invitation
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/854
  const std::vector<Card> targets = EngineTestAccess::lusamine_recovery_targets(engine);
  assert(targets.size() == 2U);
  assert(contains(targets, Card::ProfessorBurnet));
  assert(contains(targets, Card::ErikasInvitation));

  assert(EngineTestAccess::play_lusamine(engine));
  assert(state.supporter_used);
  assert(contains(state.hand, Card::ProfessorBurnet));
  assert(contains(state.hand, Card::ErikasInvitation));
  assert(contains(state.discard, Card::Lusamine));
  assert(!contains(state.discard, Card::ProfessorBurnet));
  assert(!contains(state.discard, Card::ErikasInvitation));
}

void test_one_eligible_target_remains_unplayable() {
  using namespace sim;
  std::mt19937_64 rng(85402);
  Engine engine = make_engine(rng);
  configure_payload_state(engine, false);
  State& state = EngineTestAccess::state(engine);

  // Lusamine does not say "up to 2". Professor Burnet alone cannot satisfy the
  // mandatory two-card recovery requirement:
  // https://api.pokemontcg.io/v2/cards/sm4-96
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // https://github.com/FlareZ123/pokemon-sims/issues/854
  const std::vector<Card> targets = EngineTestAccess::lusamine_recovery_targets(engine);
  assert(targets.size() == 1U);
  assert(targets.front() == Card::ProfessorBurnet);
  assert(!EngineTestAccess::play_lusamine(engine));
  assert(!state.supporter_used);
  assert(contains(state.hand, Card::Lusamine));
  assert(contains(state.discard, Card::ProfessorBurnet));
}

}  // namespace

int main() {
  test_burnet_and_erika_form_legal_two_card_recovery();
  test_one_eligible_target_remains_unplayable();
  return 0;
}
