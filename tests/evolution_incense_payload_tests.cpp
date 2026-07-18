#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool play_evolution_incense(Engine& engine, const bool permit_payload) {
    return engine.play_evolution_incense(permit_payload);
  }
  static bool play_mysterious_treasure(Engine& engine, const bool permit_payload) {
    return engine.play_mysterious_treasure(permit_payload);
  }
  static bool play_ultra_ball(Engine& engine, const bool permit_payload) {
    return engine.play_ultra_ball(permit_payload);
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

struct Fixture {
  sim::Scenario scenario{"evolution-incense-payload", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{202};
  sim::Engine engine{scenario, recipe, rng};
};

void test_evolution_incense_fetches_jit_payload_for_mysterious_treasure() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::EvolutionIncense, sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::MegaDragonite};
  state.discard = {sim::Card::ProfessorBurnet};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Evolution Incense may search the Stage 2 Mega Dragonite ex payload even after
  // K1 proves Regidrago VSTAR absent, because Mysterious Treasure remains a live
  // same-turn discard continuation and one deck card remains after the first search:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/sm6-113
  if (!sim::EngineTestAccess::play_evolution_incense(fixture.engine, true) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand, sim::Card::MegaDragonite)) {
    throw std::runtime_error("Evolution Incense should fetch Mega Dragonite ex for a live strict-JIT discard route.");
  }

  if (!sim::EngineTestAccess::play_mysterious_treasure(fixture.engine, true)) {
    throw std::runtime_error("Mysterious Treasure should discard the Evolution Incense payload this turn.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  // Regidrago VSTAR can use an attack from a Dragon Pokémon in discard:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (!contains(after.discard, sim::Card::MegaDragonite) ||
      !contains(after.discarded_this_turn, sim::Card::MegaDragonite)) {
    throw std::runtime_error("Evolution Incense's fetched payload must remain in this turn's discard.");
  }
}

void test_evolution_incense_preserves_final_payload_from_dead_search_outlet() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::EvolutionIncense, sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Fetching the final deck card would leave Mysterious Treasure unable to begin its
  // required later search, so Incense must preserve both resources instead of
  // stranding the strict-JIT payload in hand:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  if (sim::EngineTestAccess::play_evolution_incense(fixture.engine, true)) {
    throw std::runtime_error("Evolution Incense must reject a final-card payload route whose search outlet becomes illegal.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::EvolutionIncense) ||
      !contains(after.hand, sim::Card::MysteriousTreasure) ||
      !contains(after.deck, sim::Card::MegaDragonite) || !after.discard.empty()) {
    throw std::runtime_error("The dead final-card route must preserve Incense, its outlet, and the payload target.");
  }
}

void test_evolution_incense_allows_final_payload_for_serena() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::EvolutionIncense, sim::Card::Serena};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Serena's discard-and-draw mode does not require a later deck search to begin.
  // It remains a live strict-JIT outlet after Incense fetches the final payload:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (!sim::EngineTestAccess::play_evolution_incense(fixture.engine, true)) {
    throw std::runtime_error("Evolution Incense should preserve the final-card Serena payload route.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.deck.empty() || !contains(after.hand, sim::Card::MegaDragonite) ||
      !contains(after.hand, sim::Card::Serena) ||
      !contains(after.discard, sim::Card::EvolutionIncense)) {
    throw std::runtime_error("The Serena control must fetch the final payload and keep Serena available.");
  }
}

void test_evolution_incense_uses_duplicate_incense_for_ultra_ball_cost() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::EvolutionIncense, sim::Card::EvolutionIncense, sim::Card::UltraBall};
  // Regidrago V is a current modeled Pokémon target, so Ultra Ball still has
  // a legal post-cost search effect in both duplicate- and single-Incense controls:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://github.com/FlareZ123/pokemon-sims/issues/866
  state.deck = {sim::Card::MegaDragonite, sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // One Evolution Incense searches Mega Dragonite ex. Ultra Ball may then discard
  // that fetched payload and the distinct surviving Incense as its two other cards:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (!sim::EngineTestAccess::play_evolution_incense(fixture.engine, true)) {
    throw std::runtime_error("Evolution Incense should start the payable duplicate-Incense Ultra Ball route.");
  }
  if (!contains(sim::EngineTestAccess::state(fixture.engine).hand, sim::Card::MegaDragonite)) {
    throw std::runtime_error("Evolution Incense should fetch the Evolution Dragon payload.");
  }
  if (!sim::EngineTestAccess::play_ultra_ball(fixture.engine, true)) {
    throw std::runtime_error("Ultra Ball should discard the fetched payload and surviving Evolution Incense.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.discard, sim::Card::MegaDragonite) ||
      !contains(after.discarded_this_turn, sim::Card::MegaDragonite) ||
      count(after.discard, sim::Card::EvolutionIncense) != 2) {
    throw std::runtime_error("The duplicate-Incense route should leave the payload and both Incense copies in discard.");
  }
}

