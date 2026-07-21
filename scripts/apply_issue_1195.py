from __future__ import annotations

import os
import tempfile
from contextlib import contextmanager
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "trace_engine_v2" / "part_010_steven_crispin_override.inc"
TEST = ROOT / "tests" / "issue_1022_wonder_tag_steven_fss_tests.cpp"


@contextmanager
def exclusive_lock(path: Path):
    descriptor = os.open(path, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(descriptor, str(os.getpid()).encode("ascii"))
        yield
    finally:
        os.close(descriptor)
        path.unlink(missing_ok=True)


def atomic_write(path: Path, text: str) -> None:
    with tempfile.NamedTemporaryFile(
        mode="w", encoding="utf-8", newline="\n", dir=path.parent, delete=False
    ) as handle:
        handle.write(text)
        temporary = Path(handle.name)
    os.replace(temporary, path)


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected one match, found {count}")
    return text.replace(old, new, 1)


def patch_source(text: str) -> str:
    text = replace_once(
        text,
        """  bool issue_1022_banked_route_{false};

  bool wonder_tag_can_bank_steven_for_known_t4_fss_route() const {
    if (state_.turn != 1 || !scenario_.going_first ||
        scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || scenario_.max_turn < 4 ||
""",
        """  bool issue_1022_banked_route_{false};

  bool issue_1022_items_locked_by_turn_four() const {
    // The route needs Forest Seal Stone and Mysterious Treasure on T4, so every
    // full or scheduled Item lock blocks the projection even when Items are legal on T1:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Core Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1195
    return scenario_.locks == LockMode::FullItem ||
        scenario_.locks == LockMode::FullCombined ||
        scenario_.locks == LockMode::TurnTwoItem;
  }

  bool wonder_tag_can_bank_steven_for_known_t4_fss_route() const {
    // Field Blower removes Path to the Peak, so Wonder Tag is live once the active
    // lock state is cleared. Star Alchemy remains usable through Path itself:
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Forest Seal Stone ruling: https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1195
    if (state_.turn != 1 || !scenario_.going_first ||
        scenario_.dci != DciProfile::StrictJit || rule_box_abilities_locked() ||
        issue_1022_items_locked_by_turn_four() || scenario_.max_turn < 4 ||
""",
        "source route gate",
    )
    return replace_once(
        text,
        """  bool banked_steven_has_known_t4_fss_route() const {
    return issue_1022_banked_route_ && state_.turn == 2 &&
        scenario_.going_first && scenario_.dci == DciProfile::StrictJit &&
        scenario_.locks == LockMode::None && scenario_.max_turn >= 4 &&
""",
        """  bool banked_steven_has_known_t4_fss_route() const {
    // Wonder Tag has already resolved. The remaining route is legal under a
    // removed Path, while future Item lock still blocks its T4 Tool and search Items:
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Forest Seal Stone ruling: https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1195
    return issue_1022_banked_route_ && state_.turn == 2 &&
        scenario_.going_first && scenario_.dci == DciProfile::StrictJit &&
        !issue_1022_items_locked_by_turn_four() && scenario_.max_turn >= 4 &&
""",
        "banked route gate",
    )


def patch_test(text: str) -> str:
    text = replace_once(
        text,
        """  static bool t4_route(const Engine& engine) {
    return engine.wonder_tag_can_bank_steven_for_known_t4_fss_route();
  }
""",
        """  static bool t4_route(const Engine& engine) {
    return engine.wonder_tag_can_bank_steven_for_known_t4_fss_route();
  }
  static void mark_t4_route_banked(Engine& engine) {
    engine.issue_1022_banked_route_ = true;
  }
  static bool banked_t4_route(const Engine& engine) {
    return engine.banked_steven_has_known_t4_fss_route();
  }
""",
        "test access",
    )
    text = replace_once(
        text,
        """void test_rule_box_lock_rejects_star_alchemy_route() {
  const sim::Scenario scenario{"issue-1022-rulebox-lock", sim::DciProfile::StrictJit,
                               sim::LockMode::FullRuleBoxAbility, true, 5};
  std::mt19937_64 rng{1024};
  sim::Engine engine = make_engine(scenario, rng);
  sim::EngineTestAccess::set_state(engine, route_state());

  // Forest Seal Stone grants a VSTAR Power Ability to a Pokémon V, so the modeled
  // Rule Box Ability lock invalidates this complete route:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/1022
  expect(!sim::EngineTestAccess::t4_route(engine),
         "Rule Box Ability lock must reject the Star Alchemy route.");
}
""",
        """void test_active_path_rejects_wonder_tag_projection() {
  const sim::Scenario scenario{"issue-1022-rulebox-lock", sim::DciProfile::StrictJit,
                               sim::LockMode::FullRuleBoxAbility, true, 5};
  std::mt19937_64 rng{1024};
  sim::Engine engine = make_engine(scenario, rng);
  sim::EngineTestAccess::set_state(engine, route_state());

  // Active Path suppresses Tapu Lele-GX's Wonder Tag, which blocks the projection.
  // Forest Seal Stone remains usable through Path because Star Alchemy is an Ability
  // the Pokémon V can use rather than an Ability that Pokémon has:
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // TPCi Rules Team ruling: https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1195
  expect(!sim::EngineTestAccess::t4_route(engine),
         "Active Path must reject a route that starts with Wonder Tag.");
}

void test_removed_path_admits_projection_and_banked_continuation() {
  const sim::Scenario scenario{"issue-1195-path-removed", sim::DciProfile::StrictJit,
                               sim::LockMode::FullRuleBoxAbility, true, 5};
  std::mt19937_64 rng{1195};
  sim::Engine engine = make_engine(scenario, rng);
  sim::State state = route_state();
  state.path_lock_removed = true;
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Field Blower discards Path, restoring Wonder Tag. The remaining T4 Forest Seal
  // Stone route is legal because Star Alchemy is usable even through Path itself:
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Forest Seal Stone ruling: https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1195
  expect(sim::EngineTestAccess::t4_route(engine),
         "A removed Path must admit the Wonder Tag Steven-FSS projection.");

  sim::State banked;
  banked.turn = 2;
  banked.path_lock_removed = true;
  banked.active = sim::Pokemon{sim::Card::Oricorio, 1};
  banked.active->grass = 1;
  banked.bench = {sim::Pokemon{sim::Card::TapuLeleGX, 1}};
  banked.hand = {sim::Card::StevensResolve, sim::Card::Arven,
                 sim::Card::BrilliantBlender, sim::Card::MegaDragonite};
  banked.deck = {sim::Card::RegidragoV, sim::Card::Crispin, sim::Card::Grass,
                 sim::Card::Fire, sim::Card::ForestSealStone,
                 sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(engine, std::move(banked));
  sim::EngineTestAccess::mark_t4_route_banked(engine);
  expect(sim::EngineTestAccess::banked_t4_route(engine),
         "A removed Path must preserve the already banked Steven continuation.");
}

void test_future_item_locks_reject_projection() {
  for (const sim::LockMode lock : {sim::LockMode::TurnTwoItem,
                                   sim::LockMode::FullItem,
                                   sim::LockMode::FullCombined}) {
    const sim::Scenario scenario{"issue-1195-item-lock", sim::DciProfile::StrictJit,
                                 lock, true, 5};
    std::mt19937_64 rng{1196 + static_cast<unsigned>(lock)};
    sim::Engine engine = make_engine(scenario, rng);
    sim::State state = route_state();
    state.path_lock_removed = true;
    sim::EngineTestAccess::set_state(engine, std::move(state));

    // The projection needs Forest Seal Stone and Mysterious Treasure on T4, so both
    // scheduled and full Item locks remain hard blockers:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Core Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1195
    expect(!sim::EngineTestAccess::t4_route(engine),
           "Future Item lock must reject the T4 Item-dependent projection.");
  }
}
""",
        "lock tests",
    )
    return replace_once(
        text,
        """  test_rule_box_lock_rejects_star_alchemy_route();
""",
        """  test_active_path_rejects_wonder_tag_projection();
  test_removed_path_admits_projection_and_banked_continuation();
  test_future_item_locks_reject_projection();
""",
        "test registry",
    )


def main() -> int:
    with exclusive_lock(ROOT / ".issue-1195-materializer.lock"):
        source = patch_source(SOURCE.read_text(encoding="utf-8"))
        test = patch_test(TEST.read_text(encoding="utf-8"))
        atomic_write(SOURCE, source)
        atomic_write(TEST, test)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
