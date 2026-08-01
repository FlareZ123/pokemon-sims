#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen,
                        const bool prizes_revealed) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }

  static bool prizes_known(const Engine& engine) {
    return engine.prizes_known();
  }

  static bool route_available(const Engine& engine) {
    return engine.fss_blender_bank_with_complete_manual_schedule_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State complete_visible_t3_route() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 1, 0,
                              sim::Tool::ForestSealStone};
  state.hand = {sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Fire, sim::Card::EarthenVessel,
                sim::Card::PathToPeak};
  state.deck = {sim::Card::StevensResolve, sim::Card::BrilliantBlender,
                sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::GoodraVstar, sim::Card::DialgaGX,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Fire};
  state.supporter_used = true;
  state.manual_energy_used = true;
  return state;
}

sim::Engine make_engine(std::mt19937_64& rng) {
  static const sim::Scenario scenario{"issue-2067-prize-k1-fss-blender",
                                      sim::DciProfile::NoDiscardControl,
                                      sim::LockMode::None, false, 5};
  return sim::Engine{scenario, sim::baseline_recipe(), rng};
}

void both_k1_provenances_admit_the_same_blender_route() {
  std::mt19937_64 deck_rng{20670};
  sim::Engine deck_engine = make_engine(deck_rng);
  sim::EngineTestAccess::set_state(deck_engine, complete_visible_t3_route(),
                                  true, false);
  expect(sim::EngineTestAccess::prizes_known(deck_engine),
         "Deck inspection did not establish K1");
  expect(sim::EngineTestAccess::route_available(deck_engine),
         "Deck-search K1 rejected the immediate Blender route");

  std::mt19937_64 prize_rng{20671};
  sim::Engine prize_engine = make_engine(prize_rng);
  sim::EngineTestAccess::set_state(prize_engine, complete_visible_t3_route(),
                                  false, true);

  // Heavy Ball exposes every face-down Prize card. With a fixed deck list, full
  // Prize inspection establishes the same remaining-deck inventory as a search.
  // The held VSTAR plus visible Grass and Fire schedule covers the remaining T3
  // axes, so Star Alchemy can take Blender and bank the payload immediately:
  // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago V and VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // K1, no-discard-control, and earliest-route specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2067
  expect(sim::EngineTestAccess::prizes_known(prize_engine),
         "Complete Prize inspection did not establish K1");
  expect(sim::EngineTestAccess::route_available(prize_engine),
         "Prize-inspection K1 rejected the immediate Blender route");
}

void true_k0_remains_rejected() {
  std::mt19937_64 rng{20672};
  sim::Engine engine = make_engine(rng);
  sim::EngineTestAccess::set_state(engine, complete_visible_t3_route(),
                                  false, false);

  // Exact hidden-zone inventory is unavailable before a legal inspection:
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2067
  expect(!sim::EngineTestAccess::prizes_known(engine),
         "The true-K0 control unexpectedly had hidden-zone knowledge");
  expect(!sim::EngineTestAccess::route_available(engine),
         "The immediate Blender route was admitted at true K0");
}
}  // namespace

int main() {
  both_k1_provenances_admit_the_same_blender_route();
  true_k0_remains_rejected();
  return 0;
}
