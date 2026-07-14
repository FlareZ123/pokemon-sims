#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool play_ultra_ball(Engine& engine) { return engine.play_ultra_ball(false); }
  static bool play_mysterious_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(true);
  }
  static bool play_earthen_vessel(Engine& engine) {
    return engine.play_earthen_vessel(true);
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::Engine make_engine(const std::uint64_t seed, std::mt19937_64& rng) {
  using namespace sim;
  static const Scenario scenario{"ultra-ball-cross-singleton-cost",
                                 DciProfile::StrictJit, LockMode::None, false, 4};
  static const DeckRecipe recipe = baseline_recipe();
  rng.seed(seed);
  return Engine(scenario, recipe, rng);
}

void test_ultra_ball_preserves_vessel_when_energy_remains() {
  using namespace sim;
  std::mt19937_64 rng;
  Engine engine = make_engine(534, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::UltraBall, Card::MysteriousTreasure,
                Card::EarthenVessel, Card::Dipplin};
  state.deck = {Card::MegaDragonite, Card::Grass};
  EngineTestAccess::set_deck_seen(engine);

  // Ultra Ball may discard Dipplin plus the singleton Treasure while preserving
  // Earthen Vessel to discard the fetched Dragon and search the remaining Energy:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  assert(EngineTestAccess::play_ultra_ball(engine));
  assert(contains(state.hand, Card::MegaDragonite));
  assert(contains(state.hand, Card::EarthenVessel));
  assert(!contains(state.hand, Card::MysteriousTreasure));
  assert(EngineTestAccess::play_earthen_vessel(engine));
  assert(contains(state.discarded_this_turn, Card::MegaDragonite));
  assert(contains(state.hand, Card::Grass));
}

void test_ultra_ball_preserves_treasure_when_psychic_target_remains() {
  using namespace sim;
  std::mt19937_64 rng;
  Engine engine = make_engine(535, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::UltraBall, Card::MysteriousTreasure,
                Card::EarthenVessel, Card::Dipplin};
  state.deck = {Card::MegaDragonite, Card::TapuLeleGX};
  EngineTestAccess::set_deck_seen(engine);

  // When the remaining deck has a Psychic target and no Basic Energy, preserve
  // Mysterious Treasure and spend Earthen Vessel as Ultra Ball's second cost:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  assert(EngineTestAccess::play_ultra_ball(engine));
  assert(contains(state.hand, Card::MegaDragonite));
  assert(contains(state.hand, Card::MysteriousTreasure));
  assert(!contains(state.hand, Card::EarthenVessel));
  assert(EngineTestAccess::play_mysterious_treasure(engine));
  assert(contains(state.discarded_this_turn, Card::MegaDragonite));
  assert(contains(state.hand, Card::TapuLeleGX));
}

void test_cross_singleton_route_requires_a_surviving_outlet() {
  using namespace sim;
  std::mt19937_64 rng;
  Engine engine = make_engine(536, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::UltraBall, Card::MysteriousTreasure, Card::Dipplin};
  state.deck = {Card::MegaDragonite, Card::Grass};
  EngineTestAccess::set_deck_seen(engine);

  // A lone search Item must remain protected because spending it leaves no separate
  // effect to discard the fetched Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  assert(!EngineTestAccess::play_ultra_ball(engine));
  assert(state.hand.size() == 3U);
  assert(state.deck.size() == 2U);
  assert(state.discard.empty());
}

void test_cross_singleton_route_rejects_a_final_card_payload() {
  using namespace sim;
  std::mt19937_64 rng;
  Engine engine = make_engine(537, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::UltraBall, Card::MysteriousTreasure,
                Card::EarthenVessel, Card::Dipplin};
  state.deck = {Card::MegaDragonite};
  EngineTestAccess::set_deck_seen(engine);

  // Removing the final deck card would leave every surviving search Item unable to
  // begin its search, so the cross-singleton exception must stay disabled:
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  assert(!EngineTestAccess::play_ultra_ball(engine));
  assert(state.hand.size() == 4U);
  assert(state.deck.size() == 1U);
  assert(state.discard.empty());
}

void test_cross_singleton_route_requires_payload_as_the_only_missing_axis() {
  using namespace sim;
  std::mt19937_64 rng;
  Engine engine = make_engine(538, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1};
  state.hand = {Card::UltraBall, Card::MysteriousTreasure,
                Card::EarthenVessel, Card::Dipplin};
  state.deck = {Card::MegaDragonite, Card::Grass};
  EngineTestAccess::set_deck_seen(engine);

  // Preserve both singleton connectors while the VSTAR and Energy axes are still
  // missing; this exception exists only for a proved final payload route:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  assert(!EngineTestAccess::play_ultra_ball(engine));
  assert(state.hand.size() == 4U);
  assert(state.discard.empty());
}

}  // namespace

int main() {
  test_ultra_ball_preserves_vessel_when_energy_remains();
  test_ultra_ball_preserves_treasure_when_psychic_target_remains();
  test_cross_singleton_route_requires_a_surviving_outlet();
  test_cross_singleton_route_rejects_a_final_card_payload();
  test_cross_singleton_route_requires_payload_as_the_only_missing_axis();
  return 0;
}
