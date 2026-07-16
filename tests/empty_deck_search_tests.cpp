#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static TrialOutcome& outcome(Engine& engine) { return engine.outcome_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool play_mysterious_treasure(Engine& engine, const bool permit_payload) {
    return engine.play_mysterious_treasure(permit_payload);
  }
  static bool play_ultra_ball(Engine& engine, const bool permit_payload) {
    return engine.play_ultra_ball(permit_payload);
  }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
  static bool use_legacy_star(Engine& engine) { return engine.use_legacy_star(); }
  static bool bench_from_hand(Engine& engine, const Card card, const bool resolve_entry) {
    return engine.bench_from_hand(card, resolve_entry);
  }
  static bool play_crispin(Engine& engine) { return engine.play_crispin(); }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"empty-deck-search", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{322};
  sim::TraceLog trace{true, {}};
  sim::Engine engine;

  explicit Fixture(sim::State state)
      : engine(scenario, recipe, rng, &trace) {
    sim::EngineTestAccess::set_state(engine, std::move(state));
  }
};

sim::State ultra_payload_state(const sim::Card outlet, std::vector<sim::Card> deck,
                               const bool supporter_used = true) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::UltraBall, outlet, sim::Card::Dipplin,
                sim::Card::Dipplin};
  state.deck = std::move(deck);
  state.supporter_used = supporter_used;
  return state;
}

void test_search_item_does_not_pay_its_cost() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Dipplin};
  Fixture fixture(std::move(state));

  // A deck search cannot begin with zero cards, and a Trainer known to have no
  // effect cannot be played. Preserve both the Item and its discard cost:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://compendium.pokegym.net/category/7-gameplay/searching-deck-or-discard/
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  if (sim::EngineTestAccess::play_mysterious_treasure(fixture.engine, false)) {
    throw std::runtime_error("Mysterious Treasure must be held when the deck is empty.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (after.hand.size() != 2U || !after.discard.empty()) {
    throw std::runtime_error("An illegal empty-deck search must not spend the Item or discard cost.");
  }
}

void test_ultra_ball_rejects_off_target_payload_outlets() {
  struct Case {
    sim::Card outlet;
    std::vector<sim::Card> deck;
    const char* name;
  };
  const std::vector<Case> cases{
      {sim::Card::MysteriousTreasure,
       {sim::Card::MegaDragonite, sim::Card::Grass}, "Mysterious Treasure"},
      {sim::Card::QuickBall,
       {sim::Card::MegaDragonite, sim::Card::Dragapult}, "Quick Ball"},
      {sim::Card::EarthenVessel,
       {sim::Card::MegaDragonite, sim::Card::Dragapult}, "Earthen Vessel"},
  };

  // Ultra Ball may fetch any Pokémon, but the fetched Dragon reaches discard only
  // when the promised second Item still has a target after that payload leaves deck:
  // Ultra Ball https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // Mysterious Treasure https://api.pokemontcg.io/v2/cards/sm6-113
  // Quick Ball https://api.pokemontcg.io/v2/cards/swsh1-179
  // Earthen Vessel https://api.pokemontcg.io/v2/cards/sv4-163
  // Apex Dragon https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  for (const Case& test_case : cases) {
    Fixture fixture(ultra_payload_state(test_case.outlet, test_case.deck));
    sim::EngineTestAccess::set_deck_seen(fixture.engine);
    if (sim::EngineTestAccess::play_ultra_ball(fixture.engine, true)) {
      throw std::runtime_error(std::string("Ultra Ball must hold before off-target ") +
                               test_case.name + ".");
    }
    const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
    if (after.hand.size() != 4U || after.deck.size() != 2U || !after.discard.empty()) {
      throw std::runtime_error(std::string("The rejected ") + test_case.name +
                               " route must preserve every card.");
    }
  }
}

void test_ultra_ball_accepts_target_legal_payload_outlets() {
  struct Case {
    sim::Card outlet;
    std::vector<sim::Card> deck;
    const char* name;
  };
  const std::vector<Case> cases{
      {sim::Card::MysteriousTreasure,
       {sim::Card::MegaDragonite, sim::Card::Dragapult}, "Mysterious Treasure"},
      {sim::Card::QuickBall,
       {sim::Card::MegaDragonite, sim::Card::RegidragoV}, "Quick Ball"},
      {sim::Card::EarthenVessel,
       {sim::Card::MegaDragonite, sim::Card::Grass}, "Earthen Vessel"},
  };

  // These K1 controls leave the exact target class required by the held second Item,
  // so Ultra Ball may spend its costs and fetch Mega Dragonite ex:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv4-163
  for (const Case& test_case : cases) {
    Fixture fixture(ultra_payload_state(test_case.outlet, test_case.deck));
    sim::EngineTestAccess::set_deck_seen(fixture.engine);
    if (!sim::EngineTestAccess::play_ultra_ball(fixture.engine, true)) {
      throw std::runtime_error(std::string("Ultra Ball must keep the target-legal ") +
                               test_case.name + " route.");
    }
    const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
    if (std::find(after.hand.begin(), after.hand.end(), sim::Card::MegaDragonite) ==
            after.hand.end() ||
        after.deck.size() != 1U || after.discard.size() != 3U) {
      throw std::runtime_error(std::string("The live ") + test_case.name +
                               " route must fetch the payload and pay Ultra Ball.");
    }
  }
}

