#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void establish_k1(Engine& engine) {
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool pays_apex_energy_cost(const Engine& engine, const Pokemon& pokemon) {
    return engine.pays_apex_energy_cost(pokemon);
  }
  static bool issue_2158_available(const Engine& engine) {
    return engine.issue_2158_paid_oricorio_retreat_burnet_available();
  }
  static bool play_issue_2158(Engine& engine) {
    return engine.play_issue_2158_paid_oricorio_retreat_burnet();
  }
  static bool pay_tapu(Engine& engine) {
    return engine.pay_tapu_retreat_to_ready_benched_vstar();
  }
  static void arm_issue_1022(Engine& engine) {
    engine.issue_1022_banked_route_ = true;
  }
  static bool pay_banked_oricorio(Engine& engine) {
    return engine.pay_oricorio_retreat_for_known_t4_fss_route();
  }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::TraceLog trace;
  sim::Engine engine;

  Fixture()
      : scenario{"issue-2430/exact", sim::DciProfile::StrictJit,
                 sim::LockMode::None, false, 5},
        recipe(sim::double_dragon_modeling_recipe()),
        rng(2430),
        trace{true, {}},
        engine(scenario, recipe, rng, &trace) {}
};

sim::Pokemon pokemon(const sim::Card card, const int grass = 0,
                     const int fire = 0, const int dde = 0) {
  sim::Pokemon result{card, 1, grass, fire, sim::Tool::None};
  result.double_dragon = dde;
  return result;
}

void require(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void add_current_turn_payload(sim::State& state) {
  state.discard.push_back(sim::Card::Dragapult);
  state.discarded_this_turn.push_back(sim::Card::Dragapult);
}

void run_issue_2158_variant(const int grass, const int fire, const int dde) {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = pokemon(sim::Card::Oricorio);
  state.bench = {pokemon(sim::Card::RegidragoVstar, grass, fire, dde)};
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::Grass};
  state.deck = {sim::Card::Dragapult, sim::Card::Klara};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::establish_k1(fixture.engine);

  // Oricorio pays its one-Colorless Retreat Cost with the held Basic while Burnet
  // remains available to establish the strict-JIT payload. DDE + either Basic is
  // already sufficient for Apex Dragon:
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/2158
  // https://github.com/FlareZ123/pokemon-sims/issues/2430
  require(sim::EngineTestAccess::issue_2158_available(fixture.engine),
          "Issue-2158 paid Oricorio route rejected an Apex-ready Benched VSTAR.");
  require(sim::EngineTestAccess::play_issue_2158(fixture.engine),
          "Issue-2158 paid Oricorio retreat did not resolve.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  require(after.active && after.active->card == sim::Card::RegidragoVstar &&
              sim::EngineTestAccess::pays_apex_energy_cost(
                  fixture.engine, *after.active),
          "Issue-2158 did not promote the Apex-ready VSTAR.");
  require(std::count(after.hand.begin(), after.hand.end(), sim::Card::ProfessorBurnet) == 1,
          "Issue-2158 retreat consumed the held Burnet Supporter.");
}

void run_tapu_variant(const int grass, const int fire, const int dde) {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = pokemon(sim::Card::TapuLeleGX);
  state.bench = {pokemon(sim::Card::RegidragoVstar, grass, fire, dde)};
  state.hand = {sim::Card::Grass};
  add_current_turn_payload(state);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::establish_k1(fixture.engine);

  // Tapu Lele-GX has a one-Colorless Retreat Cost. With payload and Energy axes
  // complete, the held Basic may pay Retreat and promote the semantic ready VSTAR:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/2430
  require(sim::EngineTestAccess::pay_tapu(fixture.engine),
          "Paid Tapu retreat rejected an Apex-ready Benched VSTAR.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  require(after.active && after.active->card == sim::Card::RegidragoVstar &&
              sim::EngineTestAccess::pays_apex_energy_cost(
                  fixture.engine, *after.active),
          "Paid Tapu retreat did not promote the Apex-ready VSTAR.");
}

void run_banked_oricorio_variant(const int grass, const int fire, const int dde) {
  Fixture fixture;
  sim::State state;
  state.turn = 4;
  state.active = pokemon(sim::Card::Oricorio, 1, 0, 0);
  state.bench = {pokemon(sim::Card::RegidragoVstar, grass, fire, dde)};
  add_current_turn_payload(state);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::establish_k1(fixture.engine);
  sim::EngineTestAccess::arm_issue_1022(fixture.engine);

  // The banked Oricorio already carries the one Energy needed for Retreat. Once
  // FSS/Blender have completed the other axes, the promotion check must accept the
  // same semantic Apex payment states as the rest of the engine:
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/2430
  require(sim::EngineTestAccess::pay_banked_oricorio(fixture.engine),
          "Banked Oricorio retreat rejected an Apex-ready Benched VSTAR.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  require(after.active && after.active->card == sim::Card::RegidragoVstar &&
              sim::EngineTestAccess::pays_apex_energy_cost(
                  fixture.engine, *after.active),
          "Banked Oricorio retreat did not promote the Apex-ready VSTAR.");
}

}  // namespace

int main() {
  // DDE + Grass, DDE + Fire, and the original two-Grass/one-Fire Basic payment
  // are all legal Apex Dragon Energy states:
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/2430
  for (const auto [grass, fire, dde] : std::vector<std::tuple<int, int, int>>{
           {1, 0, 1}, {0, 1, 1}, {2, 1, 0}}) {
    run_issue_2158_variant(grass, fire, dde);
    run_tapu_variant(grass, fire, dde);
    run_banked_oricorio_variant(grass, fire, dde);
  }
  std::cout << "issue-2430 paid-retreat DDE regressions passed\n";
  return 0;
}