void test_evolution_incense_rejects_unpayable_single_incense_ultra_ball_route() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::EvolutionIncense, sim::Card::UltraBall};
  // Regidrago V is a current modeled Pokémon target, so Ultra Ball still has
  // a legal post-cost search effect in both duplicate- and single-Incense controls:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://github.com/FlareZ123/pokemon-sims/issues/866
  state.deck = {sim::Card::MegaDragonite, sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // With one Incense and one Ultra Ball, the fetched payload supplies only one of
  // Ultra Ball's two other-card costs. The Incense must remain held:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  if (sim::EngineTestAccess::play_evolution_incense(fixture.engine, true)) {
    throw std::runtime_error("Evolution Incense should reject an Ultra Ball route with no distinct second cost.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::EvolutionIncense) ||
      !contains(after.hand, sim::Card::UltraBall) ||
      !contains(after.deck, sim::Card::MegaDragonite)) {
    throw std::runtime_error("The unpayable single-Incense route must preserve the hand and payload target.");
  }
}

void test_evolution_incense_holds_when_k1_vstar_target_is_absent() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::EvolutionIncense};
  state.deck = {sim::Card::GoodraVstar, sim::Card::Dipplin};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Evolution Incense may search an Evolution Pokémon, but K1 proves the requested
  // Regidrago VSTAR axis target is absent. Goodra VSTAR and Dipplin are unrelated
  // fallback Evolutions after the Dragon payload axis is already satisfied:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/swsh11-136
  // https://api.pokemontcg.io/v2/cards/sv6-127
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  if (sim::EngineTestAccess::play_evolution_incense(fixture.engine, true)) {
    throw std::runtime_error("Evolution Incense should be held when K1 proves Regidrago VSTAR absent.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::EvolutionIncense) ||
      !contains(after.deck, sim::Card::GoodraVstar) || !contains(after.deck, sim::Card::Dipplin)) {
    throw std::runtime_error("The dead VSTAR-only search must preserve Incense and unrelated Evolutions.");
  }
}

void test_evolution_incense_holds_when_k1_payload_targets_are_absent() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::EvolutionIncense, sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // K1 proves that no modeled Evolution Dragon payload remains. Dipplin is a legal
  // Evolution Incense target, but it is outside the Apex Dragon payload set, so it
  // cannot justify spending Incense for the held Mysterious Treasure continuation:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sv6-127
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://api.pokemontcg.io/v2/cards/swsh11-136
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  if (sim::EngineTestAccess::play_evolution_incense(fixture.engine, true)) {
    throw std::runtime_error("Evolution Incense should remain held when K1 proves every Evolution payload absent.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::EvolutionIncense) ||
      !contains(after.hand, sim::Card::MysteriousTreasure) ||
      !contains(after.deck, sim::Card::Dipplin)) {
    throw std::runtime_error("The known-dead payload route must preserve Incense, its outlet, and the fallback Evolution.");
  }
}

void test_evolution_incense_uses_legal_post_search_fallbacks() {
  for (const sim::Card fallback : {sim::Card::MegaDragonite, sim::Card::Dragapult}) {
    Fixture fixture;
    sim::State state;
    state.turn = 2;
    state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
    state.hand = {sim::Card::EvolutionIncense};
    state.deck = {fallback};
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

    // Evolution Incense may search any Evolution Pokémon. Once the K0 search proves
    // Regidrago VSTAR absent, Stage 2 Mega Dragonite ex and Dragapult ex remain legal
    // fallback targets: https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/sv6-130
    if (!sim::EngineTestAccess::play_evolution_incense(fixture.engine, false)) {
      throw std::runtime_error("Evolution Incense should resolve the K0 VSTAR search.");
    }
    const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
    if (!contains(after.hand, fallback) || !after.deck.empty()) {
      throw std::runtime_error("Evolution Incense should take the available legal Evolution fallback.");
    }
  }
}

