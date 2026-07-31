#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_pokemon_communication(Engine& engine) {
    return engine.play_pokemon_communication(false);
  }
  static constexpr auto pokemon_communication_pokemon() {
    return Engine::pokemon_communication_pokemon();
  }
};
}  // namespace sim

namespace {
void test_rule_box_lock_still_allows_crobat_v_exchange() {
  const sim::Scenario scenario{"issue-1955-rule-box-lock-crobat-exchange",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::FullRuleBoxAbility, false, 4};
  std::mt19937_64 rng{1955};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);

  sim::State state;
  state.turn = 1;
  state.hand = {sim::Card::PokemonCommunication, sim::Card::CrobatV};
  state.deck = {sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Pokemon Communication may return any Pokemon from hand and search for any
  // Pokemon. Crobat V remains a Basic Pokemon V while Dark Asset is unavailable,
  // so Rule Box Ability lock cannot remove it from the legal exchange universe:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/swsh3-104
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // https://github.com/FlareZ123/pokemon-sims/issues/1955
  if (!sim::EngineTestAccess::play_pokemon_communication(engine)) {
    throw std::runtime_error(
        "Pokemon Communication rejected Crobat V under Rule Box Ability lock.");
  }

  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (std::count(after.hand.begin(), after.hand.end(), sim::Card::RegidragoV) != 1 ||
      std::count(after.deck.begin(), after.deck.end(), sim::Card::CrobatV) != 1 ||
      std::count(after.discard.begin(), after.discard.end(),
                 sim::Card::PokemonCommunication) != 1) {
    throw std::runtime_error(
        "Pokemon Communication did not exchange Crobat V for Regidrago V.");
  }
}

void test_exchange_universe_matches_every_modeled_pokemon() {
  constexpr std::array<sim::Card, 15> modeled_pokemon{
      sim::Card::RegidragoV, sim::Card::RegidragoVstar,
      sim::Card::Dragapult, sim::Card::MegaDragonite,
      sim::Card::DialgaGX, sim::Card::GoodraVstar,
      sim::Card::TapuLeleGX, sim::Card::LatiasEx,
      sim::Card::MawileGX, sim::Card::Oricorio,
      sim::Card::CrobatV, sim::Card::Dipplin,
      sim::Card::Appletun, sim::Card::Pineco,
      sim::Card::ForretressEx};

  // Pokemon Communication's shared return and fallback inventory must equal the
  // simulator's complete is_pokemon universe because its printed effect accepts
  // every Pokemon, regardless of Stage, Rule Box status, or Ability availability:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_001.inc
  // https://github.com/FlareZ123/pokemon-sims/issues/1955
  static_assert(sim::EngineTestAccess::pokemon_communication_pokemon() ==
                modeled_pokemon);
}
}  // namespace

int main() {
  try {
    test_rule_box_lock_still_allows_crobat_v_exchange();
    test_exchange_universe_matches_every_modeled_pokemon();
    std::cout << "Issue 1955 Pokemon Communication Crobat V tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
