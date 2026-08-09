from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def atomic_write(path: Path, text: str) -> None:
    lock_path = path.with_name(f"{path.name}.lock")
    with lock_path.open("w", encoding="utf-8") as lock:
        fcntl.flock(lock.fileno(), fcntl.LOCK_EX)
        with tempfile.NamedTemporaryFile("w", encoding="utf-8", newline="\n", dir=path.parent, delete=False) as tmp:
            tmp.write(text)
            tmp.flush()
            os.fsync(tmp.fileno())
            tmp_name = tmp.name
        os.replace(tmp_name, path)
    lock_path.unlink(missing_ok=True)


def replace_once(text: str, old: str, new: str, label: str) -> str:
    if new in text:
        return text
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"#2408 {label}: expected one anchor, found {count}")
    return text.replace(old, new, 1)


source_path = ROOT / "src/trace_engine_v2/part_014b.inc"
source = source_path.read_text(encoding="utf-8")
anchor = '    // Exact Serena text: discard up to 3 cards, but "You must discard at least 1 card."\n'
insert = '''    // Once K1 proves a deck-resident Dragon, Professor Burnet and Serena can
    // both complete a payload-only strict-JIT state. Prefer Burnet only when its
    // existing ready-turn planner proves the same turn and no Item can discard the
    // held Dragon instead. This preserves Serena's draw/gust option and the held
    // Dragon while putting up to two searched payloads directly in discard.
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter and deck-search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, DCI, and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2408
    const bool burnet_preserves_serena_and_held_payload =
        scenario_.dci == DciProfile::StrictJit && prizes_known() &&
        need_payload() && !need_regi() && !need_vstar() && !need_energy() &&
        !need_active_vstar() && hand_count(Card::ProfessorBurnet) > 0 &&
        hand_count(Card::Serena) > 0 &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) &&
        professor_burnet_has_live_ready_turn_route() &&
        !has_live_blender_payload_line() &&
        !has_live_one_discard_hand_payload_line() &&
        !has_live_ultra_ball_hand_payload_line();
    if (burnet_preserves_serena_and_held_payload && play_professor_burnet()) return;

'''
source = replace_once(source, anchor, insert + anchor, "Supporter order")
atomic_write(source_path, source)

primary_path = ROOT / "tests/issue_1341_burnet_held_payload_tests.cpp"
primary = primary_path.read_text(encoding="utf-8")
primary = replace_once(
    primary,
    '''  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
''',
    '''  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
''',
    "primary knowledge setter",
)
old_primary_call = '''  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Serena can use its mandatory discard on the held Dragon and complete strict JIT.
  // Burnet must remain out of the discard pile while Serena uses the Supporter slot:
'''
new_primary_call = '''  sim::EngineTestAccess::set_state(engine, std::move(state), false);

  // K0 has not proved a deck-resident payload, so the public held-card Serena route
  // remains the deterministic strict-JIT completion rather than assuming Burnet hits.
  // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2408
'''
primary = replace_once(primary, old_primary_call, new_primary_call, "primary K0 control")
atomic_write(primary_path, primary)

