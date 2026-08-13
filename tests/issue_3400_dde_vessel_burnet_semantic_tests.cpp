#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"
#include <random>
#include <stdexcept>
#include <utility>
namespace sim { struct EngineTestAccess { static void set_state(Engine& e, State s) { e.state_=std::move(s); e.deck_seen_=true; e.prizes_revealed_=false; } static bool visible(const Engine& e) { return e.issue_1646_vessel_burnet_finish_visible(); } }; }
namespace {
void expect(bool v, const char* m) { if (!v) throw std::runtime_error(m); }
sim::State make_state(int turn) { sim::State s; s.turn=turn; s.active=sim::Pokemon{sim::Card::RegidragoVstar,1,0,0,sim::Tool::None,1}; s.hand={sim::Card::EarthenVessel,sim::Card::ProfessorBurnet,sim::Card::QuickBall}; s.deck={sim::Card::Grass,sim::Card::Fire,sim::Card::Dragapult,sim::Card::RegidragoV}; return s; }
bool visible(sim::DciProfile d, sim::LockMode l, int turn, int max_turn) { sim::Scenario sc{"issue-3400",d,l,false,max_turn}; std::mt19937_64 rng{3400}; sim::Engine e{sc,sim::baseline_recipe(),rng}; sim::EngineTestAccess::set_state(e,make_state(turn)); return sim::EngineTestAccess::visible(e); }
}
int main() {
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md#L382-L440
  // JIT/lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3400
  expect(visible(sim::DciProfile::StrictJit,sim::LockMode::None,2,2),"T2 rejected");
  expect(visible(sim::DciProfile::MatchupFlexJit,sim::LockMode::FullRuleBoxAbility,4,4),"irrelevant lock rejected");
  expect(!visible(sim::DciProfile::NoDiscardControl,sim::LockMode::None,2,2),"NoDiscardControl admitted");
  expect(!visible(sim::DciProfile::StrictJit,sim::LockMode::FullItem,2,2),"Item lock admitted");
  expect(!visible(sim::DciProfile::StrictJit,sim::LockMode::FullSupporter,2,2),"Supporter lock admitted");
  expect(!visible(sim::DciProfile::StrictJit,sim::LockMode::None,3,2),"expired horizon admitted");
}
