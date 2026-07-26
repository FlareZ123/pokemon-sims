
#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"
#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
namespace sim { struct EngineTestAccess {}; }
namespace {
void expect(bool c,const char*m){if(!c)throw std::runtime_error(m);} bool has(const sim::TraceLog&t,const std::string&x){return std::any_of(t.lines.begin(),t.lines.end(),[&](const std::string&l){return l.find(x)!=std::string::npos;});}
struct R{sim::TrialOutcome outcome;sim::TraceLog trace;}; R run(const std::string&s,std::uint64_t seed){const auto sc=sim::scenario_by_label(s);const auto*d=sim::deck_by_id("regidrago-shell");expect(sc&&d,"fixture");std::mt19937_64 rng(seed);sim::TraceLog t{true,{}};sim::Engine e(*sc,d->recipe,rng,&t);return{e.run(),std::move(t)};}
void exact(){const auto r=run("matchup-flex-jit/go-second",155);
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Professor Turo: https://api.pokemontcg.io/v2/cards/sv4-171
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1595
  expect(r.outcome.first_ready_turn==4&&!r.outcome.setup_failed,"seed 155 did not reach T4");expect(has(r.trace,"known prized Grass Energy")&&has(r.trace,"T3 | ATTACH")&&has(r.trace,"T4 | HOLD QUICK BALL")&&has(r.trace,"T4 | PLAY ITEM")&&has(r.trace,"T4 | PLAY SUPPORTER")&&has(r.trace,"Professor Turo")&&has(r.trace,"T4 | READY"),"seed 155 route missing");}
void controls(){expect(run("strict-jit/go-first",43).outcome.first_ready_turn==2,"seed43");expect(run("no-discard-control/go-first",42).outcome.first_ready_turn==3,"seed42");}
}
int main(){exact();controls();return 0;}
