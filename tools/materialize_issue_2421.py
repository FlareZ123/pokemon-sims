from pathlib import Path
import textwrap

# Tate evolution and switch readiness must use semantic Apex payment/progress.
tate = Path("src/trace_engine_v2/part_issue_1070_tate_after_vstar_search_override.inc")
text = tate.read_text(encoding="utf-8")
replacements = [
    (
        """           state_.active->entered_turn < state_.turn &&
           state_.active->grass >= 2 && state_.active->fire >= 1;""",
        """           state_.active->entered_turn < state_.turn &&
           pays_apex_energy_cost(*state_.active); // DDE semantics: https://github.com/FlareZ123/pokemon-sims/issues/2421""",
    ),
    (
        """      return pokemon.card == Card::RegidragoV &&
          pokemon.entered_turn < state_.turn && pokemon.grass >= 2 &&
          pokemon.fire >= 1;""",
        """      return pokemon.card == Card::RegidragoV &&
          pokemon.entered_turn < state_.turn &&
          pays_apex_energy_cost(pokemon); // DDE semantics: https://github.com/FlareZ123/pokemon-sims/issues/2421""",
    ),
    (
        """      return pokemon.card == Card::RegidragoV &&
          pokemon.entered_turn < state_.turn && pokemon.grass == 1 &&
          pokemon.fire >= 1;""",
        """      // Progress 2 means exactly one Energy unit is still needed. DDE-only
      // is therefore included together with the pre-DDE GF/GG shapes.
      // DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
      // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2421
      return pokemon.card == Card::RegidragoV &&
          pokemon.entered_turn < state_.turn && apex_energy_progress(pokemon) == 2;""",
    ),
    (
        """      return pokemon.card == Card::RegidragoVstar &&
          pokemon.entered_turn < state_.turn && pokemon.grass == 1 &&
          pokemon.fire >= 1;""",
        """      // The same semantic one-attachment-short state applies after evolution.
      // DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
      // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2421
      return pokemon.card == Card::RegidragoVstar &&
          pokemon.entered_turn < state_.turn && apex_energy_progress(pokemon) == 2;""",
    ),
]
for old, new in replacements:
    if old in text:
        text = text.replace(old, new, 1)
    elif new not in text:
        raise SystemExit("Tate evolution/switch predicate changed unexpectedly")
tate.write_text(text, encoding="utf-8")

# The Tate promotion coordinator must preserve the manual attachment when the
# selected VSTAR is already powered, and otherwise use the engine's projected
# Energy selector instead of typed Basic deficits.
attach = Path("src/trace_engine_v2/part_tate_blender_attachment_override.inc")
text = attach.read_text(encoding="utf-8")
old = """    Pokemon* target = best_benched_vstar_for_promotion();
    if (target != nullptr) {
      Card energy = Card::Grass;
      if (target->grass < 2 && hand_count(Card::Grass) > 0) {
        energy = Card::Grass;
      } else if (target->fire < 1 && hand_count(Card::Fire) > 0) {
        energy = Card::Fire;
      } else {
        return attach_manual_tate_blender_original();
      }

      remove_one(state_.hand, energy);
      if (energy == Card::Grass) {
        ++target->grass;
      } else {
        ++target->fire;
      }
      state_.manual_energy_used = true;
      trace("ATTACH", "R-GAME-ENERGY; R-TATE-01; R-RVS-01; P-DCI-01",
            std::string(name(energy)) +
                " manually to the Benched Regidrago VSTAR selected for Tate & Liza promotion.");
      return true;
    }"""
