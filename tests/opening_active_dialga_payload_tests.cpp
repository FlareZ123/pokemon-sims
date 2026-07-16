#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static State& state(Engine& engine) { return engine.state_; }
  static const State& state(const Engine& engine) { return engine.state_; }
  static const TrialOutcome& outcome(const Engine& engine) { return engine.outcome_; }
  static void choose_opening_active(Engine& engine) { engine.choose_opening_active(); }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static void record_ready(Engine& engine) { engine.record_ready(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State exact_opening_hand() {
  sim::State state;
  state.hand = {sim::Card::DialgaGX, sim::Card::TapuLeleGX,
                sim::Card::RegidragoVstar, sim::Card::Crispin,
                sim::Card::MysteriousTreasure, sim::Card::QuickBall,
                sim::Card::UltraBall};
  return state;
}

sim::State oricorio_dialga_opening_hand() {
  sim::State state;
  state.hand = {sim::Card::QuickBall, sim::Card::TeamYellsCheer,
                sim::Card::MegaDragonite, sim::Card::DialgaGX,
                sim::Card::Oricorio, sim::Card::Powerglass,
                sim::Card::RegidragoVstar};
  return state;
}

void test_opening_choice_preserves_unique_dialga_payload() {
  const sim::Scenario scenario{"opening-dialga-payload", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{348};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, exact_opening_hand());

  // Setup may choose either Basic Active. Dialga-GX is the only known in-hand
  // Apex Dragon payload, while the held Crispin already covers the Supporter axis:
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::TapuLeleGX ||
      !contains(after.hand, sim::Card::DialgaGX)) {
    throw std::runtime_error("Tapu Lele-GX should start Active while Dialga-GX remains the strict-JIT payload.");
  }
}

void test_preserved_dialga_enables_turn_two_vessel_route() {
  const sim::Scenario scenario{"opening-dialga-vessel-ready", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{349};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, exact_opening_hand());
  sim::EngineTestAccess::choose_opening_active(engine);

  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active->entered_turn = 0;
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None},
                 sim::Pokemon{sim::Card::LatiasEx, 1}};
  state.hand = {sim::Card::DialgaGX, sim::Card::RegidragoVstar,
                sim::Card::EarthenVessel};
  state.deck = {sim::Card::Fire};

  // Earthen Vessel discards the preserved Dialga-GX, searches the final Fire,
  // and the manual attachment completes GGF. Regidrago V can evolve on turn two,
  // while Skyliner permits the Basic Active to retreat for free:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://www.pokemon.com/us/pokemon-tcg/rules
  sim::EngineTestAccess::run_turn(engine);
  sim::EngineTestAccess::record_ready(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::RegidragoVstar ||
      after.active->grass != 2 || after.active->fire != 1 ||
      !contains(after.discard, sim::Card::DialgaGX) ||
      sim::EngineTestAccess::outcome(engine).first_ready_turn != 2) {
    throw std::runtime_error("The preserved Dialga-GX should complete the turn-two Earthen Vessel route.");
  }
}

void test_opening_choice_keeps_legacy_order_without_crispin_signal() {
  const sim::Scenario scenario{"opening-dialga-control", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{350};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::DialgaGX, sim::Card::TapuLeleGX};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::DialgaGX ||
      !contains(after.hand, sim::Card::TapuLeleGX)) {
    throw std::runtime_error("The existing Wonder Tag-preserving order should remain without the held-Crispin signal.");
  }
}

void test_opening_choice_preserves_dialga_with_redundant_payload_graph() {
  const sim::Scenario scenario{"opening-dialga-multiple-payloads", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{653};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::DialgaGX, sim::Card::TapuLeleGX,
                sim::Card::RegidragoVstar, sim::Card::Dragapult,
                sim::Card::ProfessorTuro, sim::Card::ForestSealStone,
                sim::Card::Crispin};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Setup may choose either Basic. Crispin covers the immediate Supporter route,
  // Turo can recover the opening Tapu, and Forest Seal Stone supplies a later
  // unrestricted connector, so the additional Dragapult payload does not erase
  // Dialga-GX's DCI value:
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/sv4-171
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://github.com/FlareZ123/pokemon-sims/issues/653
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::TapuLeleGX ||
      !contains(after.hand, sim::Card::DialgaGX) ||
      !contains(after.hand, sim::Card::Dragapult)) {
    throw std::runtime_error("Tapu Lele-GX should start while both Dragon payloads remain in hand.");
  }
}

void test_multiple_payloads_without_recovery_graph_keep_legacy_order() {
  const sim::Scenario scenario{"opening-dialga-multiple-payload-control", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{654};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::DialgaGX, sim::Card::TapuLeleGX,
                sim::Card::Dragapult, sim::Card::Crispin};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::DialgaGX ||
      !contains(after.hand, sim::Card::TapuLeleGX)) {
    throw std::runtime_error("Multiple payloads alone should preserve the legacy Wonder Tag route.");
  }
}

