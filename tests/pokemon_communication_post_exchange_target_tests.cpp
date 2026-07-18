#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }

  static void set_deck_seen(Engine& engine) {
    engine.deck_seen_ = true;
  }

  static std::optional<Engine::PokemonCommunicationPlan> communication_plan(
      Engine& engine, const bool permit_payload) {
    return engine.pokemon_communication_plan(permit_payload);
  }

  static bool play_communication(Engine& engine, const bool permit_payload) {
    return engine.play_pokemon_communication(permit_payload);
  }

  static bool play_treasure(Engine& engine, const bool permit_payload) {
    return engine.play_mysterious_treasure(permit_payload);
  }

  static bool play_quick_ball(Engine& engine, const bool permit_payload) {
    return engine.play_quick_ball(permit_payload);
  }

  static const State& state(const Engine& engine) {
    return engine.state_;
  }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(std::string label, const std::uint64_t seed)
      : scenario{std::move(label), sim::DciProfile::StrictJit,
                 sim::LockMode::None, false, 4},
        rng(seed),
        engine(scenario, recipe, rng) {}
};

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State ready_attacker_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  return state;
}

void rejects_fetched_only_mysterious_treasure_target() {
  Fixture fixture("issue-808-treasure-negative", 80801);
  sim::State state = ready_attacker_state();
  state.hand = {sim::Card::PokemonCommunication, sim::Card::MawileGX,
                sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::MegaDragonite};
  const sim::State original = state;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // After Communication fetches Mega Dragonite ex, the exact deck contains only
  // Mawile-GX. Discarding the fetched payload leaves Mysterious Treasure with no
  // Psychic or Dragon target, so the one-shot exchange must be rejected up front:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/sm11-141
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  // https://github.com/FlareZ123/pokemon-sims/issues/808
  if (sim::EngineTestAccess::communication_plan(fixture.engine, true) ||
      sim::EngineTestAccess::play_communication(fixture.engine, true) ||
      sim::EngineTestAccess::state(fixture.engine).hand != original.hand ||
      sim::EngineTestAccess::state(fixture.engine).deck != original.deck) {
    throw std::runtime_error(
        "Communication accepted a fetched-only Mysterious Treasure target.");
  }
}

void preserves_returned_mysterious_treasure_target() {
  Fixture fixture("issue-808-treasure-returned-positive", 80802);
  sim::State state = ready_attacker_state();
  state.hand = {sim::Card::PokemonCommunication, sim::Card::Dipplin,
                sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Dipplin is returned to the deck before the search and is itself a Dragon
  // target for the later Mysterious Treasure. The valid continuation must remain:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv6-127
  // https://github.com/FlareZ123/pokemon-sims/issues/808
  const auto plan =
      sim::EngineTestAccess::communication_plan(fixture.engine, true);
  if (!plan || plan->returned != sim::Card::Dipplin ||
      plan->target != sim::Card::MegaDragonite ||
      !sim::EngineTestAccess::play_communication(fixture.engine, true) ||
      !sim::EngineTestAccess::play_treasure(fixture.engine, true) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::Dipplin)) {
    throw std::runtime_error(
        "Communication lost the returned-Dragon Treasure continuation.");
  }
}

void rejects_fetched_only_quick_ball_target() {
  Fixture fixture("issue-808-quick-ball-negative", 80803);
  sim::State state = ready_attacker_state();
  state.hand = {sim::Card::PokemonCommunication, sim::Card::Dipplin,
                sim::Card::QuickBall};
  state.deck = {sim::Card::DialgaGX};
  const sim::State original = state;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // After Communication fetches Dialga-GX, only Stage 1 Dipplin remains in the
  // exact deck. Quick Ball cannot discard Dialga-GX and then search a Basic:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/sv6-127
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  // https://github.com/FlareZ123/pokemon-sims/issues/808
  if (sim::EngineTestAccess::communication_plan(fixture.engine, true) ||
      sim::EngineTestAccess::play_communication(fixture.engine, true) ||
      sim::EngineTestAccess::state(fixture.engine).hand != original.hand ||
      sim::EngineTestAccess::state(fixture.engine).deck != original.deck) {
    throw std::runtime_error(
        "Communication accepted a fetched-only Quick Ball target.");
  }
}

void preserves_returned_quick_ball_target() {
  Fixture fixture("issue-808-quick-ball-returned-positive", 80804);
  sim::State state = ready_attacker_state();
  state.hand = {sim::Card::PokemonCommunication, sim::Card::MawileGX,
                sim::Card::QuickBall};
  state.deck = {sim::Card::DialgaGX};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Mawile-GX is returned to the deck before the search and is a Basic target for
  // the later Quick Ball. The valid continuation must remain available:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm11-141
  // https://github.com/FlareZ123/pokemon-sims/issues/808
  const auto plan =
      sim::EngineTestAccess::communication_plan(fixture.engine, true);
  if (!plan || plan->returned != sim::Card::MawileGX ||
      plan->target != sim::Card::DialgaGX ||
      !sim::EngineTestAccess::play_communication(fixture.engine, true) ||
      !sim::EngineTestAccess::play_quick_ball(fixture.engine, true) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::MawileGX)) {
    throw std::runtime_error(
        "Communication lost the returned-Basic Quick Ball continuation.");
  }
}

}  // namespace

int main() {
  rejects_fetched_only_mysterious_treasure_target();
  preserves_returned_mysterious_treasure_target();
  rejects_fetched_only_quick_ball_target();
  preserves_returned_quick_ball_target();
  return 0;
}