void test_k1_evolution_incense_requires_outlet_specific_remaining_targets() {
  struct Case {
    sim::Card outlet;
    sim::Card dead_remaining;
    sim::Card live_remaining;
  };
  const Case cases[] = {
      {sim::Card::MysteriousTreasure, sim::Card::Serena, sim::Card::RegidragoVstar},
      {sim::Card::QuickBall, sim::Card::Dragapult, sim::Card::RegidragoV},
      {sim::Card::EarthenVessel, sim::Card::Dipplin, sim::Card::Fire},
  };

  for (const Case& test_case : cases) {
    for (const bool live : {false, true}) {
      Fixture fixture;
      sim::State state;
      state.turn = 2;
      state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                                  sim::Tool::None};
      state.hand = {sim::Card::EvolutionIncense, test_case.outlet};
      state.deck = {sim::Card::MegaDragonite,
                    live ? test_case.live_remaining : test_case.dead_remaining};
      sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
      sim::EngineTestAccess::set_deck_seen(fixture.engine);

      // After Incense removes Mega Dragonite ex, the promised outlet must retain its
      // own printed target class. Treasure needs Psychic or Dragon, Quick Ball needs a
      // Basic Pokémon, and Vessel needs Basic Energy. Known zero-effect Trainers are
      // unplayable, so the dead controls must preserve both cards:
      // Evolution Incense https://api.pokemontcg.io/v2/cards/swsh1-163
      // Mysterious Treasure https://api.pokemontcg.io/v2/cards/sm6-113
      // Quick Ball https://api.pokemontcg.io/v2/cards/swsh1-179
      // Earthen Vessel https://api.pokemontcg.io/v2/cards/sv4-163
      // Mega Dragonite ex https://api.pokemontcg.io/v2/cards/me2pt5-152
      // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      const bool played = sim::EngineTestAccess::play_evolution_incense(fixture.engine, true);
      if (played != live) {
        throw std::runtime_error("Evolution Incense outlet target preflight disagreed with the exact post-search deck.");
      }

      const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
      if (live) {
        if (!contains(after.hand, sim::Card::MegaDragonite)) {
          throw std::runtime_error("A target-legal outlet should keep the Incense payload route live.");
        }
      } else if (!contains(after.hand, sim::Card::EvolutionIncense) ||
                 !contains(after.hand, test_case.outlet) ||
                 !contains(after.deck, sim::Card::MegaDragonite) ||
                 !after.discard.empty()) {
        throw std::runtime_error("An off-target outlet must preserve Incense, the outlet, and the payload.");
      }
    }
  }
}

void test_k0_evolution_incense_preserves_unknown_outlet_targets() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::EvolutionIncense, sim::Card::QuickBall};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // K0 cannot inspect the remaining deck identity before Evolution Incense resolves,
  // so a fixed-list Basic Pokémon target for Quick Ball remains plausible:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  if (!sim::EngineTestAccess::play_evolution_incense(fixture.engine, true)) {
    throw std::runtime_error("K0 should preserve an unknown post-Incense Quick Ball target.");
  }
  if (!contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::MegaDragonite)) {
    throw std::runtime_error("The K0 control should fetch the modeled Evolution payload.");
  }
}

}  // namespace

int main() {
  try {
    test_evolution_incense_fetches_jit_payload_for_mysterious_treasure();
    test_evolution_incense_preserves_final_payload_from_dead_search_outlet();
    test_evolution_incense_allows_final_payload_for_serena();
    test_evolution_incense_uses_duplicate_incense_for_ultra_ball_cost();
    test_evolution_incense_rejects_unpayable_single_incense_ultra_ball_route();
    test_evolution_incense_holds_when_k1_vstar_target_is_absent();
    test_evolution_incense_holds_when_k1_payload_targets_are_absent();
    test_evolution_incense_uses_legal_post_search_fallbacks();
    test_k1_evolution_incense_requires_outlet_specific_remaining_targets();
    test_k0_evolution_incense_preserves_unknown_outlet_targets();
    std::cout << "Evolution Incense payload tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
