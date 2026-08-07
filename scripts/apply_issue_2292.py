from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def replace_once(path: str, old: str, new: str) -> None:
    target = ROOT / path
    text = target.read_text(encoding="utf-8")
    if new in text:
        return
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{path}: expected one anchor, found {count}")
    target.write_text(text.replace(old, new, 1), encoding="utf-8")


replace_once(
    "src/trace_engine_v2/part_012.inc",
    "bool issue_1598_bank_prized_treasure_{false};\n\nbool play_gladion() {",
    """bool issue_1598_bank_prized_treasure_{false};

  bool issue_2292_gladion_final_prize_energy_finish(const Card energy) const {
    if (!prizes_known() || !supporter_allowed() || state_.manual_energy_used ||
        hand_count(Card::Gladion) == 0 || hand_count(energy) > 0 ||
        !state_.active || state_.active->card != Card::RegidragoVstar ||
        prize_count_after_reveal(energy) == 0 ||
        grass_needed() + fire_needed() != 1 || !payload_ready()) {
      return false;
    }

    // Gladion may take any known Prize card. When the Active Regidrago VSTAR is
    // exactly one manual attachment from GGF and the strict-JIT payload is already
    // in discard this turn, the known prized missing Energy is an immediate finish
    // even when another same-type copy remains elsewhere in the inspected deck:
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Prize, Supporter, and manual Energy attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, strict-JIT, and earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2292
    return (energy == Card::Grass && grass_needed() == 1 && fire_needed() == 0) ||
           (energy == Card::Fire && grass_needed() == 0 && fire_needed() == 1);
  }

bool play_gladion() {""",
)

replace_once(
    "src/trace_engine_v2/part_012.inc",
    """        const auto prize_energy = final_prized_energy_for_gladion();
        const bool same_energy_absent_from_deck = prize_energy.has_value() &&
            deck_count_after_search_started(*prize_energy) == 0;

        if (prize_energy && same_energy_absent_from_deck && !live_crispin) {
          known_target = *prize_energy;
""",
    """        const auto prize_energy = final_prized_energy_for_gladion();
        const bool same_energy_absent_from_deck = prize_energy.has_value() &&
            deck_count_after_search_started(*prize_energy) == 0;
        const bool direct_prize_energy_finish = prize_energy.has_value() &&
            issue_2292_gladion_final_prize_energy_finish(*prize_energy);

        // Card-name absence is sufficient evidence for the historical Gladion
        // route, while #2292 additionally admits a directly payable K1 Prize finish.
        // A same-type Energy remaining in deck does not make that deck copy
        // accessible after Wonder Tag has already selected Gladion for this turn:
        // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
        // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
        // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
        // One Supporter and one manual attachment per turn: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
        // Earliest-ready and K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2292
        if (prize_energy && (same_energy_absent_from_deck || direct_prize_energy_finish) &&
            !live_crispin) {
          known_target = *prize_energy;
""",
)

test_path = ROOT / "tests/issue_2292_gladion_final_prize_energy_tests.cpp"
test_path.write_text(
    r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = false;
  }

  static bool route_available(const Engine& engine, const Card energy) {
    return engine.issue_2292_gladion_final_prize_energy_finish(energy);
  }
};
}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"issue-2292/exact", sim::DciProfile::StrictJit,
                         sim::LockMode::None, true, 5};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{2292};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State winning_state() {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::Gladion};
  state.deck = {sim::Card::Grass, sim::Card::EarthenVessel,
                sim::Card::QuickBall};
  state.prizes = {sim::Card::Grass, sim::Card::TateLiza,
                  sim::Card::Crispin, sim::Card::RegidragoVstar,
                  sim::Card::DialgaGX, sim::Card::RegidragoV};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  return state;
}

void test_known_prized_grass_is_live_even_with_grass_in_deck() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, winning_state());

  // Quick Ball has already paid Mega Dragonite ex as this turn's strict-JIT
  // payload, so Gladion -> known Prize Grass -> manual attachment is complete.
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Prize, Supporter, and Energy attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 / strict-JIT / earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2292
  if (!sim::EngineTestAccess::route_available(fixture.engine, sim::Card::Grass)) {
    throw std::runtime_error("#2292 known Prize Grass finish was rejected.");
  }
}

void test_route_requires_k1_current_turn_payload_and_unused_actions() {
  Fixture fixture;
  sim::State state = winning_state();
  sim::EngineTestAccess::set_state(fixture.engine, state, false);
  if (sim::EngineTestAccess::route_available(fixture.engine, sim::Card::Grass)) {
    throw std::runtime_error("#2292 route used exact Prize information while K0.");
  }

  state = winning_state();
  state.discarded_this_turn.clear();
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::route_available(fixture.engine, sim::Card::Grass)) {
    throw std::runtime_error("#2292 route accepted without a current-turn JIT payload.");
  }

  state = winning_state();
  state.supporter_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::route_available(fixture.engine, sim::Card::Grass)) {
    throw std::runtime_error("#2292 route accepted after the Supporter action was spent.");
  }

  state = winning_state();
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::route_available(fixture.engine, sim::Card::Grass)) {
    throw std::runtime_error("#2292 route accepted after manual attachment was spent.");
  }
}

void test_route_requires_exact_missing_prized_energy() {
  Fixture fixture;
  sim::State state = winning_state();
  state.active->grass = 2;
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::route_available(fixture.engine, sim::Card::Grass)) {
    throw std::runtime_error("#2292 route accepted Grass when only Fire was missing.");
  }

  state = winning_state();
  state.prizes[0] = sim::Card::Fire;
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::route_available(fixture.engine, sim::Card::Grass)) {
    throw std::runtime_error("#2292 route accepted an Energy absent from known Prizes.");
  }

  state = winning_state();
  state.hand.push_back(sim::Card::Grass);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  if (sim::EngineTestAccess::route_available(fixture.engine, sim::Card::Grass)) {
    throw std::runtime_error("#2292 route spent Gladion when the final Energy was held.");
  }
}

}  // namespace

int main() {
  test_known_prized_grass_is_live_even_with_grass_in_deck();
  test_route_requires_k1_current_turn_payload_and_unused_actions();
  test_route_requires_exact_missing_prized_energy();
}
''',
    encoding="utf-8",
)

cmake = ROOT / "CMakeLists.txt"
cmake_text = cmake.read_text(encoding="utf-8")
marker = "trace_issue_2292_strict_first_seed692"
if marker not in cmake_text:
    cmake_text += r'''

# Wonder Tag may search Gladion for a K1-known final Prize Energy even when another
# same-type copy remains in deck; the Supporter and manual attachment finish T4.
# Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
# Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
# Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
# Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
# Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
# Core Prize, Supporter, Item, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
# Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2292
add_test(NAME trace_issue_2292_strict_first_seed692
  COMMAND regidrago_sim --simulate-this --scenario strict-jit/go-first --seed 692 --require-ready-by 4)
'''
    cmake.write_text(cmake_text, encoding="utf-8")
