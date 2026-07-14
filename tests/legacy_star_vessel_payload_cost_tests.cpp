#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static bool use_legacy_star(Engine& engine) { return engine.use_legacy_star(); }
  static bool play_earthen_vessel_with_payload(Engine& engine) {
    return engine.play_earthen_vessel(true);
  }
  static bool attach_manual(Engine& engine) { return engine.attach_manual(); }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

std::vector<sim::Card> vessel_bridge_deck() {
  // Legacy Star removes cards from the vector back. Fire and Dragapult remain below
  // the seven-card discard, while Vessel and six strict-protected Trainers are hit.
  return {sim::Card::Fire, sim::Card::Dragapult, sim::Card::ForestSealStone,
          sim::Card::Powerglass, sim::Card::RoseannesBackup, sim::Card::Lusamine,
          sim::Card::ProfessorTuro, sim::Card::TeamYellsCheer,
          sim::Card::EarthenVessel};
}

std::vector<sim::Card> incense_serena_bridge_deck() {
  // Legacy Star removes from the vector back. Mega Dragonite ex remains as the sole
  // searchable payload while Evolution Incense and Serena enter discard.
  return {sim::Card::MegaDragonite, sim::Card::ForestSealStone,
          sim::Card::Powerglass, sim::Card::RoseannesBackup,
          sim::Card::Lusamine, sim::Card::ProfessorTuro,
          sim::Card::EvolutionIncense, sim::Card::Serena};
}

void test_legacy_star_recovers_vessel_for_held_payload_cost() {
  using namespace sim;
  const Scenario scenario{"legacy-star-held-payload-vessel", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng{4171};
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 0, Tool::None};
  state.hand = {Card::MegaDragonite};
  state.deck = vessel_bridge_deck();

  // Legacy Star may recover Earthen Vessel. Vessel may discard the held Dragon,
  // which both pays its cost and establishes the same-turn Apex Dragon payload:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  if (!EngineTestAccess::use_legacy_star(engine) ||
      !contains(state.hand, Card::EarthenVessel)) {
    throw std::runtime_error("Legacy Star should recover Vessel when the held payload is its live cost.");
  }

  if (!EngineTestAccess::play_earthen_vessel_with_payload(engine) ||
      !contains(state.discard, Card::MegaDragonite) ||
      !contains(state.discarded_this_turn, Card::MegaDragonite) ||
      !contains(state.hand, Card::Fire)) {
    throw std::runtime_error("Vessel must discard the held payload and search the missing Fire Energy.");
  }

  // A player may attach one Energy during the turn. The recovered Fire completes GGF:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (!EngineTestAccess::attach_manual(engine) || !state.active ||
      state.active->grass != 2 || state.active->fire != 1 ||
      !EngineTestAccess::payload_ready(engine)) {
    throw std::runtime_error("The Vessel bridge must complete both Energy and strict-JIT payload axes.");
  }
}

void test_legacy_star_rejects_payload_cost_before_window_opens() {
  using namespace sim;
  const Scenario scenario{"legacy-star-closed-payload-window", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng{4172};
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 1;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 0, Tool::None};
  state.hand = {Card::MegaDragonite};
  state.deck = vessel_bridge_deck();

  // Repository strict-JIT timing opens the payload window from turn 2. Before that
  // window, the held Dragon remains protected and cannot make Vessel payable:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  if (!EngineTestAccess::use_legacy_star(engine)) {
    throw std::runtime_error("The constructed control should still resolve Legacy Star.");
  }
  if (contains(state.hand, Card::EarthenVessel) ||
      !contains(state.hand, Card::MegaDragonite) ||
      EngineTestAccess::payload_ready(engine)) {
    throw std::runtime_error("A closed payload window must preserve the Dragon and reject the Vessel bridge.");
  }
}

