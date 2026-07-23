from __future__ import annotations

import os
import re
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator, TextIO

FSS_PATH = Path("src/trace_engine_v2/part_010_fss_override.inc")
QB_PATH = Path("src/trace_engine_v2/part_009b1.inc")
TEST_PATH = Path("tests/issue_1185_quick_ball_strict_surplus_energy_tests.cpp")
LOCK_PATH = Path(".issue-1403.lock")


@contextmanager
def locked(path: Path) -> Iterator[TextIO]:
    handle = path.open("a+", encoding="utf-8")
    try:
        if os.name == "nt":
            import msvcrt
            handle.seek(0)
            msvcrt.locking(handle.fileno(), msvcrt.LK_LOCK, 1)
        else:
            import fcntl
            fcntl.flock(handle.fileno(), fcntl.LOCK_EX)
        yield handle
    finally:
        if os.name == "nt":
            import msvcrt
            handle.seek(0)
            msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)
        else:
            import fcntl
            fcntl.flock(handle.fileno(), fcntl.LOCK_UN)
        handle.close()


def atomic_write(path: Path, content: str) -> None:
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(content, encoding="utf-8")
    os.replace(temporary, path)


def replace_once(text: str, old: str, new: str, label: str) -> str:
    if new in text:
        return text
    if text.count(old) != 1:
        raise RuntimeError(f"Unexpected {label} anchor count: {text.count(old)}")
    return text.replace(old, new, 1)


def patch_fss(text: str) -> str:
    helper = '''  bool fss_should_take_grass_for_seed23_latias_burnet_route() const {
    if (!deck_seen_ || scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || state_.turn != 3 ||
        scenario_.max_turn < 4 || state_.vstar_power_used ||
        !supporter_allowed() || state_.manual_energy_used ||
        state_.retreat_used || item_locked() || !state_.active ||
        !is_basic(state_.active->card) || bench_space() == 0 ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
        deck_count_after_search_started(Card::LatiasEx) == 0 ||
        hand_count(Card::Crispin) == 0 || hand_count(Card::QuickBall) == 0 ||
        hand_count(Card::TateLiza) == 0 ||
        hand_count(Card::ProfessorBurnet) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::Grass) < 2 ||
        deck_count_after_search_started(Card::Fire) == 0 ||
        !std::any_of(state_.deck.begin(), state_.deck.end(), is_payload)) {
      return false;
    }

    const Pokemon* regi = target_regi();
    // Star Alchemy takes one Grass while held Crispin takes a second Grass and Fire.
    // The manual attachment supplies the other Grass, Quick Ball searches Latias ex,
    // and Latias replaces Tate & Liza's switch role. Fire, evolution, and Professor
    // Burnet then complete the legal strict-JIT route on T4:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // DCI and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1403
    return regi != nullptr && regi->card == Card::RegidragoV &&
        regi->entered_turn == state_.turn && regi->grass == 0 && regi->fire == 0 &&
        need_energy() && need_active_vstar() && grass_needed() == 2 &&
        fire_needed() == 1;
  }

'''
    if "fss_should_take_grass_for_seed23_latias_burnet_route" not in text:
        text = replace_once(text, "  Card fss_target_after_search_started() const {\n",
                            helper + "  Card fss_target_after_search_started() const {\n",
                            "FSS helper")
    return replace_once(
        text,
        "    if (fss_should_take_crispin_over_redundant_blender()) return Card::Crispin;\n\n"
        "    const Card original = fss_target_after_search_started_original();",
        "    if (fss_should_take_grass_for_seed23_latias_burnet_route()) return Card::Grass;\n"
        "    if (fss_should_take_crispin_over_redundant_blender()) return Card::Crispin;\n\n"
        "    const Card original = fss_target_after_search_started_original();",
        "FSS route admission")


