from pathlib import Path
import textwrap

path = Path('src/trace_engine_v2/part_issue_1069_legacy_star_combined_energy_payload_override.inc')
text = path.read_text(encoding='utf-8')
marker = '  bool held_payload_only_outlet_blocks_combined_legacy_star_route() const {\n'
helper = '''  bool legacy_star_basic_completes_apex(const Pokemon& pokemon, const Card energy) const {
    if (energy != Card::Grass && energy != Card::Fire) return false;
    Pokemon projected = pokemon;
    // DDE supplies two Energy of every type, so DDE-only is one physical Basic
    // attachment short and either Grass or Fire is a legal completion.
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // DDE semantic-readiness contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
    // Confirmed migration bug: https://github.com/FlareZ123/pokemon-sims/issues/2423
    return attach_energy_card(projected, energy) && pays_apex_energy_cost(projected);
  }

  std::vector<Card> legacy_star_completing_basics(const Pokemon& pokemon) const {
    std::vector<Card> result;
    for (const Card energy : {Card::Grass, Card::Fire}) {
      if (legacy_star_basic_completes_apex(pokemon, energy)) result.push_back(energy);
    }
    return result;
  }

'''
if helper not in text:
    if marker not in text:
        raise SystemExit('held-payload marker missing')
    text = text.replace(marker, helper + marker, 1)

old = '''    const int missing_grass = std::max(0, 2 - state_.active->grass);
    const int missing_fire = std::max(0, 1 - state_.active->fire);
    if (missing_grass + missing_fire != 1) return false;

    const Card needed_energy = missing_grass == 1 ? Card::Grass : Card::Fire;
    if (hand_count(needed_energy) > 0) return false;
'''
new = '''    const auto completing_basics = legacy_star_completing_basics(*state_.active);
    if (completing_basics.empty()) return false;
    if (std::any_of(completing_basics.begin(), completing_basics.end(),
                    [this](const Card energy) { return hand_count(energy) > 0; })) {
      return false;
    }
'''
if old not in text:
    raise SystemExit('held-payload typed deficit block missing')
text = text.replace(old, new, 1)
old = '''    const bool direct_energy_recovery =
        count_of(state_.discard, needed_energy) > 0;
    const bool public_vessel_fallback =
        count_of(state_.discard, Card::EarthenVessel) > 0 &&
        count_of(state_.deck, needed_energy) > 0;
'''
new = '''    const bool direct_energy_recovery = std::any_of(
        completing_basics.begin(), completing_basics.end(),
        [this](const Card energy) { return count_of(state_.discard, energy) > 0; });
    const bool public_vessel_fallback =
        count_of(state_.discard, Card::EarthenVessel) > 0 &&
        std::any_of(completing_basics.begin(), completing_basics.end(),
                    [this](const Card energy) { return count_of(state_.deck, energy) > 0; });
'''
if old not in text:
    raise SystemExit('held-payload recovery block missing')
text = text.replace(old, new, 1)

old = '''    const int missing_grass = std::max(0, 2 - target->grass);
    const int missing_fire = std::max(0, 1 - target->fire);
    if (missing_grass + missing_fire != 1) return false;
    const Card needed_energy = missing_grass == 1 ? Card::Grass : Card::Fire;
    if (hand_count(needed_energy) > 0) return false;
'''
new = '''    const auto completing_basics = legacy_star_completing_basics(*target);
    if (completing_basics.empty()) return false;
    if (std::any_of(completing_basics.begin(), completing_basics.end(),
                    [this](const Card energy) { return hand_count(energy) > 0; })) {
      return false;
    }
'''
if old not in text:
    raise SystemExit('issue-2314 typed deficit block missing')
text = text.replace(old, new, 1)
old = '''    if (count_of(state_.discard, Card::EarthenVessel) == 0 ||
        count_of(state_.deck, needed_energy) + count_of(state_.discard, needed_energy) == 0) {
      return false;
    }
'''
new = '''    const bool completing_energy_public = std::any_of(
        completing_basics.begin(), completing_basics.end(), [this](const Card energy) {
          return count_of(state_.deck, energy) + count_of(state_.discard, energy) > 0;
        });
    if (count_of(state_.discard, Card::EarthenVessel) == 0 ||
        !completing_energy_public) {
      return false;
    }
'''
if old not in text:
    raise SystemExit('issue-2314 public energy block missing')
text = text.replace(old, new, 1)
old = '''    const bool route_replaced_energy_cost =
        (missing_grass == 0 && hand_count(Card::Grass) > 0) ||
        (missing_fire == 0 && hand_count(Card::Fire) > 0);
'''
new = '''    const bool route_replaced_energy_cost = std::any_of(
        std::array<Card, 2>{Card::Grass, Card::Fire}.begin(),
        std::array<Card, 2>{Card::Grass, Card::Fire}.end(),
        [this, target](const Card energy) {
          return hand_count(energy) > 0 &&
              !legacy_star_basic_completes_apex(*target, energy);
        });
'''
if old not in text:
    raise SystemExit('issue-2314 route-replaced block missing')
