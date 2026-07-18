#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <initializer_list>
#include <random>
#include <stdexcept>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool can_find_after_recovery(
      const Engine& engine, const Card target, const Card supporter,
      const std::initializer_list<Card> restored_cards,
      const bool permit_payload = false) {
    return engine.pokemon_communication_can_find_after_recovery(
        target, supporter, restored_cards, permit_payload);
  }
  static bool play_team_yell_vstar_recovery(Engine& engine) {
    return engine.play_team_yell_vstar_recovery();
  }
  static void run_turn(Engine& engine) { engine.run_turn(); }
};
}  // namespace sim

namespace {
struct Fixture {
  sim::Scenario scenario{"issue-823-recovery-zones",
                         sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng;
  sim::Engine engine;
  explicit Fixture(const std::uint64_t seed)
      : rng(seed), engine(scenario, recipe, rng) {}
  Fixture(const Fixture&) = delete;
  Fixture& operator=(const Fixture&) = delete;
};

void set_latias_recovery_state(sim::Engine& engine, const sim::Card supporter) {
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None};
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                                     sim::Tool::None});
  state.hand = {supporter, sim::Card::PokemonCommunication, sim::Card::Dipplin};
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  state.discard = {sim::Card::LatiasEx, sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  state.vstar_power_used = true;
}

void team_yell_k0_moves_singleton_between_zones() {
  Fixture fixture{82301};
  set_latias_recovery_state(fixture.engine, sim::Card::TeamYellsCheer);
  // Team Yell's Cheer moves the selected Pokémon from discard into the deck.
  // Pokémon Communication may then return Dipplin and search the restored Latias ex:
  // https://api.pokemontcg.io/v2/cards/swsh9-149
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  // https://github.com/FlareZ123/pokemon-sims/issues/823
  if (!sim::EngineTestAccess::can_find_after_recovery(
          fixture.engine, sim::Card::LatiasEx, sim::Card::TeamYellsCheer,
          {sim::Card::LatiasEx})) {
    throw std::runtime_error("K0 Team Yell recovery failed to expose restored Latias ex");
  }
}

void roseanne_k0_moves_singleton_between_zones() {
  Fixture fixture{82302};
  set_latias_recovery_state(fixture.engine, sim::Card::RoseannesBackup);
  // Roseanne's Backup uses the same discard-to-deck Pokémon movement:
  // https://api.pokemontcg.io/v2/cards/swsh9-148
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://github.com/FlareZ123/pokemon-sims/issues/823
  if (!sim::EngineTestAccess::can_find_after_recovery(
          fixture.engine, sim::Card::LatiasEx, sim::Card::RoseannesBackup,
          {sim::Card::LatiasEx})) {
    throw std::runtime_error("K0 Roseanne recovery failed to expose restored Latias ex");
  }
}

void k1_exact_deck_preserves_the_route() {
  Fixture fixture{82303};
  set_latias_recovery_state(fixture.engine, sim::Card::TeamYellsCheer);
  sim::EngineTestAccess::set_deck_seen(fixture.engine);
  if (!sim::EngineTestAccess::can_find_after_recovery(
          fixture.engine, sim::Card::LatiasEx, sim::Card::TeamYellsCheer,
          {sim::Card::LatiasEx})) {
    throw std::runtime_error("K1 exact-zone recovery failed to expose restored Latias ex");
  }
}

void absent_recovery_card_is_rejected() {
  Fixture fixture{82304};
  set_latias_recovery_state(fixture.engine, sim::Card::TeamYellsCheer);
  sim::EngineTestAccess::state(fixture.engine).discard = {sim::Card::MegaDragonite};
  // A Supporter cannot restore a Pokémon that is absent from discard:
  // https://api.pokemontcg.io/v2/cards/swsh9-149
  // https://github.com/FlareZ123/pokemon-sims/issues/823
  if (sim::EngineTestAccess::can_find_after_recovery(
          fixture.engine, sim::Card::LatiasEx,
          sim::Card::TeamYellsCheer, {sim::Card::LatiasEx})) {
    throw std::runtime_error("preflight invented a Latias ex absent from discard");
  }
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

sim::State team_yell_incense_communication_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None}};
  state.hand = {sim::Card::TeamYellsCheer, sim::Card::EvolutionIncense,
                sim::Card::PokemonCommunication, sim::Card::Dipplin};
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  state.discard = {sim::Card::RegidragoVstar, sim::Card::LatiasEx,
                   sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  state.vstar_power_used = true;
  return state;
}

