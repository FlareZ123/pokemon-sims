#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string_view>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_professor_burnet(Engine& engine) {
    return engine.play_professor_burnet();
  }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"burnet-ready-turn-gate", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{719};
  sim::Engine engine{scenario, recipe, rng};
};

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State active_vstar_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::Fire};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

void expect_rejected(sim::State state, const std::string_view label) {
  Fixture fixture;
  const std::vector<sim::Card> hand_before = state.hand;
  const std::vector<sim::Card> deck_before = state.deck;
  const std::vector<sim::Card> discard_before = state.discard;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  if (sim::EngineTestAccess::play_professor_burnet(fixture.engine)) {
    throw std::runtime_error(std::string(label) + ": Burnet must be held.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (after.hand != hand_before || after.deck != deck_before ||
      after.discard != discard_before || after.supporter_used) {
    throw std::runtime_error(std::string(label) +
                             ": rejection must preserve every resource.");
  }
}

void test_manual_attachment_used_holds_burnet() {
  sim::State state = active_vstar_state();
  state.manual_energy_used = true;

  // A player may attach one Energy from hand during a turn. Once that attachment
  // has been used, Burnet cannot make this GG Regidrago VSTAR pay Apex Dragon's
  // GGF cost, and a strict-JIT payload discarded now expires at the next turn:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  // https://github.com/FlareZ123/pokemon-sims/issues/719
  expect_rejected(std::move(state), "used manual attachment");
}

void test_stranded_benched_vstar_holds_burnet() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None}};
  state.hand = {sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::Grass};

  // Burnet consumes the one Supporter play, so Tate & Liza cannot subsequently
  // promote the Benched VSTAR. With no Latias ex route and no legal same-turn
  // Energy completion, the current-turn payload cannot create readiness:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/719
  expect_rejected(std::move(state), "stranded Benched VSTAR");
}

void test_held_energy_allows_immediate_route() {
  Fixture fixture;
  sim::State state = active_vstar_state();
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // The unused manual attachment can still put the held Fire on Regidrago VSTAR
  // after Burnet, so this Dragon discard belongs to a legal same-turn ready route:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/719
  if (!sim::EngineTestAccess::play_professor_burnet(fixture.engine)) {
    throw std::runtime_error("Held final Energy must keep the immediate Burnet route live.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.supporter_used || contains(after.hand, sim::Card::ProfessorBurnet) ||
      !contains(after.discard, sim::Card::MegaDragonite)) {
    throw std::runtime_error("The legal held-Energy Burnet route must resolve normally.");
  }
}

void test_held_oricorio_allows_energy_compression() {
  Fixture fixture;
  sim::State state = active_vstar_state();
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::Oricorio};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Vital Dance may find the final Basic Energy while Burnet uses the Supporter
  // slot for the Dragon payload, preserving the existing compression route:
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/719
  if (!sim::EngineTestAccess::play_professor_burnet(fixture.engine)) {
    throw std::runtime_error("Held Oricorio must preserve the legal Burnet route.");
  }
}

void test_earthen_vessel_allows_final_energy_route() {
  Fixture fixture;
  sim::State state = active_vstar_state();
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::EarthenVessel,
                sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Earthen Vessel can discard a separate card, find the final Basic Energy, and
  // leave the unused manual attachment available after Burnet resolves:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/719
  if (!sim::EngineTestAccess::play_professor_burnet(fixture.engine)) {
    throw std::runtime_error("A payable Earthen Vessel route must keep Burnet playable.");
  }
}

void test_latias_route_allows_benched_vstar() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None},
      sim::Pokemon{sim::Card::LatiasEx, 1, 0, 0, sim::Tool::None},
  };
  state.hand = {sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Skyliner removes the Basic Active's Retreat Cost, so Burnet may supply the
  // payload before the legal free retreat to the already powered Benched VSTAR:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/719
  if (!sim::EngineTestAccess::play_professor_burnet(fixture.engine)) {
    throw std::runtime_error("A live Latias route must keep Burnet playable.");
  }
}

void test_no_discard_control_keeps_banking_behavior() {
  sim::Scenario scenario{"burnet-no-discard-control",
                         sim::DciProfile::NoDiscardControl,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{720};
  sim::Engine engine{scenario, recipe, rng};
  sim::State state = active_vstar_state();
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // No-discard-control permits earlier payload banking, so the strict same-turn
  // gate must not alter that profile:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://github.com/FlareZ123/pokemon-sims/issues/719
  if (!sim::EngineTestAccess::play_professor_burnet(engine)) {
    throw std::runtime_error("No-discard-control Burnet banking must remain legal.");
  }
}

}  // namespace

int main() {
  try {
    test_manual_attachment_used_holds_burnet();
    test_stranded_benched_vstar_holds_burnet();
    test_held_energy_allows_immediate_route();
    test_held_oricorio_allows_energy_compression();
    test_earthen_vessel_allows_final_energy_route();
    test_latias_route_allows_benched_vstar();
    test_no_discard_control_keeps_banking_behavior();
    std::cout << "Burnet ready-turn gate tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
