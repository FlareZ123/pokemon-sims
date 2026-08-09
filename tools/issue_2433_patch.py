#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]

def one(path,old,new):
 p=root/path;s=p.read_text()
 if s.count(old)!=1: raise SystemExit(f'{path}: matches={s.count(old)}')
 p.write_text(s.replace(old,new,1))

one('src/trace_engine_v2/part_010.inc', '''    const int active_grass_needed = std::max(0, 2 - state_.active->grass);
    const int active_fire_needed = std::max(0, 1 - state_.active->fire);
    // Powerglass resolves after the attack step. Preserve a one-discard payload
    // outlet for the next turn only when its Active holder lacks exactly one Energy
    // and that Energy is already in discard:
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/issues/944
    return active_grass_needed + active_fire_needed == 1 &&
        ((active_grass_needed == 1 && count_of(state_.discard, Card::Grass) > 0) ||
         (active_fire_needed == 1 && count_of(state_.discard, Card::Fire) > 0));
''', '''    if (pays_apex_energy_cost(*state_.active)) return false;
    for (const Card basic : {Card::Grass, Card::Fire}) {
      if (count_of(state_.discard, basic) == 0) continue;
      Pokemon projected = *state_.active;
      if (attach_energy_card(projected, basic) && pays_apex_energy_cost(projected)) {
        // Preserve the post-Powerglass payload outlet exactly when one recoverable
        // Basic card completes Apex. DDE alone is one physical Basic short even
        // though typed missing-Grass/Fire helpers expose both legal alternatives:
        // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
        // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
        // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2433
        return true;
      }
    }
    return false;
''')

one('src/trace_engine_v2/part_014c.inc', '''    // Powerglass attaches Energy to its own Active holder, not to the policy's
    // separately selected Regidrago line: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    const int active_grass_needed = std::max(0, 2 - state_.active->grass);
    const int active_fire_needed = std::max(0, 1 - state_.active->fire);
    if (active_grass_needed == 0 && active_fire_needed == 0) return false;
    const bool needed_grass_in_discard = active_grass_needed > 0 && count_of(state_.discard, Card::Grass) > 0;
    const bool needed_fire_in_discard = active_fire_needed > 0 && count_of(state_.discard, Card::Fire) > 0;
    if (!needed_grass_in_discard && !needed_fire_in_discard) return false;
''', '''    // Powerglass belongs only on an incomplete Active holder. First prefer a
    // recoverable Basic that semantically completes Apex, which correctly treats
    // DDE plus either Basic as GGF. With no DDE, preserve the older incremental
    // Basic-only behavior when a still-needed typed Basic is recoverable:
    // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Tool/attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2433
    if (pays_apex_energy_cost(*state_.active)) return false;
    bool recoverable_basic_advances = false;
    for (const Card basic : {Card::Grass, Card::Fire}) {
      if (count_of(state_.discard, basic) == 0) continue;
      Pokemon projected = *state_.active;
      if (attach_energy_card(projected, basic) && pays_apex_energy_cost(projected)) {
        recoverable_basic_advances = true;
        break;
      }
    }
    if (!recoverable_basic_advances && state_.active->double_dragon == 0) {
      recoverable_basic_advances =
          (state_.active->grass < 2 && count_of(state_.discard, Card::Grass) > 0) ||
          (state_.active->fire < 1 && count_of(state_.discard, Card::Fire) > 0);
    }
    if (!recoverable_basic_advances) return false;
''')

one('src/trace_engine_v2/part_014c.inc', '''    // The effect can attach only to the attached Active Pokémon. Determine the
    // Energy type from that holder's own GGF deficit: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    const int active_grass_needed = std::max(0, 2 - state_.active->grass);
    const int active_fire_needed = std::max(0, 1 - state_.active->fire);
    if (active_grass_needed == 0 && active_fire_needed == 0) return false;

    Card energy = Card::Grass;
    if (active_grass_needed > 0 && count_of(state_.discard, Card::Grass) > 0) {
      energy = Card::Grass;
    } else if (active_fire_needed > 0 && count_of(state_.discard, Card::Fire) > 0) {
      energy = Card::Fire;
    } else {
      return false;
    }
''', '''    // Re-evaluate the holder at end of turn. A DDE attached after Powerglass can
    // make a previously incomplete attacker ready, in which case the Tool must not
    // attach a redundant Basic. Prefer a Basic whose projected attachment completes
    // Apex; without DDE, preserve the existing incremental typed recovery behavior:
    // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // End-of-turn/attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2433
    if (pays_apex_energy_cost(*state_.active)) return false;
    std::optional<Card> semantic_energy;
    for (const Card basic : {Card::Grass, Card::Fire}) {
      if (count_of(state_.discard, basic) == 0) continue;
      Pokemon projected = *state_.active;
      if (attach_energy_card(projected, basic) && pays_apex_energy_cost(projected)) {
        semantic_energy = basic;
        break;
      }
    }
    Card energy = Card::Grass;
    if (semantic_energy) {
      energy = *semantic_energy;
    } else if (state_.active->double_dragon == 0 && state_.active->grass < 2 &&
               count_of(state_.discard, Card::Grass) > 0) {
      energy = Card::Grass;
    } else if (state_.active->double_dragon == 0 && state_.active->fire < 1 &&
               count_of(state_.discard, Card::Fire) > 0) {
      energy = Card::Fire;
    } else {
      return false;
    }
''')

t=root/'tests/issue_2433_powerglass_dde_tests.cpp'
t.write_text(r'''#define REGIDRAGO_SIM_NO_MAIN
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
''')