text = text.replace(old, new, 1)
# Fix the temporary-array range immediately in generated output.
text = text.replace(
'''    const bool route_replaced_energy_cost = std::any_of(
        std::array<Card, 2>{Card::Grass, Card::Fire}.begin(),
        std::array<Card, 2>{Card::Grass, Card::Fire}.end(),
        [this, target](const Card energy) {
          return hand_count(energy) > 0 &&
              !legacy_star_basic_completes_apex(*target, energy);
        });''',
'''    const std::array<Card, 2> basic_energies{Card::Grass, Card::Fire};
    const bool route_replaced_energy_cost = std::any_of(
        basic_energies.begin(), basic_energies.end(), [this, target](const Card energy) {
          return hand_count(energy) > 0 &&
              !legacy_star_basic_completes_apex(*target, energy);
        });''')

old = '''    const Card needed_energy = target->grass < 2 ? Card::Grass : Card::Fire;

    state_.vstar_power_used = true;'''
new = '''    const auto completing_basics = legacy_star_completing_basics(*target);
    if (completing_basics.empty()) return false;

    state_.vstar_power_used = true;'''
if old not in text:
    raise SystemExit('issue-2314 use fixed needed energy missing')
text = text.replace(old, new, 1)
old = '''    if (count_of(state_.discard, needed_energy) > 0 && recovered.size() < 2U) {
      if (!recover_discard_to_hand(needed_energy)) {
        throw std::logic_error("Issue 2314 Legacy Star Energy target disappeared");
      }
      recovered.push_back(needed_energy);
    } else if (count_of(state_.deck, needed_energy) > 0 && recovered.size() < 2U) {
      if (!recover_discard_to_hand(Card::EarthenVessel)) {
        throw std::logic_error("Issue 2314 Legacy Star Vessel target disappeared");
      }
      recovered.push_back(Card::EarthenVessel);
    }
'''
new = '''    const auto discard_energy = std::find_if(
        completing_basics.begin(), completing_basics.end(),
        [this](const Card energy) { return count_of(state_.discard, energy) > 0; });
    const auto deck_energy = std::find_if(
        completing_basics.begin(), completing_basics.end(),
        [this](const Card energy) { return count_of(state_.deck, energy) > 0; });
    if (discard_energy != completing_basics.end() && recovered.size() < 2U) {
      if (!recover_discard_to_hand(*discard_energy)) {
        throw std::logic_error("Issue 2314 Legacy Star completing Energy disappeared");
      }
      recovered.push_back(*discard_energy);
    } else if (deck_energy != completing_basics.end() && recovered.size() < 2U) {
      if (!recover_discard_to_hand(Card::EarthenVessel)) {
        throw std::logic_error("Issue 2314 Legacy Star Vessel target disappeared");
      }
      recovered.push_back(Card::EarthenVessel);
    }
'''
if old not in text:
    raise SystemExit('issue-2314 recovery selection missing')
text = text.replace(old, new, 1)

old = '''    const int missing_grass = std::max(0, 2 - target->grass);
    const int missing_fire = std::max(0, 1 - target->fire);
    if (missing_grass + missing_fire != 1) return false;
    const Card needed_energy = missing_grass == 1 ? Card::Grass : Card::Fire;
    const int needed_energy_before = needed_energy == Card::Grass ? grass_before : fire_before;
    if (hand_count(needed_energy) <= needed_energy_before ||
        count_of(state_.deck, needed_energy) == 0) {
      return false;
    }
'''
new = '''    const auto completing_basics = legacy_star_completing_basics(*target);
    const auto needed_energy_it = std::find_if(
        completing_basics.begin(), completing_basics.end(),
        [this, grass_before, fire_before](const Card energy) {
          const int before = energy == Card::Grass ? grass_before : fire_before;
          return hand_count(energy) > before && count_of(state_.deck, energy) > 0;
        });
    if (needed_energy_it == completing_basics.end()) return false;
    const Card needed_energy = *needed_energy_it;
'''
if old not in text:
    raise SystemExit('issue-2315 typed deficit block missing')
text = text.replace(old, new, 1)
path.write_text(text, encoding='utf-8')

path = Path('src/trace_engine_v2/part_013_legacy_star_override.inc')
text = path.read_text(encoding='utf-8')
old = '''    const bool exact_reported_axis = state_.turn == 2 && active_is_vstar() &&
        state_.active->grass == 1 && state_.active->fire == 1 &&
        grass_needed() == 1 && fire_needed() == 0 &&
        count_of(state_.deck, Card::Grass) > 0;
'''
new = '''    const bool completing_basic_in_deck = state_.active && std::any_of(
        std::array<Card, 2>{Card::Grass, Card::Fire}.begin(),
        std::array<Card, 2>{Card::Grass, Card::Fire}.end(),
        [this](const Card energy) {
          if (count_of(state_.deck, energy) == 0) return false;
          Pokemon projected = *state_.active;
          return attach_energy_card(projected, energy) && pays_apex_energy_cost(projected);
        });
    const bool exact_reported_axis = state_.turn == 2 && active_is_vstar() &&
        apex_energy_progress(*state_.active) == 2 && completing_basic_in_deck;
'''
if old not in text:
    raise SystemExit('delayed Vessel typed axis missing')
