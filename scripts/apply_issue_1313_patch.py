from __future__ import annotations

import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def atomic_write(path: Path, text: str) -> None:
    descriptor, temporary_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as handle:
            handle.write(text)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temporary_name, path)
    except BaseException:
        try:
            os.unlink(temporary_name)
        except FileNotFoundError:
            pass
        raise


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"Expected one source anchor in {path}, found {count}")
    atomic_write(path, text.replace(old, new, 1))


def main() -> None:
    source = ROOT / "src/trace_engine_v2/part_empty_deck_search_override.inc"
    replace_once(
        source,
        """    std::optional<Card> planned_payload;
    for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                               Card::GoodraVstar, Card::DialgaGX}) {
""",
        """    std::optional<Card> planned_payload;
    // Appletun is a modeled Pokémon payload and must participate in the exact
    // first-search projection just like every other Apex Dragon payload:
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/1313
    for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                               Card::GoodraVstar, Card::DialgaGX,
                               Card::Appletun}) {
""",
    )

    test = ROOT / "tests/issue_1313_appletun_second_ultra_tests.cpp"
    atomic_write(
        test,
        r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool second_ultra_target_survives(Engine& engine) {
    return engine.second_ultra_ball_has_post_payload_target();
  }
};
}  // namespace sim

namespace {

void test_appletun_first_search_leaves_regidrago_v_target() {
  using namespace sim;
  const Scenario scenario{"issue-1313/appletun-second-ultra", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng{1313};
  Engine engine{scenario, recipe, rng};
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::UltraBall, Card::UltraBall, Card::RegidragoVstar,
                Card::RegidragoVstar, Card::RegidragoVstar};
  state.deck = {Card::Appletun, Card::RegidragoV};
  EngineTestAccess::set_deck_seen(engine);

  // Ultra Ball may search Appletun. Removing that exact first target leaves
  // Regidrago V as a legal Pokémon target for the promised second Ultra Ball:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://github.com/FlareZ123/pokemon-sims/issues/1313
  if (!EngineTestAccess::second_ultra_target_survives(engine)) {
    throw std::runtime_error("Appletun projection rejected a surviving Regidrago V target.");
  }
}

void test_appletun_first_search_rejects_zero_remaining_targets() {
  using namespace sim;
  const Scenario scenario{"issue-1313/appletun-zero-target", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng{1314};
  Engine engine{scenario, recipe, rng};
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.deck = {Card::Appletun, Card::Grass};
  EngineTestAccess::set_deck_seen(engine);

  // A known second Ultra Ball route remains illegal when no Pokémon survives
  // the exact Appletun removal:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  if (EngineTestAccess::second_ultra_target_survives(engine)) {
    throw std::runtime_error("Appletun projection invented a second Pokémon target.");
  }
}

}  // namespace

int main() {
  test_appletun_first_search_leaves_regidrago_v_target();
  test_appletun_first_search_rejects_zero_remaining_targets();
  std::cout << "Issue 1313 Appletun second-Ultra tests passed.\n";
  return 0;
}
''',
    )


if __name__ == "__main__":
    main()
