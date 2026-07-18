#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <initializer_list>
#include <random>
#include <stdexcept>

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
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None};
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                                     sim::Tool::None});
  state.hand = {supporter, sim::Card::PokemonCommunication,
                sim::Card::Dipplin};
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  state.discard = {sim::Card::LatiasEx, sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  state.vstar_power_used = true;
}

void team_yell_k0_moves_singleton_between_zones() {
  Fixture fixture{82301};
  set_latias_recovery_state(fixture.engine, sim::Card::TeamYellsCheer);

  // Team Yell's Cheer moves the selected Pokémon from discard into the deck.
  // Pokémon Communication may then return Dipplin and search the restored Latias ex.
  // Leaving Latias ex in both public zones makes K0 fixed-list accounting reject
  // this legal associativity edge:
  // https://api.pokemontcg.io/v2/cards/swsh9-149
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  // https://github.com/FlareZ123/pokemon-sims/issues/823
  if (!sim::EngineTestAccess::can_find_after_recovery(
          fixture.engine, sim::Card::LatiasEx, sim::Card::TeamYellsCheer,
          {sim::Card::LatiasEx})) {
    throw std::runtime_error(
        "K0 Team Yell recovery failed to expose restored Latias ex");
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
    throw std::runtime_error(
        "K0 Roseanne recovery failed to expose restored Latias ex");
  }
}

void k1_exact_deck_preserves_the_route() {
  Fixture fixture{82303};
  set_latias_recovery_state(fixture.engine, sim::Card::TeamYellsCheer);
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  if (!sim::EngineTestAccess::can_find_after_recovery(
          fixture.engine, sim::Card::LatiasEx, sim::Card::TeamYellsCheer,
          {sim::Card::LatiasEx})) {
    throw std::runtime_error(
        "K1 exact-zone recovery failed to expose restored Latias ex");
  }
}

void absent_recovery_card_is_rejected() {
  Fixture fixture{82304};
  set_latias_recovery_state(fixture.engine, sim::Card::TeamYellsCheer);
  sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  state.discard = {sim::Card::MegaDragonite};

  // A Supporter cannot restore a Pokémon that is absent from discard:
  // https://api.pokemontcg.io/v2/cards/swsh9-149
  // https://github.com/FlareZ123/pokemon-sims/issues/823
  if (sim::EngineTestAccess::can_find_after_recovery(
          fixture.engine, sim::Card::LatiasEx,
          sim::Card::TeamYellsCheer, {sim::Card::LatiasEx})) {
    throw std::runtime_error(
        "preflight invented a Latias ex that was absent from discard");
  }
}

}  // namespace

int main() {
  team_yell_k0_moves_singleton_between_zones();
  roseanne_k0_moves_singleton_between_zones();
  k1_exact_deck_preserves_the_route();
  absent_recovery_card_is_rejected();
  return 0;
}
