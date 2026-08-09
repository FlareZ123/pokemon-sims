from pathlib import Path

source = Path('src/trace_engine_v2/part_blender_vstar_axis_override.inc')
text = source.read_text(encoding='utf-8')
old = '''  bool issue_1646_hold_blender_for_burnet_finish_visible() const {
    const bool grass_axis_completes = state_.active &&
        ((!state_.manual_energy_used && state_.active->grass == 1 &&
          hand_count(Card::Grass) > 0) ||
         (state_.manual_energy_used && state_.active->grass >= 2));

    // Earthen Vessel has paid its printed cost with the route-dominated Quick
    // Ball. Before the attachment, the final Grass is held. After the attachment,
    // GGF is complete. In both states the Supporter slot and Professor Burnet are
    // live, so Brilliant Blender would duplicate Burnet's current-turn payload:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, Supporter, attachment, search, discard, and shuffle procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, dynamic DCI, strict JIT, and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1646
    return scenario_.dci == DciProfile::StrictJit &&
        scenario_.locks == LockMode::None && state_.turn == 3 &&
        prizes_known() && supporter_allowed() && state_.active &&
        state_.active->card == Card::RegidragoVstar &&
        state_.active->fire >= 1 && grass_axis_completes && need_payload() &&
        hand_count(Card::ProfessorBurnet) > 0 &&
        hand_count(Card::BrilliantBlender) > 0 &&
        count_of(state_.discard, Card::EarthenVessel) > 0 &&
        count_of(state_.discard, Card::QuickBall) > 0 &&
        count_of(state_.discarded_this_turn, Card::QuickBall) > 0 &&
        payload_might_be_in_deck();
  }
'''
new = '''  bool issue_1646_hold_blender_for_burnet_finish_visible() const {
    const auto energy_axis_completes = [this]() {
      if (!state_.active || state_.active->card != Card::RegidragoVstar) {
        return false;
      }
      if (pays_apex_energy_cost(*state_.active)) return true;
      if (state_.manual_energy_used) return false;

      // Preserve the original pre-attachment hold: when one held Basic can be the
      // legal manual attachment that finishes Apex, Burnet still owns the equal-turn
      // payload route and Blender should remain unused. Project the physical card so
      // DDE and Basic-only states share the same semantic payment test:
      // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
      // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Official manual Energy attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2429
      for (const Card basic : {Card::Grass, Card::Fire}) {
        if (hand_count(basic) == 0) continue;
        Pokemon projected = *state_.active;
        if (attach_energy_card(projected, basic) &&
            pays_apex_energy_cost(projected)) {
          return true;
        }
      }
      return false;
    }();

    // Earthen Vessel has paid its printed cost with the route-dominated Quick
    // Ball. Before the final attachment, a held Basic may finish Apex. After the
    // attachment, semantic Apex payment may be Basic-only or DDE-powered. In both
    // states Burnet supplies the current-turn payload without consuming Blender:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, Supporter, attachment, search, discard, and shuffle procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, dynamic DCI, strict JIT, and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Original route contract: https://github.com/FlareZ123/pokemon-sims/issues/1646
    // Confirmed DDE regression: https://github.com/FlareZ123/pokemon-sims/issues/2429
    return scenario_.dci == DciProfile::StrictJit &&
        scenario_.locks == LockMode::None && state_.turn == 3 &&
        prizes_known() && supporter_allowed() && state_.active &&
        state_.active->card == Card::RegidragoVstar &&
        energy_axis_completes && need_payload() &&
        hand_count(Card::ProfessorBurnet) > 0 &&
        hand_count(Card::BrilliantBlender) > 0 &&
        count_of(state_.discard, Card::EarthenVessel) > 0 &&
        count_of(state_.discard, Card::QuickBall) > 0 &&
        count_of(state_.discarded_this_turn, Card::QuickBall) > 0 &&
        payload_might_be_in_deck();
  }
'''
if text.count(old) != 1:
    raise SystemExit(f'expected one issue-1646 block, found {text.count(old)}')
source.write_text(text.replace(old, new), encoding='utf-8')

test = Path('tests/issue_2429_blender_burnet_dde_hold_tests.cpp')
test.write_text(r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool hold_blender(const Engine& engine) {
    return engine.issue_1646_hold_blender_for_burnet_finish_visible();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Pokemon attacker(const int grass, const int fire, const int dde) {
  sim::Pokemon result{sim::Card::RegidragoVstar, 1, grass, fire,
                      sim::Tool::None};
  result.double_dragon = dde;
  return result;
}

struct Fixture {
  sim::Scenario scenario{"issue-2429", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2429};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State base_state() {
  sim::State state;
  state.turn = 3;
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::BrilliantBlender};
  state.deck = {sim::Card::Dragapult};
  state.discard = {sim::Card::EarthenVessel, sim::Card::QuickBall};
  state.discarded_this_turn = {sim::Card::QuickBall};
  return state;
}

void test_dde_complete_is_held(const sim::Card basic) {
  Fixture fixture;
  sim::State state = base_state();
  state.manual_energy_used = true;
  state.active = attacker(basic == sim::Card::Grass ? 1 : 0,
                          basic == sim::Card::Fire ? 1 : 0, 1);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // One DDE plus either Basic pays Apex, so Burnet can supply the missing payload
  // without spending the one-copy Blender.
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2429
  expect(sim::EngineTestAccess::hold_blender(fixture.engine),
         "DDE-complete Active failed to hold Blender for Burnet.");
}

void test_original_pre_attachment_control() {
  Fixture fixture;
  sim::State state = base_state();
  state.active = attacker(1, 1, 0);
  state.hand.push_back(sim::Card::Grass);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // The original GF state remains a hold because the unused manual Grass attachment
  // completes Apex before Burnet supplies payload.
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official manual attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Original contract: https://github.com/FlareZ123/pokemon-sims/issues/1646
  expect(sim::EngineTestAccess::hold_blender(fixture.engine),
         "Original GF plus held Grass Blender-hold route regressed.");
}

void test_nonfinishing_basic_rejected() {
  Fixture fixture;
  sim::State state = base_state();
  state.active = attacker(0, 1, 0);
  state.hand.push_back(sim::Card::Grass);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  expect(!sim::EngineTestAccess::hold_blender(fixture.engine),
         "Blender was held when one Grass still does not pay Apex.");
}

}  // namespace

int main() {
  try {
    test_dde_complete_is_held(sim::Card::Grass);
    test_dde_complete_is_held(sim::Card::Fire);
    test_original_pre_attachment_control();
    test_nonfinishing_basic_rejected();
    std::cout << "Issue 2429 Blender/Burnet DDE hold tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
''', encoding='utf-8')