new = """    Pokemon* target = best_benched_vstar_for_promotion();
    if (target != nullptr) {
      if (pays_apex_energy_cost(*target)) {
        // Preserve the once-per-turn manual attachment when the selected VSTAR
        // already pays Apex through DDE plus a Basic Energy.
        // DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
        // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2421
        return false;
      }
      const auto preferred = preferred_manual_energy_from_hand(*target);
      if (!preferred.has_value()) {
        return attach_manual_tate_blender_original();
      }
      const Card energy = *preferred;
      if (!remove_one(state_.hand, energy) || !attach_energy_card(*target, energy)) {
        throw std::logic_error("Tate promotion Energy projection disappeared");
      }
      state_.manual_energy_used = true;
      trace("ATTACH", "R-GAME-ENERGY; R-TATE-01; R-RVS-01; P-DCI-01",
            std::string(name(energy)) +
                " manually to the Benched Regidrago VSTAR selected for Tate & Liza promotion.");
      return true;
    }"""
if old in text:
    text = text.replace(old, new, 1)
elif new not in text:
    raise SystemExit("Tate promotion coordinator changed unexpectedly")
attach.write_text(text, encoding="utf-8")

# #2315 specifically needs a held Basic Energy that can finish the target after
# Tate switches it Active. Project each physical Basic independently so DDE's
# alternative type coverage is never added as two separate deficits.
supporter = Path("src/trace_engine_v2/part_issue_2225_arven_vessel_supporter_override.inc")
text = supporter.read_text(encoding="utf-8")
old = """    const Pokemon* target = best_benched_vstar_for_promotion();
    if (target == nullptr) return false;
    const int missing_grass = std::max(0, 2 - target->grass);
    const int missing_fire = std::max(0, 1 - target->fire);
    if (missing_grass + missing_fire != 1) return false;
    const Card needed_energy = missing_grass == 1 ? Card::Grass : Card::Fire;

    // Tate & Liza's switch mode resolves the only remaining board-position axis,"""
new = """    const Pokemon* target = best_benched_vstar_for_promotion();
    if (target == nullptr || pays_apex_energy_cost(*target)) return false;
    const bool held_basic_completes_apex = std::any_of(
        std::array<Card, 2>{Card::Grass, Card::Fire}.begin(),
        std::array<Card, 2>{Card::Grass, Card::Fire}.end(),
        [this, target](const Card energy) {
          if (hand_count(energy) == 0) return false;
          Pokemon projected = *target;
          return attach_energy_card(projected, energy) &&
              pays_apex_energy_cost(projected);
        });
    if (!held_basic_completes_apex) return false;

    // DDE makes Grass and Fire alternative one-card completions, so test each
    // held physical Basic through the actual attachment/payment semantics.
    // DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Confirmed DDE migration bug: https://github.com/FlareZ123/pokemon-sims/issues/2421
    // Tate & Liza's switch mode resolves the only remaining board-position axis,"""
if old in text:
    text = text.replace(old, new, 1)
elif new not in text:
    raise SystemExit("issue-2315 projection changed unexpectedly")
old_return = "    return hand_count(needed_energy) > 0;"
new_return = "    return true; // Completing held Basic was projected above: https://github.com/FlareZ123/pokemon-sims/issues/2421"
if old_return in text:
    text = text.replace(old_return, new_return, 1)
elif new_return not in text:
    raise SystemExit("issue-2315 return changed unexpectedly")
supporter.write_text(text, encoding="utf-8")