def patch_qb(text: str) -> str:
    helper = '''  std::optional<Card> quick_ball_latias_replaced_tate_cost() const {
    if (!deck_seen_ || scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || state_.turn != 3 ||
        scenario_.max_turn < 4 || !state_.vstar_power_used ||
        !state_.supporter_used || !state_.manual_energy_used ||
        state_.retreat_used || item_locked() || !state_.active ||
        !is_basic(state_.active->card) || bench_space() == 0 ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
        deck_count_after_search_started(Card::LatiasEx) == 0 ||
        hand_count(Card::TateLiza) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::ProfessorBurnet) == 0 ||
        std::find(state_.discard.begin(), state_.discard.end(), Card::Crispin) ==
            state_.discard.end() ||
        !std::any_of(state_.deck.begin(), state_.deck.end(), is_payload)) {
      return std::nullopt;
    }

    const Pokemon* regi = target_regi();
    if (regi == nullptr || regi->card != Card::RegidragoV ||
        regi->entered_turn != state_.turn || !need_active_vstar()) {
      return std::nullopt;
    }
    const bool fire_finishes = regi->grass >= 2 && regi->fire == 0 &&
        hand_count(Card::Fire) > 0 && grass_needed() == 0 && fire_needed() == 1;
    const bool grass_finishes = regi->grass >= 1 && regi->fire >= 1 &&
        hand_count(Card::Grass) > 0 && grass_needed() == 1 && fire_needed() == 0;

    // Latias ex immediately replaces Tate & Liza's remaining switch role. The held
    // Basic Energy, VSTAR, and Professor Burnet then complete Energy, evolution, and
    // the current-turn Dragon payload on T4:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Route-conditioned DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1403
    return fire_finishes || grass_finishes
        ? std::optional<Card>{Card::TateLiza}
        : std::nullopt;
  }

'''
    if "quick_ball_latias_replaced_tate_cost" not in text:
        text = replace_once(text,
                            "  std::optional<Card> quick_ball_final_surplus_energy_cost() const {\n",
                            helper + "  std::optional<Card> quick_ball_final_surplus_energy_cost() const {\n",
                            "Quick Ball helper")
    return replace_once(
        text,
        "    if (!cost && want_latias) {\n"
        "      cost = quick_ball_final_surplus_energy_cost();\n"
        "    }\n"
        "    if (!cost && want_regi && hand_count(Card::RegidragoVstar) >= 2) {",
        "    if (!cost && want_latias) {\n"
        "      cost = quick_ball_final_surplus_energy_cost();\n"
        "    }\n"
        "    if (!cost && want_latias) {\n"
        "      cost = quick_ball_latias_replaced_tate_cost();\n"
        "    }\n"
        "    if (!cost && want_regi && hand_count(Card::RegidragoVstar) >= 2) {",
        "Quick Ball Tate fallback")