void test_legacy_star_recovers_incense_serena_payload_bridge() {
  using namespace sim;
  const Scenario scenario{"legacy-star-incense-serena", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng{4511};
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.deck = incense_serena_bridge_deck();

  // Legacy Star may recover any two discarded cards. Evolution Incense then fetches
  // the remaining Evolution Dragon, and Serena's first mode must discard at least one
  // hand card before drawing to five, so that Dragon is a legal strict-JIT payload:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  if (!EngineTestAccess::use_legacy_star(engine) ||
      !contains(state.hand, Card::EvolutionIncense) ||
      !contains(state.hand, Card::Serena)) {
    throw std::runtime_error("Legacy Star should recover the complete Evolution Incense plus Serena payload bridge.");
  }
}

void test_legacy_star_rejects_incense_serena_after_supporter_use() {
  using namespace sim;
  const Scenario scenario{"legacy-star-incense-serena-spent-supporter", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng{4512};
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.supporter_used = true;
  state.deck = incense_serena_bridge_deck();

  // Only one Supporter may be played during a turn. A spent slot makes recovered
  // Serena unusable for the same-turn bridge:
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://www.pokemon.com/us/pokemon-tcg/rules
  if (!EngineTestAccess::use_legacy_star(engine)) {
    throw std::runtime_error("The control should still resolve Legacy Star's top-seven effect.");
  }
  if (contains(state.hand, Card::Serena)) {
    throw std::runtime_error("Legacy Star must not recover Serena after the Supporter slot is spent.");
  }
}

void test_legacy_star_rejects_incense_serena_without_remaining_payload() {
  using namespace sim;
  const Scenario scenario{"legacy-star-incense-serena-no-target", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng{4513};
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  // Dipplin remains after Legacy Star, while Mega Dragonite ex is itself discarded.
  // The payload is already ready, so no Incense plus Serena recovery is warranted.
  state.deck = {Card::Dipplin, Card::ForestSealStone,
                Card::Powerglass, Card::RoseannesBackup,
                Card::Lusamine, Card::EvolutionIncense,
                Card::Serena, Card::MegaDragonite};

  // Evolution Incense searches an Evolution Pokémon, but no modeled A/S payload
  // remains in deck after the top-seven discard:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (!EngineTestAccess::use_legacy_star(engine)) {
    throw std::runtime_error("The control should resolve Legacy Star and discard the payload directly.");
  }
  if (contains(state.hand, Card::EvolutionIncense) || contains(state.hand, Card::Serena)) {
    throw std::runtime_error("Legacy Star must not recover the bridge when no payload remains to search.");
  }
}

void test_legacy_star_rejects_incense_bridge_without_serena() {
  using namespace sim;
  const Scenario scenario{"legacy-star-incense-no-serena", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng{4514};
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.deck = {Card::MegaDragonite, Card::ForestSealStone,
                Card::Powerglass, Card::RoseannesBackup,
                Card::Lusamine, Card::ProfessorTuro,
                Card::TeamYellsCheer, Card::EvolutionIncense};

  // Legacy Star may return only cards actually present in discard:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (!EngineTestAccess::use_legacy_star(engine)) {
    throw std::runtime_error("The control should still resolve Legacy Star's top-seven effect.");
  }
  if (contains(state.hand, Card::Serena) || contains(state.hand, Card::EvolutionIncense)) {
    throw std::runtime_error("Evolution Incense alone must not be recovered as the two-card Serena bridge.");
  }
}

}  // namespace

int main() {
  try {
    test_legacy_star_recovers_vessel_for_held_payload_cost();
    test_legacy_star_rejects_payload_cost_before_window_opens();
    test_legacy_star_recovers_incense_serena_payload_bridge();
    test_legacy_star_rejects_incense_serena_after_supporter_use();
    test_legacy_star_rejects_incense_serena_without_remaining_payload();
    test_legacy_star_rejects_incense_bridge_without_serena();
    std::cout << "Legacy Star recovery bridge tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