Path("tests/issue_2421_tate_dde_routes_tests.cpp").write_text(textwrap.dedent(r'''
#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool known = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = known;
    engine.prizes_revealed_ = known;
  }
  static int apex_progress(const Engine& engine, const Pokemon& pokemon) {
    return engine.apex_energy_progress(pokemon);
  }
  static bool pays_apex(const Engine& engine, const Pokemon& pokemon) {
    return engine.pays_apex_energy_cost(pokemon);
  }
  static bool attach_projected(const Engine& engine, Pokemon& pokemon, const Card energy) {
    return engine.attach_energy_card(pokemon, energy);
  }
  static bool tate_draw_route(const Engine& engine) {
    return engine.tate_draw_after_active_vstar_evolution_route();
  }
  static bool tate_held_switch_route(const Engine& engine) {
    return engine.tate_switch_after_held_vstar_evolution_route();
  }
  static bool issue_2315_switch(const Engine& engine) {
    return engine.issue_2315_tate_switch_finishes_ready_turn();
  }
  static bool attach_manual(Engine& engine) { return engine.attach_manual(); }
  static bool manual_used(const Engine& engine) { return engine.state_.manual_energy_used; }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Pokemon regi(const sim::Card card, const int entered_turn, const int grass,
                  const int fire, const int dde) {
  sim::Pokemon result{card, entered_turn, grass, fire, sim::Tool::None};
  result.double_dragon = dde;
  return result;
}

struct Fixture {
  sim::Scenario scenario{"issue-2421", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2421};
  sim::Engine engine{scenario, recipe, rng};
};

void test_dde_only_is_one_physical_basic_short() {
  Fixture fixture;
  const auto target = regi(sim::Card::RegidragoVstar, 1, 0, 0, 1);
  // One DDE provides two units of every type; one Basic Grass OR Fire completes GGF.
  // DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2421
  expect(sim::EngineTestAccess::apex_progress(fixture.engine, target) == 2,
         "DDE-only was not classified one attachment short");
  for (const sim::Card basic : {sim::Card::Grass, sim::Card::Fire}) {
    auto projected = target;
    expect(sim::EngineTestAccess::attach_projected(fixture.engine, projected, basic),
           "Basic projection could not attach");
    expect(sim::EngineTestAccess::pays_apex(fixture.engine, projected),
           "DDE plus projected Basic did not pay Apex");
  }
}

void test_tate_draw_accepts_dde_ready_active_v() {
  sim::Scenario scenario{"issue-2421-draw", sim::DciProfile::StrictJit,
                         sim::LockMode::TurnTwoItem, false, 4};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{24211};
  sim::Engine engine{scenario, recipe, rng};
  sim::State state;
  state.turn = 2;
  state.active = regi(sim::Card::RegidragoV, 1, 1, 0, 1);
  state.hand = {sim::Card::TateLiza, sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(sim::EngineTestAccess::tate_draw_route(engine),
         "Tate draw route missed prior-turn DDE plus Basic Active Regidrago V");
}

void test_tate_held_switch_accepts_dde_ready_benched_v() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = regi(sim::Card::TapuLeleGX, 1, 0, 0, 0);
  state.bench = {regi(sim::Card::RegidragoV, 1, 0, 1, 1)};
  state.hand = {sim::Card::TateLiza, sim::Card::RegidragoVstar,
                sim::Card::BrilliantBlender};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(sim::EngineTestAccess::tate_held_switch_route(fixture.engine),
         "Held Tate switch route missed DDE plus Basic Benched Regidrago V");
}

void test_tate_coordinator_preserves_attachment_when_target_ready() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = regi(sim::Card::RegidragoV, 3, 0, 0, 0);
  state.bench = {regi(sim::Card::RegidragoVstar, 1, 1, 0, 1)};
  state.hand = {sim::Card::TateLiza, sim::Card::BrilliantBlender,
                sim::Card::Grass};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::attach_manual(fixture.engine),
         "Tate coordinator spent an attachment on a DDE-ready VSTAR");
  expect(!sim::EngineTestAccess::manual_used(fixture.engine),
         "Tate coordinator marked the preserved attachment used");
}

void test_issue_2315_accepts_dde_only_plus_held_basic() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = regi(sim::Card::DialgaGX, 1, 0, 0, 0);
  state.bench = {regi(sim::Card::RegidragoVstar, 1, 0, 0, 1)};
  state.hand = {sim::Card::TateLiza, sim::Card::Grass};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(sim::EngineTestAccess::issue_2315_switch(fixture.engine),
         "Post-payload Tate switch missed DDE-only plus held Basic completion");
}
}  // namespace

int main() {
  try {
    test_dde_only_is_one_physical_basic_short();
    test_tate_draw_accepts_dde_ready_active_v();
    test_tate_held_switch_accepts_dde_ready_benched_v();
    test_tate_coordinator_preserves_attachment_when_target_ready();
    test_issue_2315_accepts_dde_only_plus_held_basic();
    std::cout << "Issue 2421 Tate DDE route tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
''').lstrip(), encoding="utf-8")