def patch_test(text: str) -> str:
    text = replace_once(
        text,
        "  static std::optional<Card> final_surplus_cost(const Engine& engine) {\n"
        "    return engine.quick_ball_final_surplus_energy_cost();\n"
        "  }",
        "  static std::optional<Card> final_surplus_cost(const Engine& engine) {\n"
        "    return engine.quick_ball_final_surplus_energy_cost();\n"
        "  }\n"
        "  static Card fss_target(const Engine& engine) {\n"
        "    return engine.fss_target_after_search_started();\n"
        "  }\n"
        "  static std::optional<Card> latias_tate_cost(const Engine& engine) {\n"
        "    return engine.quick_ball_latias_replaced_tate_cost();\n"
        "  }",
        "test accessors")

    states = '''
sim::State seed23_fss_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1};
  state.bench = {sim::Pokemon{sim::Card::TapuLeleGX, 2},
                 sim::Pokemon{sim::Card::RegidragoV, 3, 0, 0,
                              sim::Tool::ForestSealStone}};
  state.hand = {sim::Card::TateLiza, sim::Card::QuickBall, sim::Card::Crispin,
                sim::Card::RegidragoVstar, sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Fire,
                sim::Card::LatiasEx, sim::Card::MegaDragonite,
                sim::Card::Dragapult, sim::Card::RegidragoV};
  state.discard = {sim::Card::StevensResolve};
  return state;
}

sim::State seed23_quick_ball_state() {
  sim::State state = seed23_fss_state();
  state.vstar_power_used = true;
  state.supporter_used = true;
  state.manual_energy_used = true;
  state.bench.back().grass = 2;
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::Crispin));
  state.hand.push_back(sim::Card::Fire);
  state.discard.push_back(sim::Card::Crispin);
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(), sim::Card::Grass));
  return state;
}

'''
    if "seed23_fss_state" not in text:
        text = replace_once(text,
                            "void test_strict_jit_route_and_controls() {\n",
                            states + "void test_strict_jit_route_and_controls() {\n",
                            "seed-23 states")

    controls = '''
void test_seed23_route_admission_controls() {
  const sim::Scenario strict{"issue-1403", sim::DciProfile::StrictJit,
                             sim::LockMode::None, true, 5};
  std::mt19937_64 rng{140300};
  sim::Engine fss = make_engine(strict, rng, seed23_fss_state());

  // Direct Grass and route-replaced Tate & Liza are admitted only while every
  // observable Energy, evolution, Active-position, and Burnet payload axis survives:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1403
  expect(sim::EngineTestAccess::fss_target(fss) == sim::Card::Grass,
         "The complete K1 route must search Grass.");

  sim::State one_grass = seed23_fss_state();
  one_grass.deck.erase(std::find(one_grass.deck.begin(), one_grass.deck.end(),
                                 sim::Card::Grass));
  std::mt19937_64 one_grass_rng{140301};
  sim::Engine blocked_fss = make_engine(strict, one_grass_rng, std::move(one_grass));
  expect(sim::EngineTestAccess::fss_target(blocked_fss) != sim::Card::Grass,
         "Crispin must retain a second searchable Grass.");

  sim::State no_latias = seed23_fss_state();
  no_latias.deck.erase(std::find(no_latias.deck.begin(), no_latias.deck.end(),
                                 sim::Card::LatiasEx));
  std::mt19937_64 no_latias_rng{140302};
  sim::Engine blocked_latias = make_engine(strict, no_latias_rng, std::move(no_latias));
  expect(sim::EngineTestAccess::fss_target(blocked_latias) != sim::Card::Grass,
         "The route must require searchable Latias ex.");

  std::mt19937_64 qb_rng{140303};
  sim::Engine qb = make_engine(strict, qb_rng, seed23_quick_ball_state());
  expect(sim::EngineTestAccess::latias_tate_cost(qb) == sim::Card::TateLiza,
         "Latias must replace Tate & Liza's switch role.");

  sim::State no_burnet = seed23_quick_ball_state();
  no_burnet.hand.erase(std::find(no_burnet.hand.begin(), no_burnet.hand.end(),
                                 sim::Card::ProfessorBurnet));
  std::mt19937_64 no_burnet_rng{140304};
  sim::Engine blocked_burnet = make_engine(strict, no_burnet_rng, std::move(no_burnet));
  expect(!sim::EngineTestAccess::latias_tate_cost(blocked_burnet),
         "Tate & Liza stays protected without Burnet.");

  const sim::Scenario rulebox{"issue-1403-rulebox", sim::DciProfile::StrictJit,
                              sim::LockMode::FullRuleBoxAbility, true, 5};
  std::mt19937_64 lock_rng{140305};
  sim::Engine blocked_lock = make_engine(rulebox, lock_rng, seed23_quick_ball_state());
  expect(!sim::EngineTestAccess::latias_tate_cost(blocked_lock),
         "Rule Box Ability lock must block the Latias route.");
}

'''
    if "test_seed23_route_admission_controls" not in text:
        text = replace_once(text,
                            "void test_seed_23_records_t5_recovery() {\n",
                            controls + "void test_seed_23_records_t5_recovery() {\n",
                            "seed-23 controls")

    pattern = re.compile(r"void test_seed_23_records_t5_recovery\(\) \{.*?\n\}\n\}  // namespace",
                         re.DOTALL)
    replacement = '''void test_seed_23_records_t4_route() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  if (!scenario) throw std::runtime_error("Missing strict-jit/go-first scenario");
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{23};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Exact source-bound route and policy evidence:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1403
  expect(outcome.first_ready_turn == 4, "Seed 23 must reach T4 readiness.");
  expect(!outcome.setup_failed, "T4 readiness must count as setup success.");
  expect(trace_contains(trace, "T3 | STAR ALCHEMY", "Grass Energy"),
         "Star Alchemy must search Grass.");
  expect(trace_contains(trace, "T3 | DISCARD", "Tate & Liza (Quick Ball cost)"),
         "Quick Ball must spend route-replaced Tate & Liza.");
  expect(trace_contains(trace, "T3 | PLAY ITEM", "Latias ex"),
         "Quick Ball must search Latias ex.");
  expect(trace_contains(trace, "T3 | RETREAT", "R-LATIAS-01"),
         "Skyliner must promote the Regidrago line on T3.");
  expect(trace_contains(trace, "T4 | PLAY SUPPORTER", "R-BURNET-01"),
         "Burnet must establish the T4 payload.");
  expect(trace_contains(trace, "T4 | READY"), "Seed 23 must emit T4 readiness.");
  expect(!trace_contains(trace, "T4 | PLAY SUPPORTER", "R-CRISPIN-01"),
         "Duplicate Crispin must not consume the T4 Supporter action.");
}
}  // namespace'''
    if "test_seed_23_records_t4_route" not in text:
        text, count = pattern.subn(replacement, text, count=1)
        if count != 1:
            raise RuntimeError(f"Unexpected seed-23 function count: {count}")
    text = text.replace("    test_seed_23_records_t5_recovery();",
                        "    test_seed23_route_admission_controls();\n"
                        "    test_seed_23_records_t4_route();", 1)
    return text


def main() -> int:
    with locked(LOCK_PATH):
        atomic_write(FSS_PATH, patch_fss(FSS_PATH.read_text(encoding="utf-8")))
        atomic_write(QB_PATH, patch_qb(QB_PATH.read_text(encoding="utf-8")))
        atomic_write(TEST_PATH, patch_test(TEST_PATH.read_text(encoding="utf-8")))
    LOCK_PATH.unlink(missing_ok=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
