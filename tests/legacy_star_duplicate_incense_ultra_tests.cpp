#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static bool use_legacy_star(Engine& engine) { return engine.use_legacy_star(); }
  static bool play_evolution_incense(Engine& engine) {
    return engine.play_evolution_incense(true);
  }
  static bool play_ultra_ball(Engine& engine) {
    return engine.play_ultra_ball(true);
  }
};

}  // namespace sim

namespace {

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

sim::Engine make_engine(const std::uint64_t seed, std::mt19937_64& rng,
                        const sim::DeckRecipe& recipe) {
  using namespace sim;
  static const Scenario scenario{"legacy-star-duplicate-incense-ultra",
                                 DciProfile::StrictJit, LockMode::None, false, 4};
  rng.seed(seed);
  return Engine(scenario, recipe, rng);
}

std::vector<sim::Card> discard_seven_with_incense_and_ultra() {
  using namespace sim;
  return {Card::EvolutionIncense, Card::UltraBall, Card::MawileGX,
          Card::Guzma, Card::Channeler, Card::FieldBlower,
          Card::ChaoticSwell};
}

void append_top_seven(std::vector<sim::Card>& deck) {
  const std::vector<sim::Card> seven = discard_seven_with_incense_and_ultra();
  deck.insert(deck.end(), seven.begin(), seven.end());
}

void test_legacy_star_recovers_duplicate_incense_ultra_bridge() {
  using namespace sim;
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng;
  Engine engine = make_engine(539, rng, recipe);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::EvolutionIncense};
  state.deck = {Card::MegaDragonite, Card::RegidragoV};
  append_top_seven(state.deck);

  // Legacy Star may recover Evolution Incense plus Ultra Ball. After one Incense
  // fetches Mega Dragonite ex, the distinct held Incense may join that payload as
  // Ultra Ball's two-card cost, leaving the Dragon in discard while the remaining
  // Regidrago V supplies Ultra Ball's legal Pokémon search target:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  assert(EngineTestAccess::use_legacy_star(engine));
  assert(count(state.hand, Card::EvolutionIncense) == 2);
  assert(count(state.hand, Card::UltraBall) == 1);
  assert(EngineTestAccess::play_evolution_incense(engine));
  assert(count(state.hand, Card::MegaDragonite) == 1);
  assert(EngineTestAccess::play_ultra_ball(engine));
  assert(count(state.discarded_this_turn, Card::MegaDragonite) == 1);
}

void test_legacy_star_rejects_bridge_without_held_duplicate() {
  using namespace sim;
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng;
  Engine engine = make_engine(540, rng, recipe);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.deck = {Card::MegaDragonite, Card::RegidragoV};
  append_top_seven(state.deck);

  // Ultra Ball has a legal Regidrago V target, yet still needs two other cards.
  // With only the played Incense and fetched payload, no distinct second cost remains:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  assert(EngineTestAccess::use_legacy_star(engine));
  assert(count(state.hand, Card::EvolutionIncense) == 0);
  assert(count(state.hand, Card::UltraBall) == 0);
}

void test_legacy_star_rejects_bridge_when_incense_empties_deck() {
  using namespace sim;
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng;
  Engine engine = make_engine(541, rng, recipe);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::EvolutionIncense};
  state.deck = {Card::MegaDragonite};
  append_top_seven(state.deck);

  // If Incense removes the final deck card, Ultra Ball cannot begin its later deck
  // search and the recovery pair must remain in discard:
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  assert(EngineTestAccess::use_legacy_star(engine));
  assert(count(state.hand, Card::EvolutionIncense) == 1);
  assert(count(state.hand, Card::UltraBall) == 0);
}

void test_legacy_star_rejects_bridge_when_energy_is_also_missing() {
  using namespace sim;
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng;
  Engine engine = make_engine(542, rng, recipe);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 1, 1};
  state.hand = {Card::EvolutionIncense};
  state.deck = {Card::MegaDragonite, Card::RegidragoV};
  append_top_seven(state.deck);

  // Regidrago V keeps Ultra Ball's search legal. Preserve the pair while the
  // selected attacker still lacks Apex Dragon's GGF cost; the duplicate-Incense
  // exception is reserved for a complete final axis:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  assert(EngineTestAccess::use_legacy_star(engine));
  assert(count(state.hand, Card::EvolutionIncense) == 1);
  assert(count(state.hand, Card::UltraBall) == 0);
}

}  // namespace

int main() {
  test_legacy_star_recovers_duplicate_incense_ultra_bridge();
  test_legacy_star_rejects_bridge_without_held_duplicate();
  test_legacy_star_rejects_bridge_when_incense_empties_deck();
  test_legacy_star_rejects_bridge_when_energy_is_also_missing();
  return 0;
}
