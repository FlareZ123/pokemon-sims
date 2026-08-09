#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[1]
p = root / 'src/trace_engine_v2/part_forretress_ex_combo.inc'
s = p.read_text()
old = '''  if (!state_.active && !state_.bench.empty()) {
    const auto promotion_rank = [](const Pokemon& pokemon) {
      const int role = pokemon.card == Card::RegidragoVstar ? 3 :
          (pokemon.card == Card::RegidragoV ? 2 : 1);
      return std::tuple{role, pokemon.grass + pokemon.fire,
                        -pokemon.entered_turn};
    };
'''
new = '''  if (!state_.active && !state_.bench.empty()) {
    const auto promotion_rank = [this](const Pokemon& pokemon) {
      const int role = pokemon.card == Card::RegidragoVstar ? 3 :
          (pokemon.card == Card::RegidragoV ? 2 : 1);
      const bool regidrago = pokemon.card == Card::RegidragoVstar ||
          pokemon.card == Card::RegidragoV;
      const bool apex_paid = regidrago && pays_apex_energy_cost(pokemon);
      bool one_basic_from_apex = false;
      if (regidrago && !apex_paid) {
        for (const Card basic : {Card::Grass, Card::Fire}) {
          Pokemon projected = pokemon;
          if (attach_energy_card(projected, basic) &&
              pays_apex_energy_cost(projected)) {
            one_basic_from_apex = true;
            break;
          }
        }
      }
      // Preserve evolution-stage priority, then prefer the same-stage attacker
      // closest to an immediate Apex Dragon. DDE supplies two Energy of every type:
      // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
      // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Knock Out / replacement Active: https://www.pokemon.com/us/pokemon-tcg/rules
      // DDE semantic contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
      // Policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2447
      const int semantic_energy_class = apex_paid ? 2 :
          (one_basic_from_apex ? 1 : 0);
      const int attached_energy_units = pokemon.grass + pokemon.fire +
          2 * pokemon.double_dragon;
      return std::tuple{role, semantic_energy_class, attached_energy_units,
                        -pokemon.entered_turn};
    };
'''
if s.count(old) != 1:
    raise SystemExit(f'promotion block matches={s.count(old)}')
p.write_text(s.replace(old, new, 1))

t = root / 'tests/issue_2447_forretress_dde_promotion_tests.cpp'
t.write_text(r'''#define REGIDRAGO_SIM_NO_MAIN
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
''')
for rel in ['results/baseline_manifest.json','results/multi_deck_manifest.json','docs/MULTI_DECK_REPORT.md']:
    q=root/rel; z=q.read_text(); old_digest='ece779cdcb23679589981da36900554c25a488378f900d562f94a73830c0ce8b'; new_digest='a9e66a627c7ca7c20c49380beaf19257af5ebbc1187ad98465c830465f18392d'
    if z.count(old_digest)!=1: raise SystemExit(f'{rel} old digest matches={z.count(old_digest)}')
    q.write_text(z.replace(old_digest,new_digest,1))