void team_yell_incense_communication_k1() {
  Fixture fixture{83501};
  sim::EngineTestAccess::state(fixture.engine) = team_yell_incense_communication_state();
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Team Yell's Cheer restores both Pokémon, Evolution Incense finds the VSTAR,
  // and Pokémon Communication exchanges Dipplin for Latias ex:
  // https://api.pokemontcg.io/v2/cards/swsh9-149
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/835
  if (!sim::EngineTestAccess::play_team_yell_vstar_recovery(fixture.engine)) {
    throw std::runtime_error("K1 Team Yell Incense Communication route was rejected");
  }
  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  if (!state.supporter_used || !contains(state.deck, sim::Card::RegidragoVstar) ||
      !contains(state.deck, sim::Card::LatiasEx) ||
      contains(state.discard, sim::Card::LatiasEx)) {
    throw std::runtime_error("K1 Team Yell recovery produced incorrect zones");
  }
}

void team_yell_incense_communication_k0() {
  Fixture fixture{83502};
  sim::State state = team_yell_incense_communication_state();
  state.discard = {sim::Card::RegidragoVstar, sim::Card::RegidragoVstar,
                   sim::Card::RegidragoVstar, sim::Card::LatiasEx,
                   sim::Card::MegaDragonite};
  sim::EngineTestAccess::state(fixture.engine) = state;

  // Exact K0 public-zone accounting must move one fixed-list VSTAR into deck:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc#L60-L105
  // https://api.pokemontcg.io/v2/cards/swsh9-149
  // https://github.com/FlareZ123/pokemon-sims/issues/835
  if (!sim::EngineTestAccess::play_team_yell_vstar_recovery(fixture.engine)) {
    throw std::runtime_error("K0 Team Yell Incense Communication route was rejected");
  }
  const sim::State& recovered = sim::EngineTestAccess::state(fixture.engine);
  if (count(recovered.discard, sim::Card::RegidragoVstar) != 2 ||
      count(recovered.deck, sim::Card::RegidragoVstar) != 1 ||
      !contains(recovered.deck, sim::Card::LatiasEx)) {
    throw std::runtime_error("K0 Team Yell recovery double-counted public copies");
  }
}

void team_yell_incense_communication_full_turn() {
  Fixture fixture{83503};
  sim::EngineTestAccess::state(fixture.engine) = team_yell_incense_communication_state();
  sim::EngineTestAccess::set_deck_seen(fixture.engine);
  sim::EngineTestAccess::run_turn(fixture.engine);

  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  const bool latias_benched = std::any_of(
      state.bench.begin(), state.bench.end(), [](const sim::Pokemon& pokemon) {
        return pokemon.card == sim::Card::LatiasEx;
      });
  if (!state.active || state.active->card != sim::Card::RegidragoVstar ||
      state.active->grass != 2 || state.active->fire != 1 || !state.retreat_used ||
      !latias_benched || !contains(state.discard, sim::Card::PokemonCommunication) ||
      !contains(state.deck, sim::Card::Dipplin)) {
    throw std::runtime_error("full Team Yell Incense Communication route did not complete");
  }
}

void team_yell_incense_communication_item_lock() {
  sim::Scenario scenario{"issue-835-item-lock", sim::DciProfile::StrictJit,
                         sim::LockMode::FullItem, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{83504};
  sim::Engine engine{scenario, recipe, rng};
  sim::EngineTestAccess::state(engine) = team_yell_incense_communication_state();
  sim::EngineTestAccess::set_deck_seen(engine);
  const sim::State before = sim::EngineTestAccess::state(engine);

  if (sim::EngineTestAccess::play_team_yell_vstar_recovery(engine)) {
    throw std::runtime_error("Item lock allowed Team Yell Incense Communication route");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (after.hand != before.hand || after.deck != before.deck ||
      after.discard != before.discard || after.supporter_used) {
    throw std::runtime_error("rejected Item-lock route mutated state");
  }
}

}  // namespace

int main() {
  team_yell_k0_moves_singleton_between_zones();
  roseanne_k0_moves_singleton_between_zones();
  k1_exact_deck_preserves_the_route();
  absent_recovery_card_is_rejected();
  team_yell_incense_communication_k1();
  team_yell_incense_communication_k0();
  team_yell_incense_communication_full_turn();
  team_yell_incense_communication_item_lock();
  return 0;
}
