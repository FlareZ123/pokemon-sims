#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <optional>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_mysterious_treasure(Engine& engine, const bool permit_payload) {
    return engine.play_mysterious_treasure(permit_payload);
  }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"k0-mysterious-treasure-target", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{693};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{scenario, recipe, rng, &trace};
};

int public_in_play_count(const sim::State& state, const sim::Card card) {
  int count = state.active && state.active->card == card ? 1 : 0;
  count += static_cast<int>(std::count_if(
      state.bench.begin(), state.bench.end(), [card](const sim::Pokemon& pokemon) {
        return pokemon.card == card;
      }));
  if (card == sim::Card::RegidragoV) {
    count += state.active && state.active->card == sim::Card::RegidragoVstar ? 1 : 0;
    count += static_cast<int>(std::count_if(
        state.bench.begin(), state.bench.end(), [](const sim::Pokemon& pokemon) {
          return pokemon.card == sim::Card::RegidragoVstar;
        }));
  }
  return count;
}

sim::State public_accounting_state(const std::optional<sim::Card> hidden_target) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::MegaDragonite};
  state.deck = hidden_target
      ? std::vector<sim::Card>{*hidden_target, sim::Card::Grass,
                               sim::Card::Fire, sim::Card::ChaoticSwell}
      : std::vector<sim::Card>{sim::Card::Grass, sim::Card::Fire,
                               sim::Card::ChaoticSwell};

  for (const auto& [card, copies] : sim::baseline_recipe()) {
    if (!sim::is_dragon_or_psychic(card)) continue;
    const int held = static_cast<int>(std::count(state.hand.begin(), state.hand.end(), card));
    const int hidden = hidden_target && card == *hidden_target ? 1 : 0;
    const int publicly_accounted = held + public_in_play_count(state, card);
    const int remaining_public = copies - publicly_accounted - hidden;
    if (remaining_public < 0) {
      throw std::runtime_error("The Mysterious Treasure fixture over-counted a target copy.");
    }
    if (sim::is_payload(card)) {
      state.hand.insert(state.hand.end(), static_cast<std::size_t>(remaining_public), card);
    } else {
      state.discard.insert(state.discard.end(), static_cast<std::size_t>(remaining_public), card);
    }
  }
  return state;
}

void test_publicly_exhausted_k0_target_is_rejected() {
  Fixture fixture;
  sim::State state = public_accounting_state(std::nullopt);
  const std::vector<sim::Card> hand_before = state.hand;
  const std::vector<sim::Card> deck_before = state.deck;
  const std::vector<sim::Card> discard_before = state.discard;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Mysterious Treasure requires a Psychic or Dragon Pokémon target. Public fixed-list
  // accounting can prove that none remains, so the Item cannot discard a held Dragon
  // only to create an Apex Dragon payload:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  // https://github.com/FlareZ123/pokemon-sims/issues/693
  if (sim::EngineTestAccess::play_mysterious_treasure(fixture.engine, true)) {
    throw std::runtime_error("Mysterious Treasure must reject the publicly exhausted K0 target class.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (after.hand != hand_before || after.deck != deck_before ||
      after.discard != discard_before) {
    throw std::runtime_error("The rejected K0 Mysterious Treasure must preserve every zone.");
  }
}

void test_one_unaccounted_target_keeps_k0_route_live() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(
      fixture.engine, public_accounting_state(sim::Card::Oricorio));

  // One Psychic Pokémon copy remains publicly unaccounted and is physically in the
  // fixture deck, so the ordinary K0 Mysterious Treasure route stays legal:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  // https://github.com/FlareZ123/pokemon-sims/issues/693
  if (!sim::EngineTestAccess::play_mysterious_treasure(fixture.engine, true)) {
    throw std::runtime_error("A target-bearing K0 Mysterious Treasure route must remain playable.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (std::find(after.deck.begin(), after.deck.end(), sim::Card::Oricorio) !=
          after.deck.end() ||
      std::find(after.discard.begin(), after.discard.end(), sim::Card::MysteriousTreasure) ==
          after.discard.end() ||
      !std::any_of(after.discard.begin(), after.discard.end(), sim::is_payload)) {
    throw std::runtime_error("The positive control must pay the Item, discard a payload, and search Oricorio.");
  }
}

}  // namespace

int main() {
  try {
    test_publicly_exhausted_k0_target_is_rejected();
    test_one_unaccounted_target_keeps_k0_route_live();
    std::cout << "K0 Mysterious Treasure target tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
