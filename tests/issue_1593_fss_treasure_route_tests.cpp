#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"
#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
namespace sim { struct EngineTestAccess {}; }
namespace {
void expect(bool c,const char*m){if(!c)throw std::runtime_error(m);} 
bool has(const sim::TraceLog&t,const std::string&x){return std::any_of(t.lines.begin(),t.lines.end(),[&](const std::string&l){return l.find(x)!=std::string::npos;});}
struct R{sim::TrialOutcome outcome;sim::TraceLog trace;};
R run(const std::string&scenario,std::uint64_t seed){const auto sc=sim::scenario_by_label(scenario);const auto*d=sim::deck_by_id("regidrago-pineco");expect(sc&&d,"fixture");std::mt19937_64 rng(seed);sim::TraceLog tr{true,{}};sim::Engine e(*sc,d->recipe,rng,&tr);return{e.run(),std::move(tr)};}
void exact(){auto r=run("strict-jit/go-first",16);
// Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
// Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
// Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
// Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1593
expect(r.outcome.first_ready_turn==3&&!r.outcome.setup_failed,"seed16 not T3");expect(has(r.trace,"Star Alchemy searched Mysterious Treasure")&&has(r.trace,"T3 | DISCARD")&&has(r.trace,"Hisuian Goodra VSTAR (Mysterious Treasure cost)")&&has(r.trace,"T3 | READY"),"route absent");}
void controls(){auto a=run("strict-jit/go-second",35);expect(a.outcome.first_ready_turn==2&&!a.outcome.setup_failed,"seed35");auto b=run("strict-jit/go-first",72);expect(b.outcome.first_ready_turn>=3,"seed72 unexpectedly invalid");}
}
int main(){exact();controls();}
