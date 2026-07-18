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

void test_opening_choice_preserves_dialga_with_direct_regi_route() {
  const sim::Scenario scenario{"opening-dialga-direct-regi-route", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{65301};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::DialgaGX,
                sim::Card::Crispin, sim::Card::MysteriousTreasure,
                sim::Card::QuickBall, sim::Card::MegaDragonite,
                sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Dipplin TWM 127 cannot enter play in this model because the deck has no Applin,
  // so the centralized strict-DCI selector admits it as the one-discard Item cost.
  // The resulting Quick Ball or Mysterious Treasure route exposes Regidrago V while
  // Crispin covers the Supporter axis and Mega Dragonite ex remains a second payload:
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/sv6-127
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/653
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::TapuLeleGX ||
      !contains(after.hand, sim::Card::DialgaGX) ||
      !contains(after.hand, sim::Card::MegaDragonite)) {
    throw std::runtime_error("Tapu Lele-GX should start when a payable one-discard Item exposes Regidrago V.");
  }
}

void test_protected_singleton_search_items_preserve_wonder_tag() {
  const sim::Scenario scenario{"opening-dialga-unpayable-search-control", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{78801};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::DialgaGX,
                sim::Card::Crispin, sim::Card::MysteriousTreasure,
                sim::Card::QuickBall, sim::Card::MegaDragonite,
                sim::Card::ChaoticSwell};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Both search Items require a discard before searching. In this strict-DCI hand,
  // every other singleton is protected, so neither Item exposes Regidrago V and
  // Tapu Lele-GX remains in hand for a later Wonder Tag:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/788
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::DialgaGX ||
      !contains(after.hand, sim::Card::TapuLeleGX)) {
    throw std::runtime_error("Unpayable singleton search Items must preserve Wonder Tag.");
  }
}

void test_duplicate_quick_ball_is_a_payable_regi_route() {
  const sim::Scenario scenario{"opening-dialga-duplicate-quick-ball", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{78802};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::DialgaGX,
                sim::Card::Crispin, sim::Card::QuickBall,
                sim::Card::QuickBall, sim::Card::MegaDragonite,
                sim::Card::ChaoticSwell};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // A second Quick Ball may pay the first copy's discard cost:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/788
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::TapuLeleGX ||
      !contains(after.hand, sim::Card::DialgaGX)) {
    throw std::runtime_error("Duplicate Quick Ball should preserve Dialga as a payload.");
  }
}

void test_duplicate_mysterious_treasure_is_a_payable_regi_route() {
  const sim::Scenario scenario{"opening-dialga-duplicate-treasure", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{78803};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::DialgaGX,
                sim::Card::Crispin, sim::Card::MysteriousTreasure,
                sim::Card::MysteriousTreasure, sim::Card::MegaDragonite,
                sim::Card::ChaoticSwell};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // A second Mysterious Treasure may pay the first copy's discard cost:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/788
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::TapuLeleGX ||
      !contains(after.hand, sim::Card::DialgaGX)) {
    throw std::runtime_error("Duplicate Mysterious Treasure should preserve Dialga as a payload.");
  }
}

void test_full_item_lock_rejects_direct_regi_item_signal() {
  const sim::Scenario scenario{"opening-dialga-direct-regi-item-lock", sim::DciProfile::StrictJit,
                               sim::LockMode::FullItem, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{65302};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::DialgaGX,
                sim::Card::Crispin, sim::Card::MysteriousTreasure,
                sim::Card::QuickBall, sim::Card::MegaDragonite,
                sim::Card::ChaoticSwell};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Full Item lock prevents Quick Ball and Mysterious Treasure from supplying the
  // public Regidrago route, so Wonder Tag remains the stronger live connector:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-scenarios
  // https://github.com/FlareZ123/pokemon-sims/issues/653
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::DialgaGX ||
      !contains(after.hand, sim::Card::TapuLeleGX)) {
    throw std::runtime_error("Full Item lock should preserve Tapu Lele-GX as the live connector.");
  }
}

void test_blender_duplicate_crispin_preserves_wonder_tag() {
  const sim::Scenario scenario{"opening-dialga-blender-control", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{65303};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::TapuLeleGX, sim::Card::DialgaGX,
                sim::Card::Crispin, sim::Card::QuickBall,
                sim::Card::MegaDragonite, sim::Card::BrilliantBlender,
                sim::Card::Crispin};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Brilliant Blender already supplies direct payload access, and duplicate Crispin
  // saturates the held Energy-Supporter route. Wonder Tag retains greater marginal
  // value than preserving Dialga-GX in hand:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/653
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::DialgaGX ||
      !contains(after.hand, sim::Card::TapuLeleGX)) {
    throw std::runtime_error("Blender plus duplicate Crispin should preserve Wonder Tag.");
  }
}

}  // namespace

int main() {
  test_opening_choice_preserves_unique_dialga_payload();
  test_preserved_dialga_enables_turn_two_vessel_route();
  test_opening_choice_keeps_legacy_order_without_crispin_signal();
  test_opening_choice_preserves_dialga_with_redundant_payload_graph();
  test_multiple_payloads_without_recovery_graph_keep_legacy_order();
  test_opening_choice_preserves_dialga_with_direct_regi_route();
  test_protected_singleton_search_items_preserve_wonder_tag();
  test_duplicate_quick_ball_is_a_payable_regi_route();
  test_duplicate_mysterious_treasure_is_a_payable_regi_route();
  test_full_item_lock_rejects_direct_regi_item_signal();
  test_blender_duplicate_crispin_preserves_wonder_tag();
  return 0;
}