void test_ultra_ball_preserves_serena_payload_outlet() {
  Fixture fixture(ultra_payload_state(sim::Card::Serena,
                                      {sim::Card::MegaDragonite}, false));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Serena's mandatory hand discard does not require a second deck-search target,
  // so it remains a live outlet after Ultra Ball removes the final payload:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (!sim::EngineTestAccess::play_ultra_ball(fixture.engine, true)) {
    throw std::runtime_error("Ultra Ball must preserve the non-search Serena outlet.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.deck.empty() ||
      std::find(after.hand.begin(), after.hand.end(), sim::Card::MegaDragonite) ==
          after.hand.end()) {
    throw std::runtime_error("The Serena control must fetch the final Dragon payload.");
  }
}

void test_star_alchemy_preserves_the_vstar_power() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::ForestSealStone};
  Fixture fixture(std::move(state));

  // Star Alchemy is an optional deck search. With zero cards, it cannot be used and
  // must preserve the shared one-per-game VSTAR Power resource:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://compendium.pokegym.net/category/7-gameplay/searching-deck-or-discard/
  if (sim::EngineTestAccess::use_fss(fixture.engine)) {
    throw std::runtime_error("Star Alchemy must not activate when the deck is empty.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (after.vstar_power_used || sim::EngineTestAccess::outcome(fixture.engine).used_fss) {
    throw std::runtime_error("A rejected Star Alchemy search must preserve the VSTAR Power.");
  }
}

void test_legacy_star_rejects_impossible_empty_deck_payload() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  Fixture fixture(std::move(state));

  // Legacy Star discards the top seven cards. A publicly empty deck cannot contain
  // a hidden Dragon payload, so the simulator must retain its one-per-game VSTAR
  // Power instead of resolving a zero-card strict-JIT payload attempt:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (sim::EngineTestAccess::use_legacy_star(fixture.engine)) {
    throw std::runtime_error("Legacy Star must not activate for an impossible empty-deck payload route.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (after.vstar_power_used ||
      sim::EngineTestAccess::outcome(fixture.engine).used_legacy_star ||
      !after.discard.empty()) {
    throw std::runtime_error("The rejected empty-deck Legacy Star route must preserve every resource.");
  }
}

void test_legacy_star_keeps_nonempty_payload_and_latias_recovery() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None}};
  state.deck = {sim::Card::MegaDragonite};
  state.discard = {sim::Card::LatiasEx};
  Fixture fixture(std::move(state));

  // A nonempty deck can still supply the current-turn Dragon payload, and Legacy
  // Star may recover Latias ex so Skyliner can support the Benched VSTAR promotion:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/sv8-76
  if (!sim::EngineTestAccess::use_legacy_star(fixture.engine)) {
    throw std::runtime_error("A real nonempty Legacy Star payload and Latias route must remain live.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.vstar_power_used ||
      !sim::EngineTestAccess::outcome(fixture.engine).used_legacy_star ||
      std::find(after.discard.begin(), after.discard.end(), sim::Card::MegaDragonite) == after.discard.end() ||
      std::find(after.hand.begin(), after.hand.end(), sim::Card::LatiasEx) == after.hand.end()) {
    throw std::runtime_error("The nonempty Legacy Star control must discard the payload and recover Latias ex.");
  }
}

void test_optional_entry_search_is_declined_after_legal_bench() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::Oricorio};
  Fixture fixture(std::move(state));

  // Playing Oricorio to the Bench remains legal. Vital Dance says "you may search",
  // so its optional search is declined when the deck has zero cards:
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://compendium.pokegym.net/category/7-gameplay/searching-deck-or-discard/
  if (!sim::EngineTestAccess::bench_from_hand(fixture.engine, sim::Card::Oricorio, true)) {
    throw std::runtime_error("Oricorio should still be legally Benched from hand.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (after.bench.size() != 1U || after.bench.front().card != sim::Card::Oricorio ||
      !after.hand.empty()) {
    throw std::runtime_error("The empty-deck ruling must suppress only Vital Dance, not the Bench play.");
  }
  if (fixture.trace.lines.empty() ||
      fixture.trace.lines.back().find("ABILITY DECLINED") == std::string::npos) {
    throw std::runtime_error("The declined optional search should be explicit in the trace.");
  }
}

void test_search_supporter_does_not_consume_the_supporter_slot() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::Crispin};
  Fixture fixture(std::move(state));

  // Crispin cannot be played when its deck search cannot begin:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  if (sim::EngineTestAccess::play_crispin(fixture.engine)) {
    throw std::runtime_error("Crispin must be held when the deck is empty.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (after.supporter_used || after.hand.size() != 1U || !after.discard.empty()) {
    throw std::runtime_error("A rejected empty-deck Supporter must preserve the Supporter slot and card.");
  }
}

}  // namespace

int main() {
  try {
    test_search_item_does_not_pay_its_cost();
    test_ultra_ball_rejects_off_target_payload_outlets();
    test_ultra_ball_accepts_target_legal_payload_outlets();
    test_ultra_ball_preserves_serena_payload_outlet();
    test_star_alchemy_preserves_the_vstar_power();
    test_legacy_star_rejects_impossible_empty_deck_payload();
    test_legacy_star_keeps_nonempty_payload_and_latias_recovery();
    test_optional_entry_search_is_declined_after_legal_bench();
    test_search_supporter_does_not_consume_the_supporter_slot();
    std::cout << "empty-deck search tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
