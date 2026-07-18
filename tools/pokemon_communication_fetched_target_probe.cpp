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

sim::Engine make_engine(const std::uint64_t seed, std::mt19937_64& rng) {
  const sim::Scenario scenario{"communication-fetched-target-probe",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  rng.seed(seed);
  return sim::Engine(scenario, sim::baseline_recipe(), rng);
}

bool treasure_reproduction() {
  std::mt19937_64 rng;
  sim::Engine engine = make_engine(2026071801ULL, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::PokemonCommunication, sim::Card::MawileGX,
                sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Communication fetches the deck's only Dragon. The later Mysterious Treasure
  // would discard that fetched Dragon, leaving a known deck with no Psychic or
  // Dragon target and therefore cannot legally be played:
  // Pokémon Communication: https://api.pokemontcg.io/v2/cards/sm9-152
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Trainer no-effect ruling: https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  const auto plan = sim::EngineTestAccess::plan(engine, true);
  const bool planned = plan && plan->returned == sim::Card::MawileGX &&
      plan->target == sim::Card::MegaDragonite;
  const bool communication_played =
      sim::EngineTestAccess::play_communication(engine, true);
  const bool treasure_played =
      sim::EngineTestAccess::play_treasure(engine, true);
  std::cout << "treasure_plan=" << planned << '\n'
            << "treasure_communication_played=" << communication_played << '\n'
            << "treasure_followup_played=" << treasure_played << '\n';
  return planned && communication_played && !treasure_played;
}

bool quick_ball_reproduction() {
  std::mt19937_64 rng;
  sim::Engine engine = make_engine(2026071802ULL, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::PokemonCommunication, sim::Card::Dipplin,
                sim::Card::QuickBall};
  state.deck = {sim::Card::DialgaGX};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Communication fetches the deck's only Basic Pokémon. The later Quick Ball
  // would discard that fetched Basic, leaving only the returned Stage 1 Dipplin in
  // the known deck and therefore has no legal Basic target:
  // Pokémon Communication: https://api.pokemontcg.io/v2/cards/sm9-152
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Dipplin: https://api.pokemontcg.io/v2/cards/sv6-127
  const auto plan = sim::EngineTestAccess::plan(engine, true);
  const bool planned = plan && plan->returned == sim::Card::Dipplin &&
      plan->target == sim::Card::DialgaGX;
  const bool communication_played =
      sim::EngineTestAccess::play_communication(engine, true);
  const bool quick_ball_played =
      sim::EngineTestAccess::play_quick_ball(engine, true);
  std::cout << "quick_ball_plan=" << planned << '\n'
            << "quick_ball_communication_played=" << communication_played << '\n'
            << "quick_ball_followup_played=" << quick_ball_played << '\n';
  return planned && communication_played && !quick_ball_played;
}

}  // namespace

int main() {
  return treasure_reproduction() && quick_ball_reproduction() ? 0 : 1;
}
