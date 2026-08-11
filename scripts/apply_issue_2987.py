from __future__ import annotations

import os
import tempfile
from pathlib import Path


def replace_exact(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if old not in text:
        raise RuntimeError(f"Expected source block not found in {path}")
    updated = text.replace(old, new, 1)
    if updated == text:
        raise RuntimeError(f"No change produced in {path}")

    fd, temporary_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="") as temporary:
            temporary.write(updated)
            temporary.flush()
            os.fsync(temporary.fileno())
        os.replace(temporary_name, path)
    finally:
        if os.path.exists(temporary_name):
            os.unlink(temporary_name)


source = Path("src/trace_engine_v2/part_tate_blender_attachment_override.inc")
replace_exact(
    source,
    """  return !secret_box_combo_enabled() &&\n      scenario_.dci == DciProfile::StrictJit &&\n      scenario_.locks == LockMode::FullCombined && scenario_.going_first &&\n      state_.turn == 1 && scenario_.max_turn >= 5 &&\n      !state_.manual_energy_used && !state_.retreat_used &&\n""",
    """  // The proactive bank is governed by the visible resource state below, not\n  // by DCI profile, lock schedule, player order, turn number, or horizon. Those\n  // scenario coordinates do not change Tapu's printed one-Energy Retreat Cost\n  // or Regidrago VSTAR's two-Grass attack reserve:\n  // https://api.pokemontcg.io/v2/cards/sm2-60\n  // https://api.pokemontcg.io/v2/cards/swsh12-136\n  // https://github.com/FlareZ123/pokemon-sims/issues/1845#issuecomment-5123772411\n  // https://github.com/FlareZ123/pokemon-sims/issues/2987\n  return !secret_box_combo_enabled() &&\n      !state_.manual_energy_used && !state_.retreat_used &&\n""",
)

tests = Path("tests/issue_1845_proactive_tapu_retreat_tests.cpp")
replace_exact(
    tests,
    """sim::Scenario scenario() {\n  // FullCombined now means Rule Box Ability suppression from the start plus Item\n  // lock beginning on turn 2. These exact-state tests exercise the proactive\n  // attachment/retreat policy independent of the retired historical seed trace:\n  // https://assets.pokemon.com/assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf\n  // https://github.com/FlareZ123/pokemon-sims/issues/2247\n  return sim::Scenario{\"issue-1845-proactive-tapu-retreat\",\n                       sim::DciProfile::StrictJit,\n                       sim::LockMode::FullCombined, true, 5};\n}\n""",
    """sim::Scenario scenario(\n    const sim::DciProfile dci = sim::DciProfile::StrictJit,\n    const sim::LockMode locks = sim::LockMode::FullCombined,\n    const bool going_first = true, const int max_turn = 5) {\n  // Lock/profile metadata remains useful for the surrounding simulation, while\n  // the proactive attachment itself is defined by observable resources:\n  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-model\n  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment\n  // https://github.com/FlareZ123/pokemon-sims/issues/2987\n  return sim::Scenario{\"issue-1845-proactive-tapu-retreat\", dci, locks,\n                       going_first, max_turn};\n}\n""",
)
replace_exact(
    tests,
    """  Fixture()\n      : scenario_value(scenario()),\n        recipe(sim::deck_by_id(\"regidrago-shell\")->recipe),\n        rng(1845),\n        engine(scenario_value, recipe, rng) {}\n""",
    """  explicit Fixture(sim::Scenario selected = scenario())\n      : scenario_value(std::move(selected)),\n        recipe(sim::deck_by_id(\"regidrago-shell\")->recipe),\n        rng(1845),\n        engine(scenario_value, recipe, rng) {}\n""",
)
replace_exact(
    tests,
    """void only_two_grass_is_rejected() {\n""",
    """void scenario_coordinates_do_not_gate_public_surplus() {\n  const auto verify = [](sim::Scenario selected, const int turn,\n                         const char* message) {\n    Fixture fixture(std::move(selected));\n    sim::State state = opening_state();\n    state.turn = turn;\n    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));\n    expect(sim::EngineTestAccess::proactive_tapu_attachment(fixture.engine),\n           message);\n  };\n\n  // Tapu's one-Colorless Retreat Cost and Regidrago's GGF attack requirement\n  // are unchanged by these scenario coordinates. The visible third Grass stays\n  // surplus after reserving the two Grass required by Apex Dragon:\n  // https://api.pokemontcg.io/v2/cards/sm2-60\n  // https://api.pokemontcg.io/v2/cards/swsh12-136\n  // https://github.com/FlareZ123/pokemon-sims/issues/1845#issuecomment-5123772411\n  // https://github.com/FlareZ123/pokemon-sims/issues/2987\n  verify(scenario(sim::DciProfile::MatchupFlexJit), 1,\n         \"Matchup-flex JIT incorrectly gated the public surplus attachment\");\n  verify(scenario(sim::DciProfile::NoDiscardControl), 1,\n         \"No-discard-control incorrectly gated the public surplus attachment\");\n  verify(scenario(sim::DciProfile::StrictJit, sim::LockMode::TurnTwoItem), 1,\n         \"Lock schedule incorrectly gated the public surplus attachment\");\n  verify(scenario(sim::DciProfile::StrictJit, sim::LockMode::FullCombined, false),\n         1, \"Going second incorrectly gated the public surplus attachment\");\n  verify(scenario(), 3,\n         \"Later turn incorrectly gated the same public surplus attachment\");\n  verify(scenario(sim::DciProfile::StrictJit, sim::LockMode::FullCombined, true,\n                  3),\n         1, \"Shorter horizon incorrectly gated the public surplus attachment\");\n}\n\nvoid only_two_grass_is_rejected() {\n""",
)
replace_exact(
    tests,
    """    public_surplus_attachment_is_admitted();\n    only_two_grass_is_rejected();\n""",
    """    public_surplus_attachment_is_admitted();\n    scenario_coordinates_do_not_gate_public_surplus();\n    only_two_grass_is_rejected();\n""",
)
