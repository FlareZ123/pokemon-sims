#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>
namespace sim { struct EngineTestAccess {
 static void set_state(Engine&e,State s){e.state_=std::move(s);e.deck_seen_=true;e.prizes_revealed_=true;}
 static bool attach(Engine&e){return e.attach_powerglass();}
 static bool resolve(Engine&e){return e.resolve_powerglass_end_turn();}
 static bool hold(const Engine&e){return e.hold_payload_outlet_for_post_powerglass_turn();}
 static const State& state(const Engine&e){return e.state_;}
}; }
namespace {
void ex(bool v,const char*m){if(!v)throw std::runtime_error(m);}
sim::Pokemon vstar(int g,int f,int d,sim::Tool tool=sim::Tool::None){sim::Pokemon p{sim::Card::RegidragoVstar,1,g,f,tool};p.double_dragon=d;return p;}
struct F{sim::Scenario sc{"2433",sim::DciProfile::StrictJit,sim::LockMode::None,false,5};sim::DeckRecipe r{sim::double_dragon_modeling_recipe()};std::mt19937_64 rng{2433};sim::Engine e{sc,r,rng};};
void already_ready_no_attach(int g,int f){F x;sim::State s;s.turn=3;s.active=vstar(g,f,1);s.hand={sim::Card::Powerglass};s.discard={sim::Card::Grass,sim::Card::Fire};sim::EngineTestAccess::set_state(x.e,std::move(s));
// DDE + either Basic already pays Apex, so Powerglass must preserve the Tool slot:
// https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/ https://api.pokemontcg.io/v2/cards/swsh12-136
// Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63 ; confirmed: https://github.com/FlareZ123/pokemon-sims/issues/2433
ex(!sim::EngineTestAccess::attach(x.e),"redundant Powerglass attached");}
void dde_only_live(sim::Card b){F x;sim::State s;s.turn=3;s.active=vstar(0,0,1);s.hand={sim::Card::Powerglass,sim::Card::Dragapult};s.discard={b};s.manual_energy_used=true;sim::EngineTestAccess::set_state(x.e,std::move(s));
// DDE alone is exactly one physical Basic short, and either recoverable Basic completes Apex:
// https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/ https://api.pokemontcg.io/v2/cards/sv6pt5-63
ex(sim::EngineTestAccess::attach(x.e),"live DDE Powerglass rejected");}
void no_redundant_end_turn(){F x;sim::State s;s.turn=3;s.active=vstar(1,0,1,sim::Tool::Powerglass);s.discard={sim::Card::Fire};sim::EngineTestAccess::set_state(x.e,std::move(s));ex(!sim::EngineTestAccess::resolve(x.e),"ready DDE attacker recovered redundant Energy");ex(sim::EngineTestAccess::state(x.e).discard.size()==1,"discard mutated");}
void preserve_payload(sim::Card b){F x;sim::State s;s.turn=3;s.active=vstar(0,0,1);s.hand={sim::Card::Powerglass,sim::Card::Dragapult};s.discard={b};s.manual_energy_used=true;sim::EngineTestAccess::set_state(x.e,std::move(s));
// Strict-JIT keeps the one-discard payload outlet when post-attack Powerglass will supply the final Basic:
// https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://api.pokemontcg.io/v2/cards/sv6pt5-63
ex(sim::EngineTestAccess::hold(x.e),"post-Powerglass payload outlet not preserved");}
}
int main(){try{already_ready_no_attach(1,0);already_ready_no_attach(0,1);dde_only_live(sim::Card::Grass);dde_only_live(sim::Card::Fire);no_redundant_end_turn();preserve_payload(sim::Card::Grass);preserve_payload(sim::Card::Fire);std::cout<<"Issue 2433 tests passed\n";return 0;}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
