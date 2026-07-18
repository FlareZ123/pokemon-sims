#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void choose_opening_active(Engine& engine) { engine.choose_opening_active(); }
};
}  // namespace sim

namespace {
bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State issue_663_hand() {
  sim::State state;
  state.hand = {sim::Card::QuickBall, sim::Card::QuickBall,
                sim::Card::MegaDragonite, sim::Card::DialgaGX,
                sim::Card::Oricorio, sim::Card::Powerglass,
                sim::Card::RegidragoVstar};
  return state;
}

void require_dialga_active_with_oricorio_held(const sim::State& state, const char* message) {
  if (!state.active || state.active->card != sim::Card::DialgaGX ||
      !contains(state.hand, sim::Card::Oricorio)) {
    throw std::runtime_error(message);
  }
}

void require_oricorio_active_with_dialga_held(const sim::State& state, const char* message) {
  if (!state.active || state.active->card != sim::Card::Oricorio ||
      !contains(state.hand, sim::Card::DialgaGX)) {
    throw std::runtime_error(message);
  }
}

void test_payable_issue_hand_preserves_vital_dance() {
  const sim::Scenario scenario{"issue-663-exact", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{663};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, issue_663_hand());

  // Setup can start Dialga-GX. Mega Dragonite ex preserves a second strict-JIT
  // payload, and one Quick Ball may discard the distinct second copy before
  // searching Regidrago V. Held Oricorio remains a later Vital Dance connector:
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://github.com/FlareZ123/pokemon-sims/issues/663
  sim::EngineTestAccess::choose_opening_active(engine);
  require_dialga_active_with_oricorio_held(
      sim::EngineTestAccess::state(engine),
      "A payable issue route should start Dialga-GX and preserve Oricorio.");
}

void test_supporter_search_graph_preserves_vital_dance() {
  const sim::Scenario scenario{"issue-663-supporter-route", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{66301};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::Oricorio, sim::Card::RegidragoVstar,
                sim::Card::StevensResolve, sim::Card::Lusamine,
                sim::Card::Arven, sim::Card::GoodraVstar,
                sim::Card::DialgaGX};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Arven can expose a Basic-search Item and Steven's Resolve can search any three
  // cards, so the public route does not depend on literally holding Quick Ball:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://github.com/FlareZ123/pokemon-sims/issues/663
  sim::EngineTestAccess::choose_opening_active(engine);
  require_dialga_active_with_oricorio_held(
      sim::EngineTestAccess::state(engine),
      "A public Arven or Steven route should preserve Oricorio for Vital Dance.");
}

void test_unique_dialga_payload_remains_protected() {
  const sim::Scenario scenario{"issue-663-unique-payload", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{66302};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = issue_663_hand();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::MegaDragonite),
                   state.hand.end());
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Strict JIT protects the only held modeled payload before the ready turn:
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  sim::EngineTestAccess::choose_opening_active(engine);
  require_oricorio_active_with_dialga_held(
      sim::EngineTestAccess::state(engine),
      "The unique Dialga-GX payload must remain protected in hand.");
}

void test_complete_energy_hand_preserves_payload() {
  const sim::Scenario scenario{"issue-663-energy-complete", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{66303};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = issue_663_hand();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::TeamYellsCheer),
                   state.hand.end());
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::Powerglass),
                   state.hand.end());
  state.hand.push_back(sim::Card::Grass);
  state.hand.push_back(sim::Card::Fire);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Vital Dance has lower marginal value when both required Basic Energy types are
  // already public in hand, so the payload-preserving legacy order remains correct:
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  sim::EngineTestAccess::choose_opening_active(engine);
  require_oricorio_active_with_dialga_held(
      sim::EngineTestAccess::state(engine),
      "A complete GGF Energy hand should preserve Dialga-GX in hand.");
}

void test_no_public_regidrago_route_keeps_legacy_order() {
  const sim::Scenario scenario{"issue-663-no-public-route", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{66304};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::Oricorio, sim::Card::DialgaGX,
                sim::Card::RegidragoVstar, sim::Card::GoodraVstar,
                sim::Card::TeamYellsCheer, sim::Card::Powerglass,
                sim::Card::ChaoticSwell};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // A redundant payload alone does not erase the need for a public Regidrago V
  // connector. Without one, retain Dialga-GX and use the fixed legacy order:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  sim::EngineTestAccess::choose_opening_active(engine);
  require_oricorio_active_with_dialga_held(
      sim::EngineTestAccess::state(engine),
      "No public Regidrago V route should keep the legacy Oricorio-active order.");
}

void test_non_strict_profile_keeps_legacy_order() {
  const sim::Scenario scenario{"issue-663-profile-control", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{66305};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, issue_663_hand());
  sim::EngineTestAccess::choose_opening_active(engine);
  require_oricorio_active_with_dialga_held(
      sim::EngineTestAccess::state(engine),
      "Issue 663's strict-JIT selector must not change no-discard-control openings.");
}
}  // namespace

int main() {
  test_payable_issue_hand_preserves_vital_dance();
  test_supporter_search_graph_preserves_vital_dance();
  test_unique_dialga_payload_remains_protected();
  test_complete_energy_hand_preserves_payload();
  test_no_public_regidrago_route_keeps_legacy_order();
  test_non_strict_profile_keeps_legacy_order();
  return 0;
}
