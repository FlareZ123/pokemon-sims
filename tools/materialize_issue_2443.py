from pathlib import Path

source = Path('src/trace_engine_v2/part_klara_recovery_override.inc')
text = source.read_text(encoding='utf-8')
old = '''    return !state_.manual_energy_used && grass_needed() + fire_needed() == 1 &&
           ((grass_needed() == 1 && count_of(energy, Card::Grass) > 0) ||
            (fire_needed() == 1 && count_of(energy, Card::Fire) > 0));
'''
new = '''    if (state_.manual_energy_used) return false;
    const Pokemon* target = target_regi();
    if (target == nullptr || pays_apex_energy_cost(*target)) return false;

    // Klara puts recovered Basic Energy into hand. Preserve the historical
    // one-attachment completion boundary by projecting each actually recovered
    // Basic and requiring that the unused manual attachment pays Apex Dragon.
    // Klara: https://api.pokemontcg.io/v2/cards/swsh6-145
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2443
    for (const Card basic : {Card::Grass, Card::Fire}) {
      if (count_of(energy, basic) == 0) continue;
      Pokemon projected = *target;
      if (attach_energy_card(projected, basic) &&
          pays_apex_energy_cost(projected)) {
        return true;
      }
    }
    return false;
'''
if text.count(old) != 1:
    raise SystemExit(f'expected one Klara gate, found {text.count(old)}')
source.write_text(text.replace(old, new), encoding='utf-8')

test = Path('tests/issue_2443_klara_dde_setup_advance_tests.cpp')
test.write_text(r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static bool klara_advances(Engine& engine, const std::vector<Card>& energy) {
    return engine.klara_targets_advance_setup({}, energy);
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
  sim::Scenario scenario{"issue-2443", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2443};
  sim::Engine engine{scenario, recipe, rng};
};

void test_dde_completion(const sim::Card basic) {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = attacker(0, 0, 1);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Klara recovers Basic Energy to hand. With one DDE attached, either Basic
  // is the single manual attachment that completes Apex Dragon's GGF.
  // Klara: https://api.pokemontcg.io/v2/cards/swsh6-145
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2443
  expect(sim::EngineTestAccess::klara_advances(fixture.engine, {basic}),
         "Klara rejected a DDE plus one-Basic Apex completion.");
}

void test_basic_only_control() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = attacker(1, 1, 0);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(sim::EngineTestAccess::klara_advances(fixture.engine, {sim::Card::Grass}),
         "Klara regressed the original GF plus Grass completion.");
}

void test_nonfinishing_basic_stays_rejected() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = attacker(0, 1, 0);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::klara_advances(fixture.engine, {sim::Card::Grass}),
         "Klara accepted a Basic that still leaves Apex one attachment short.");
}

void test_spent_attachment_stays_rejected() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = attacker(0, 0, 1);
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  // Official attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2443
  expect(!sim::EngineTestAccess::klara_advances(fixture.engine, {sim::Card::Grass}),
         "Klara accepted a finishing Basic after the manual attachment was spent.");
}

}  // namespace

int main() {
  try {
    test_dde_completion(sim::Card::Grass);
    test_dde_completion(sim::Card::Fire);
    test_basic_only_control();
    test_nonfinishing_basic_stays_rejected();
    test_spent_attachment_stays_rejected();
    std::cout << "Issue 2443 Klara DDE tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
''', encoding='utf-8')
