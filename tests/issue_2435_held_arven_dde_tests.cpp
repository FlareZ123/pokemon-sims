#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>
namespace sim {struct EngineTestAccess{static void set_state(Engine&e,State s){e.state_=std::move(s);e.deck_seen_=true;e.prizes_revealed_=true;}static bool live(const Engine&e){return e.wonder_tag_duplicate_held_arven_has_no_marginal_route();}};}
namespace{void ex(bool x,const char*m){if(!x)throw std::runtime_error(m);}sim::Pokemon v(int g,int f,int d){sim::Pokemon p{sim::Card::RegidragoVstar,1,g,f,sim::Tool::None};p.double_dragon=d;return p;}void run(int g,int f,int d,sim::Card basic,bool expected){sim::Scenario sc{"2435",sim::DciProfile::MatchupFlexJit,sim::LockMode::None,false,5};auto r=sim::double_dragon_modeling_recipe();std::mt19937_64 rng(2435);sim::Engine e(sc,r,rng);sim::State s;s.turn=3;s.active=v(g,f,d);s.hand={sim::Card::TapuLeleGX,sim::Card::Arven,sim::Card::Dipplin};s.deck={sim::Card::Arven,sim::Card::EarthenVessel,sim::Card::ForestSealStone,basic};s.discard={sim::Card::MegaDragonite};s.discarded_this_turn={sim::Card::MegaDragonite};sim::EngineTestAccess::set_state(e,std::move(s));
// Held Arven searches Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv1-166 https://api.pokemontcg.io/v2/cards/sv4-163
// DDE + either Basic pays Apex: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/ https://api.pokemontcg.io/v2/cards/swsh12-136
// Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2435
ex(sim::EngineTestAccess::live(e)==expected,"held-Arven projection mismatch");}}
int main(){try{run(0,0,1,sim::Card::Grass,true);run(0,0,1,sim::Card::Fire,true);run(2,0,0,sim::Card::Fire,true);run(1,0,0,sim::Card::Grass,false);std::cout<<"Issue 2435 tests passed\n";return 0;}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