text = text.replace(old, new, 1)
text = text.replace(
'''    const bool completing_basic_in_deck = state_.active && std::any_of(
        std::array<Card, 2>{Card::Grass, Card::Fire}.begin(),
        std::array<Card, 2>{Card::Grass, Card::Fire}.end(),
        [this](const Card energy) {''',
'''    const std::array<Card, 2> delayed_route_basics{Card::Grass, Card::Fire};
    const bool completing_basic_in_deck = state_.active && std::any_of(
        delayed_route_basics.begin(), delayed_route_basics.end(),
        [this](const Card energy) {''')
text = text.replace(
'''    const bool exact_reported_axis = state_.turn == 2 && active_is_vstar() &&
        apex_energy_progress(*state_.active) == 2 && completing_basic_in_deck;

    // Legacy Star may return Earthen Vessel now''',
'''    // DDE-only and the canonical GF/GG states are all one physical Basic
    // attachment short when semantic Apex progress is two.
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Confirmed migration bug: https://github.com/FlareZ123/pokemon-sims/issues/2423
    const bool exact_reported_axis = state_.turn == 2 && active_is_vstar() &&
        apex_energy_progress(*state_.active) == 2 && completing_basic_in_deck;

    // Legacy Star may return Earthen Vessel now''')
path.write_text(text, encoding='utf-8')

Path('tests/issue_2423_legacy_star_dde_projection_tests.cpp').write_text(textwrap.dedent(r'''
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
  static bool basic_completes(const Engine& engine, const Pokemon& pokemon, const Card energy) {
    return engine.legacy_star_basic_completes_apex(pokemon, energy);
  }
  static bool delayed_vessel(const Engine& engine) {
    return engine.legacy_star_delayed_vessel_route();
  }
};
}  // namespace sim

namespace {
void expect(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}

sim::Pokemon vstar(int grass, int fire, int dde) {
  sim::Pokemon pokemon{sim::Card::RegidragoVstar, 1, grass, fire, sim::Tool::None};
  pokemon.double_dragon = dde;
  return pokemon;
}

struct Fixture {
  sim::Scenario scenario{"issue-2423", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2423};
  sim::Engine engine{scenario, recipe, rng};
};

void test_completing_basic_truth_table() {
  Fixture f;
  // DDE-only is completed by either Basic. https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Apex GGF cost: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Bug: https://github.com/FlareZ123/pokemon-sims/issues/2423
  expect(sim::EngineTestAccess::basic_completes(f.engine, vstar(0, 0, 1), sim::Card::Grass),
         "DDE-only + Grass should complete Apex");
  expect(sim::EngineTestAccess::basic_completes(f.engine, vstar(0, 0, 1), sim::Card::Fire),
         "DDE-only + Fire should complete Apex");
  expect(sim::EngineTestAccess::basic_completes(f.engine, vstar(1, 1, 0), sim::Card::Grass),
         "GF + Grass should complete Apex");
  expect(!sim::EngineTestAccess::basic_completes(f.engine, vstar(1, 1, 0), sim::Card::Fire),
         "GF + Fire should remain short");
  expect(sim::EngineTestAccess::basic_completes(f.engine, vstar(2, 0, 0), sim::Card::Fire),
         "GG + Fire should complete Apex");
  expect(!sim::EngineTestAccess::basic_completes(f.engine, vstar(2, 0, 0), sim::Card::Grass),
         "GG + Grass should remain short");
}

void test_delayed_vessel_accepts_dde_only() {
  Fixture f;
  sim::State state;
  state.turn = 2;
  state.active = vstar(0, 0, 1);
  state.manual_energy_used = true;
  state.hand = {sim::Card::MegaDragonite, sim::Card::EarthenVessel};
  state.deck = {sim::Card::Grass};
  sim::EngineTestAccess::set_state(f.engine, std::move(state));
  expect(sim::EngineTestAccess::delayed_vessel(f.engine),
         "Delayed Vessel route missed DDE-only plus a searchable completing Basic");
}

void test_delayed_vessel_preserves_basic_controls() {
  Fixture f;
  sim::State state;
  state.turn = 2;
  state.active = vstar(1, 1, 0);
  state.manual_energy_used = true;
  state.hand = {sim::Card::MegaDragonite, sim::Card::EarthenVessel};
  state.deck = {sim::Card::Grass};
  sim::EngineTestAccess::set_state(f.engine, std::move(state));
  expect(sim::EngineTestAccess::delayed_vessel(f.engine),
         "Delayed Vessel regressed canonical GF + Grass");
}
}  // namespace

int main() {
  try {
    test_completing_basic_truth_table();
    test_delayed_vessel_accepts_dde_only();
    test_delayed_vessel_preserves_basic_controls();
    std::cout << "Issue 2423 Legacy Star DDE projection tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
''').lstrip(), encoding='utf-8')
