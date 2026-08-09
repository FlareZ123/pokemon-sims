from pathlib import Path

source = Path('src/trace_engine_v2/part_011_burnet_thinning_override.inc')
text = source.read_text(encoding='utf-8')
old = '''    const int missing_energy = grass_needed() + fire_needed();
    if (missing_energy == 0) return true;
    if (missing_energy != 1 || state_.manual_energy_used) return false;

    const Card needed_energy = grass_needed() == 1 ? Card::Grass : Card::Fire;
    const bool held_energy_route = hand_count(needed_energy) > 0;
    const bool needed_energy_might_be_in_deck = might_be_unseen(needed_energy);
    const bool held_oricorio_route = needed_energy_might_be_in_deck &&
        hand_count(Card::Oricorio) > 0 && !in_play(Card::Oricorio) &&
        bench_space() > 0 && ability_available_for_pokemon(Card::Oricorio);
    const bool held_vessel_route = needed_energy_might_be_in_deck &&
        !item_locked() && hand_count(Card::EarthenVessel) > 0 &&
        choose_discard(false, true, true, Card::EarthenVessel).has_value();
'''
new = '''    const Pokemon* energy_target = target_regi();
    if (energy_target == nullptr) return false;
    if (pays_apex_energy_cost(*energy_target)) return true;
    if (state_.manual_energy_used) return false;

    // DDE supplies two flexible Energy units to a Dragon, so Grass and Fire can
    // be alternative one-card finishes rather than two additive deficits. Project
    // each physical Basic attachment against the selected Regidrago's Apex cost:
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Energy attachment and Supporter procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2449
    const auto held_finishing_energy = preferred_manual_energy_for(
        *energy_target, [this](const Card card) {
          return (card == Card::Grass || card == Card::Fire) &&
                 hand_count(card) > 0;
        });
    const auto deck_finishing_energy = preferred_manual_energy_for(
        *energy_target, [this](const Card card) {
          return (card == Card::Grass || card == Card::Fire) &&
                 might_be_unseen(card);
        });
    const bool held_energy_route = held_finishing_energy.has_value();
    const bool finishing_energy_might_be_in_deck = deck_finishing_energy.has_value();
    // Vital Dance and Earthen Vessel both put Basic Energy into hand; the still-
    // unused manual attachment then supplies the projected one-card Apex finish:
    // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2449
    const bool held_oricorio_route = finishing_energy_might_be_in_deck &&
        hand_count(Card::Oricorio) > 0 && !in_play(Card::Oricorio) &&
        bench_space() > 0 && ability_available_for_pokemon(Card::Oricorio);
    const bool held_vessel_route = finishing_energy_might_be_in_deck &&
        !item_locked() && hand_count(Card::EarthenVessel) > 0 &&
        choose_discard(false, true, true, Card::EarthenVessel).has_value();
'''
if text.count(old) != 1:
    raise SystemExit(f'expected one Burnet block, found {text.count(old)}')
source.write_text(text.replace(old, new), encoding='utf-8')

test = Path('tests/issue_2449_burnet_dde_ready_tests.cpp')
test.write_text(r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true,
                        const bool prizes_revealed = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool burnet_live(Engine& engine) {
    return engine.professor_burnet_has_live_ready_turn_route();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Pokemon pokemon(const int grass, const int fire, const int dde) {
  sim::Pokemon result{sim::Card::RegidragoVstar, 1, grass, fire,
                      sim::Tool::None};
  result.double_dragon = dde;
  return result;
}

struct Fixture {
  sim::Scenario scenario{"issue-2449", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2449};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State base_state() {
  sim::State state;
  state.turn = 3;
  state.active = pokemon(0, 0, 1);
  state.hand = {sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::MegaDragonite};
  return state;
}

void test_dde_held_basic_finish(const sim::Card basic) {
  Fixture fixture;
  sim::State state = base_state();
  state.hand.push_back(basic);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // One DDE contributes two flexible units, so either held Basic is the single
  // legal manual attachment that completes Apex Dragon's GGF before Burnet payload.
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Official Supporter and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2449
  expect(sim::EngineTestAccess::burnet_live(fixture.engine),
         "Burnet rejected DDE plus a held finishing Basic Energy.");
}

void test_dde_oricorio_finish(const sim::Card basic) {
  Fixture fixture;
  sim::State state = base_state();
  state.hand.push_back(sim::Card::Oricorio);
  state.deck.push_back(basic);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Vital Dance may search Basic Energy into hand. With DDE already attached,
  // either Basic type can then be the unused manual attachment that completes Apex.
  // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2449
  expect(sim::EngineTestAccess::burnet_live(fixture.engine),
         "Burnet rejected the DDE plus Oricorio one-Basic finish.");
}

void test_basic_only_control() {
  Fixture fixture;
  sim::State state = base_state();
  state.active = pokemon(1, 1, 0);
  state.hand.push_back(sim::Card::Grass);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  expect(sim::EngineTestAccess::burnet_live(fixture.engine),
         "Burnet regressed the original one-Grass Basic-only completion.");
}

void test_manual_attachment_spent_rejects_dde_finish() {
  Fixture fixture;
  sim::State state = base_state();
  state.hand.push_back(sim::Card::Grass);
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  expect(!sim::EngineTestAccess::burnet_live(fixture.engine),
         "Burnet accepted a DDE finish after the manual attachment was spent.");
}

}  // namespace

int main() {
  try {
    test_dde_held_basic_finish(sim::Card::Grass);
    test_dde_held_basic_finish(sim::Card::Fire);
    test_dde_oricorio_finish(sim::Card::Grass);
    test_dde_oricorio_finish(sim::Card::Fire);
    test_basic_only_control();
    test_manual_attachment_spent_rejects_dde_finish();
    std::cout << "Issue 2449 Burnet DDE tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
''', encoding='utf-8')