shadow_path = ROOT / "tests/issue_2091_prize_k1_issue_1341_burnet_held_payload_tests.cpp"
shadow = shadow_path.read_text(encoding="utf-8")
shadow = replace_once(
    shadow,
    "void test_serena_uses_held_payload_instead_of_burnet() {\n",
    "void test_k1_burnet_preserves_serena_and_held_payload() {\n",
    "Prize-K1 function name",
)
old_comment = '''  // Serena can use its mandatory discard on the held Dragon and complete strict JIT.
  // Burnet must remain out of the discard pile while Serena uses the Supporter slot:
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/888
  // https://github.com/FlareZ123/pokemon-sims/issues/1341
'''
new_comment = '''  // Complete Prize inspection is K1. Burnet can use known deck payloads while
  // preserving Serena and the held Dragon for later discrete-value routes.
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Supporter/search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1/resource policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2408
'''
shadow = replace_once(shadow, old_comment, new_comment, "Prize-K1 comment")
old_expect = '''  expect(result.supporter_used, "Serena must use the Supporter play.");
  expect(contains(result.discard, sim::Card::Serena),
         "Serena must be the Supporter placed in discard.");
  expect(contains(result.discard, sim::Card::MegaDragonite),
         "Serena must discard the held Dragon payload.");
  expect(!contains(result.discard, sim::Card::ProfessorBurnet),
         "Professor Burnet must not be consumed before Serena.");
'''
new_expect = '''  expect(result.supporter_used, "Professor Burnet must use the Supporter play.");
  expect(contains(result.discard, sim::Card::ProfessorBurnet),
         "Prize-derived K1 must prefer Professor Burnet.");
  expect(contains(result.hand, sim::Card::Serena) &&
             !contains(result.discard, sim::Card::Serena),
         "Serena must remain held after the K1 Burnet route.");
  expect(contains(result.hand, sim::Card::MegaDragonite) &&
             !contains(result.discard, sim::Card::MegaDragonite),
         "The held Dragon must remain held after the K1 Burnet route.");
  expect(contains(result.discard, sim::Card::Dragapult) &&
             contains(result.discard, sim::Card::DialgaGX),
         "Burnet must discard the K1-known deck payloads.");
'''
shadow = replace_once(shadow, old_expect, new_expect, "Prize-K1 expectations")
shadow = replace_once(
    shadow,
    "    test_serena_uses_held_payload_instead_of_burnet();\n",
    "    test_k1_burnet_preserves_serena_and_held_payload();\n",
    "Prize-K1 invocation",
)
atomic_write(shadow_path, shadow)

new_test_path = ROOT / "tests/issue_2408_burnet_before_serena_tests.cpp"
new_test = r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen,
                        const bool prizes_revealed) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
};
}  // namespace sim

namespace {
bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State payload_only_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::Serena,
                sim::Card::MegaDragonite, sim::Card::Grass};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::RegidragoV,
                sim::Card::Dragapult, sim::Card::DialgaGX,
                sim::Card::Fire, sim::Card::Grass};
  return state;
}

void test_k1_prefers_burnet_for_equal_turn_payload_completion() {
  const sim::Scenario scenario{"issue-2408-k1", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{2408};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, payload_only_state(), true, true);

  // K1 proves the deck-resident Dragon payloads. Burnet and Serena both finish the
  // sole strict-JIT payload axis this turn, so the lower resource-cost route keeps
  // Serena's draw/gust mode and the held Dragon available for later turns.
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Supporter/search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1/resource policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2408
  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.discard, sim::Card::ProfessorBurnet),
         "K1 equal-turn payload completion failed to prefer Burnet.");
  expect(contains(result.hand, sim::Card::Serena),
         "K1 Burnet route failed to preserve Serena.");
  expect(contains(result.hand, sim::Card::MegaDragonite),
         "K1 Burnet route failed to preserve held Dragon payload.");
}

void test_k0_keeps_observable_serena_route() {
  const sim::Scenario scenario{"issue-2408-k0", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{2409};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, payload_only_state(), false, false);

  // K0 cannot infer that Burnet has a deck-resident Dragon target. Serena's held
  // Dragon route is observable and deterministic, so the K1 optimization must stay off.
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2408
  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.discard, sim::Card::Serena),
         "K0 incorrectly assumed a Burnet deck payload.");
  expect(contains(result.discard, sim::Card::MegaDragonite),
         "K0 Serena route failed to use the observable held payload.");
}
}  // namespace

int main() {
  try {
    test_k1_prefers_burnet_for_equal_turn_payload_completion();
    test_k0_keeps_observable_serena_route();
    std::cout << "Issue 2408 Burnet-before-Serena tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
'''
atomic_write(new_test_path, new_test)

cmake_path = ROOT / "CMakeLists.txt"
cmake = cmake_path.read_text(encoding="utf-8")
registration = r'''
# Exact source-bound trace for the confirmed K1 Burnet-over-Serena resource order.
# Rules/card/spec URLs are adjacent to production and state regression logic:
# https://github.com/FlareZ123/pokemon-sims/issues/2408
add_test(NAME trace_issue_2408_burnet_before_serena
  COMMAND regidrago_sim --simulate-this --deck regidrago-shell
          --scenario strict-jit/go-second --seed 19 --require-ready-by 2)
'''
if "trace_issue_2408_burnet_before_serena" not in cmake:
    cmake = cmake.rstrip() + "\n" + registration
atomic_write(cmake_path, cmake)
