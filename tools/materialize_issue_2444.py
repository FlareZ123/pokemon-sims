from pathlib import Path

source = Path('src/trace_engine_v2/part_014b.inc')
text = source.read_text(encoding='utf-8')
old = '''  bool play_roseanne_energy_recovery() {
    if (!supporter_allowed() || hand_count(Card::RoseannesBackup) == 0 ||
        state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoVstar || need_regi() || need_vstar() ||
        need_active_vstar() || need_payload()) {
      return false;
    }

    const int active_grass_needed = std::max(0, 2 - state_.active->grass);
    const int active_fire_needed = std::max(0, 1 - state_.active->fire);
    if (active_grass_needed + active_fire_needed != 1) return false;
    const Card needed_energy = active_grass_needed == 1 ? Card::Grass : Card::Fire;

    // Roseanne's Backup may choose its Energy mode and shuffle one Energy from the
    // discard pile into the deck. Earthen Vessel can then search that restored Basic
    // Energy for the still-unused manual attachment in the same turn:
    // https://api.pokemontcg.io/v2/cards/swsh9-148
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    if (count_of(state_.discard, needed_energy) == 0 || might_be_unseen(needed_energy) ||
        !has_payable_roseanne_energy_vessel()) {
      return false;
    }
'''
new = '''  std::optional<Card> roseanne_finishing_basic_to_recover() const {
    if (!state_.active || state_.active->card != Card::RegidragoVstar ||
        pays_apex_energy_cost(*state_.active)) {
      return std::nullopt;
    }

    // Roseanne's Backup can restore one Energy from discard. Keep this recovery
    // route limited to a Basic that is absent from the searchable deck and whose
    // projected manual attachment actually completes Apex after accounting for DDE.
    // Roseanne's Backup: https://api.pokemontcg.io/v2/cards/swsh9-148
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2444
    for (const Card basic : {Card::Grass, Card::Fire}) {
      if (count_of(state_.discard, basic) == 0 || might_be_unseen(basic)) continue;
      Pokemon projected = *state_.active;
      if (attach_energy_card(projected, basic) &&
          pays_apex_energy_cost(projected)) {
        return basic;
      }
    }
    return std::nullopt;
  }

  bool play_roseanne_energy_recovery() {
    if (!supporter_allowed() || hand_count(Card::RoseannesBackup) == 0 ||
        state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoVstar || need_regi() || need_vstar() ||
        need_active_vstar() || need_payload()) {
      return false;
    }

    const auto needed_energy = roseanne_finishing_basic_to_recover();
    if (!needed_energy.has_value() || !has_payable_roseanne_energy_vessel()) {
      return false;
    }

    // Roseanne's Backup may choose its Energy mode and shuffle one Energy from the
    // discard pile into the deck. Earthen Vessel can then search that restored Basic
    // Energy for the still-unused manual attachment in the same turn:
    // https://api.pokemontcg.io/v2/cards/swsh9-148
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2444
    const Card recovered_energy = *needed_energy;
'''
if text.count(old) != 1:
    raise SystemExit(f'expected one Roseanne block, found {text.count(old)}')
text = text.replace(old, new)
text = text.replace('''    if (!move_discard_to_deck(needed_energy)) return false;\n''', '''    if (!move_discard_to_deck(recovered_energy)) return false;\n''', 1)
source.write_text(text, encoding='utf-8')

test = Path('tests/issue_2444_roseanne_dde_energy_recovery_tests.cpp')
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
  static std::optional<Card> roseanne_finisher(const Engine& engine) {
    return engine.roseanne_finishing_basic_to_recover();
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
  sim::Scenario scenario{"issue-2444", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2444};
  sim::Engine engine{scenario, recipe, rng};
};

void test_dde_finisher(const sim::Card basic) {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = attacker(0, 0, 1);
  state.discard = {basic};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Roseanne can restore the otherwise unavailable Basic, Vessel can search it,
  // and the unused manual attachment completes Apex alongside one DDE.
  // Roseanne's Backup: https://api.pokemontcg.io/v2/cards/swsh9-148
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2444
  expect(sim::EngineTestAccess::roseanne_finisher(fixture.engine) == basic,
         "Roseanne failed to recognize a DDE one-Basic completion.");
}

void test_basic_only_control() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = attacker(1, 1, 0);
  state.discard = {sim::Card::Grass};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(sim::EngineTestAccess::roseanne_finisher(fixture.engine) == sim::Card::Grass,
         "Roseanne regressed the original GF plus Grass recovery.");
}

void test_nonfinishing_basic_stays_rejected() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = attacker(0, 1, 0);
  state.discard = {sim::Card::Grass};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::roseanne_finisher(fixture.engine).has_value(),
         "Roseanne accepted a recovered Basic that does not complete Apex.");
}

void test_recovery_is_unnecessary_when_same_basic_is_searchable() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = attacker(0, 0, 1);
  state.discard = {sim::Card::Grass};
  state.deck = {sim::Card::Grass, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::roseanne_finisher(fixture.engine).has_value(),
         "Roseanne recovered an Energy that was already searchable in deck.");
}

}  // namespace

int main() {
  try {
    test_dde_finisher(sim::Card::Grass);
    test_dde_finisher(sim::Card::Fire);
    test_basic_only_control();
    test_nonfinishing_basic_stays_rejected();
    test_recovery_is_unnecessary_when_same_basic_is_searchable();
    std::cout << "Issue 2444 Roseanne DDE tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
''', encoding='utf-8')
