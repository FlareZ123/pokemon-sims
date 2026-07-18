#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <optional>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }

  static void set_deck_seen(Engine& engine) {
    engine.deck_seen_ = true;
  }

  static std::optional<Engine::PokemonCommunicationPlan> plan(
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
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"pokemon-communication-post-exchange-probe",
                         sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(const std::uint64_t seed)
      : rng(seed), engine(scenario, recipe, rng) {}
};

bool treasure_reproduction() {
  Fixture fixture{2026071801ULL};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::PokemonCommunication, sim::Card::MawileGX,
                sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Pokémon Communication removes the fetched Mega Dragonite ex from the deck
  // before Mysterious Treasure can pay its cost and search that deck. A known
  // targetless Trainer cannot be played:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  // https://github.com/FlareZ123/pokemon-sims/issues/808
  const auto plan = sim::EngineTestAccess::plan(fixture.engine, true);
  const bool planned = plan && plan->returned == sim::Card::MawileGX &&
      plan->target == sim::Card::MegaDragonite;
  const bool communication_played =
      sim::EngineTestAccess::play_communication(fixture.engine, true);
  const bool followup_played =
      sim::EngineTestAccess::play_treasure(fixture.engine, true);

  std::cout << "treasure_plan=" << planned << '\n'
            << "treasure_communication_played=" << communication_played << '\n'
            << "treasure_followup_played=" << followup_played << '\n';
  return planned && communication_played && !followup_played;
}

bool quick_ball_reproduction() {
  Fixture fixture{2026071802ULL};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::PokemonCommunication, sim::Card::Dipplin,
                sim::Card::QuickBall};
  state.deck = {sim::Card::DialgaGX};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Dialga-GX leaves the deck during Pokémon Communication. After Quick Ball
  // discards it, the returned Stage 1 Dipplin is not a legal Basic target:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/sv6-127
  // https://github.com/FlareZ123/pokemon-sims/issues/808
  const auto plan = sim::EngineTestAccess::plan(fixture.engine, true);
  const bool planned = plan && plan->returned == sim::Card::Dipplin &&
      plan->target == sim::Card::DialgaGX;
  const bool communication_played =
      sim::EngineTestAccess::play_communication(fixture.engine, true);
  const bool followup_played =
      sim::EngineTestAccess::play_quick_ball(fixture.engine, true);

  std::cout << "quick_ball_plan=" << planned << '\n'
            << "quick_ball_communication_played=" << communication_played << '\n'
            << "quick_ball_followup_played=" << followup_played << '\n';
  return planned && communication_played && !followup_played;
}

}  // namespace

int main() {
  return treasure_reproduction() && quick_ball_reproduction() ? 0 : 1;
}
