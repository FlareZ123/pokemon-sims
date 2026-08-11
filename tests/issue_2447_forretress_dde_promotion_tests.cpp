#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>
namespace sim { struct EngineTestAccess {
  static void set_state(Engine& e, State s) { e.state_=std::move(s); e.deck_seen_=true; e.prizes_revealed_=true; }
  static bool resolve(Engine& e,const std::vector<std::size_t>& d){return e.resolve_exploding_energy(d);}
  static const State& state(const Engine& e){return e.state_;}
}; }
namespace {
void expect(bool x,const char* m){if(!x) throw std::runtime_error(m);}
sim::Pokemon mon(sim::Card c,int g=0,int f=0,int d=0){sim::Pokemon p{c,1,g,f,sim::Tool::None};p.double_dragon=d;return p;}
sim::DeckRecipe recipe(){auto r=sim::double_dragon_modeling_recipe();r.emplace_back(sim::Card::Pineco,1);r.emplace_back(sim::Card::ForretressEx,1);return r;}
void run(bool dde){sim::Scenario sc{"2447",sim::DciProfile::StrictJit,sim::LockMode::None,false,5};std::mt19937_64 rng(2447);sim::Engine e(sc,recipe(),rng);sim::State s;s.turn=3;s.active=mon(sim::Card::ForretressEx);s.bench=dde?std::vector<sim::Pokemon>{mon(sim::Card::RegidragoVstar,0,0,1),mon(sim::Card::RegidragoVstar,2,0,0)}:std::vector<sim::Pokemon>{mon(sim::Card::RegidragoVstar,1,1,0),mon(sim::Card::RegidragoVstar,1,0,0)};s.deck={sim::Card::Grass};sim::EngineTestAccess::set_state(e,std::move(s));
// Exploding Energy attaches Basic Grass, then Forretress ex KOs itself and forces a replacement Active:
// https://api.pokemontcg.io/v2/cards/sv4pt5-2 https://www.pokemon.com/us/pokemon-tcg/rules
// DDE supplies two Energy of every type on a Dragon: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
// Apex Dragon costs GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
// Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2447
expect(sim::EngineTestAccess::resolve(e,{1U}),"Exploding Energy failed");auto&a=*sim::EngineTestAccess::state(e).active;if(dde)expect(a.double_dragon==1&&a.grass==1,"DDE-ready VSTAR not promoted");else expect(a.double_dragon==0&&a.grass==2&&a.fire==1,"Basic control regressed");}
}
int main(){try{run(true);run(false);std::cout<<"Issue 2447 tests passed\n";return 0;}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