void test_opening_choice_preserves_oricorio_for_vital_dance() {
  const sim::Scenario scenario{"opening-oricorio-dialga-live-route", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{663};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, oricorio_dialga_opening_hand());

  // Dialga-GX may start Active because Mega Dragonite ex already preserves a Dragon
  // payload, Quick Ball supplies Regidrago access, and Oricorio remains a live
  // on-play Energy connector when played from hand to the Bench:
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://github.com/FlareZ123/pokemon-sims/issues/663
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::DialgaGX ||
      !contains(after.hand, sim::Card::Oricorio) ||
      !contains(after.hand, sim::Card::MegaDragonite)) {
    throw std::runtime_error("Dialga-GX should start while the live Vital Dance route remains in hand.");
  }
}

void test_oricorio_dialga_route_controls() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();

  {
    const sim::Scenario scenario{"opening-oricorio-dialga-unique-payload", sim::DciProfile::StrictJit,
                                 sim::LockMode::None, false, 4};
    std::mt19937_64 rng{66301};
    sim::Engine engine(scenario, recipe, rng);
    sim::State state = oricorio_dialga_opening_hand();
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::MegaDragonite),
                     state.hand.end());
    sim::EngineTestAccess::set_state(engine, std::move(state));
    sim::EngineTestAccess::choose_opening_active(engine);
    const sim::State& after = sim::EngineTestAccess::state(engine);
    if (!after.active || after.active->card != sim::Card::Oricorio ||
        !contains(after.hand, sim::Card::DialgaGX)) {
      throw std::runtime_error("Unique Dialga-GX payload value must preserve Dialga-GX in hand.");
    }
  }

  {
    const sim::Scenario scenario{"opening-oricorio-dialga-energy-complete", sim::DciProfile::StrictJit,
                                 sim::LockMode::None, false, 4};
    std::mt19937_64 rng{66302};
    sim::Engine engine(scenario, recipe, rng);
    sim::State state = oricorio_dialga_opening_hand();
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::TeamYellsCheer),
                     state.hand.end());
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::Powerglass),
                     state.hand.end());
    state.hand.push_back(sim::Card::Grass);
    state.hand.push_back(sim::Card::Fire);
    sim::EngineTestAccess::set_state(engine, std::move(state));
    sim::EngineTestAccess::choose_opening_active(engine);
    const sim::State& after = sim::EngineTestAccess::state(engine);
    if (!after.active || after.active->card != sim::Card::Oricorio ||
        !contains(after.hand, sim::Card::DialgaGX)) {
      throw std::runtime_error("A hand already holding both Energy types must preserve the Dialga payload route.");
    }
  }

  {
    const sim::Scenario scenario{"opening-oricorio-dialga-ability-lock", sim::DciProfile::StrictJit,
                                 sim::LockMode::FullRuleBoxAbility, false, 4};
    std::mt19937_64 rng{66303};
    sim::Engine engine(scenario, recipe, rng);
    sim::EngineTestAccess::set_state(engine, oricorio_dialga_opening_hand());
    sim::EngineTestAccess::choose_opening_active(engine);
    const sim::State& after = sim::EngineTestAccess::state(engine);
    if (!after.active || after.active->card != sim::Card::Oricorio ||
        !contains(after.hand, sim::Card::DialgaGX)) {
      throw std::runtime_error("Rule Box Ability lock must prevent preserving a dead Vital Dance route.");
    }
  }

  {
    const sim::Scenario scenario{"opening-oricorio-dialga-no-regi-route", sim::DciProfile::StrictJit,
                                 sim::LockMode::None, false, 4};
    std::mt19937_64 rng{66304};
    sim::Engine engine(scenario, recipe, rng);
    sim::State state = oricorio_dialga_opening_hand();
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::QuickBall),
                     state.hand.end());
    sim::EngineTestAccess::set_state(engine, std::move(state));
    sim::EngineTestAccess::choose_opening_active(engine);
    const sim::State& after = sim::EngineTestAccess::state(engine);
    if (!after.active || after.active->card != sim::Card::Oricorio ||
        !contains(after.hand, sim::Card::DialgaGX)) {
      throw std::runtime_error("Without the public Regidrago route, the selector must retain Dialga-GX.");
    }
  }

  // Vital Dance requires an unlocked on-play Ability, while the redundant-payload
  // route depends on the public hand graph rather than future deck knowledge:
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
}

}  // namespace

int main() {
  test_opening_choice_preserves_unique_dialga_payload();
  test_preserved_dialga_enables_turn_two_vessel_route();
  test_opening_choice_keeps_legacy_order_without_crispin_signal();
  test_opening_choice_preserves_dialga_with_redundant_payload_graph();
  test_multiple_payloads_without_recovery_graph_keep_legacy_order();
  test_opening_choice_preserves_oricorio_for_vital_dance();
  test_oricorio_dialga_route_controls();
  return 0;
}
