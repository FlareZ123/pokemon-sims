#define REGIDRAGO_SIM_NO_MAIN
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

  // Preserve the historical Basic-only one-attachment boundary while adding DDE.
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2449
  expect(sim::EngineTestAccess::burnet_live(fixture.engine),
         "Burnet regressed the original one-Grass Basic-only completion.");
}

void test_two_basic_attachments_away_stays_rejected() {
  Fixture fixture;
  sim::State state = base_state();
  state.active = pokemon(0, 1, 0);
  state.hand.push_back(sim::Card::Grass);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // One Grass improves GGF progress here but does not complete Apex Dragon.
  // The ready-turn gate must continue rejecting routes needing two attachments.
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2449
  expect(!sim::EngineTestAccess::burnet_live(fixture.engine),
         "Burnet accepted a state still two Basic attachments from Apex.");
}

void test_manual_attachment_spent_rejects_dde_finish() {
  Fixture fixture;
  sim::State state = base_state();
  state.hand.push_back(sim::Card::Grass);
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // The only remaining DDE finish is the once-per-turn manual attachment.
  // Official attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2449
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
    test_two_basic_attachments_away_stays_rejected();
    test_manual_attachment_spent_rejects_dde_finish();
    std::cout << "Issue 2449 Burnet DDE tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
