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
    source = ROOT / "src/trace_engine_v2/part_prize_payload_outlet_override.inc"
    replace_once(
        source,
        """    const bool payload_still_in_deck = might_be_unseen(Card::MegaDragonite) ||
                                       might_be_unseen(Card::Dragapult) ||
                                       might_be_unseen(Card::GoodraVstar) ||
                                       might_be_unseen(Card::DialgaGX);
""",
        """    // Professor Burnet can discard Appletun from deck for Apex Dragon, so
    // Appletun keeps the same Burnet-preserving Oricorio priority live:
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/1315
    const bool payload_still_in_deck = might_be_unseen(Card::MegaDragonite) ||
                                       might_be_unseen(Card::Dragapult) ||
                                       might_be_unseen(Card::GoodraVstar) ||
                                       might_be_unseen(Card::DialgaGX) ||
                                       might_be_unseen(Card::Appletun);
""",
    )

    test = ROOT / "tests/issue_1315_heavy_ball_appletun_burnet_tests.cpp"
    atomic_write(
        test,
        r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static bool play_heavy_ball(Engine& engine) { return engine.play_heavy_ball(); }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State heavy_ball_state(const sim::Card payload) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::HisuianHeavyBall, sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::Fire, payload, sim::Card::Crispin, sim::Card::Grass};
  state.prizes = {sim::Card::TapuLeleGX, sim::Card::Oricorio};
  return state;
}

void test_appletun_preserves_burnet_by_recovering_oricorio() {
  using namespace sim;
  const Scenario scenario{"issue-1315/appletun-burnet", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng{1315};
  Engine engine{scenario, recipe, rng};
  EngineTestAccess::set_state(engine, heavy_ball_state(Card::Appletun));

  // Heavy Ball reveals the Prize cards. Recovering Oricorio supplies the missing
  // Basic Fire Energy through Vital Dance and preserves Professor Burnet to discard
  // Appletun for Apex Dragon during the same turn:
  // https://api.pokemontcg.io/v2/cards/swsh10-146
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1315
  if (!EngineTestAccess::play_heavy_ball(engine)) {
    throw std::runtime_error("Hisuian Heavy Ball did not resolve.");
  }

  const State& after = EngineTestAccess::state(engine);
  if (!contains(after.hand, Card::Oricorio) || contains(after.hand, Card::TapuLeleGX)) {
    throw std::runtime_error("Heavy Ball failed to recover Oricorio over Tapu Lele-GX.");
  }
}

void test_no_deck_payload_does_not_invent_the_burnet_route() {
  using namespace sim;
  const Scenario scenario{"issue-1315/no-deck-payload", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng{1316};
  Engine engine{scenario, recipe, rng};
  EngineTestAccess::set_state(engine, heavy_ball_state(Card::Grass));

  // The Oricorio-over-Tapu compression requires a live deck payload for Burnet.
  // Without one, the ordinary Tapu Lele-GX connector priority remains available:
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  if (!EngineTestAccess::play_heavy_ball(engine)) {
    throw std::runtime_error("Hisuian Heavy Ball did not resolve in the control state.");
  }

  const State& after = EngineTestAccess::state(engine);
  if (!contains(after.hand, Card::TapuLeleGX) || contains(after.hand, Card::Oricorio)) {
    throw std::runtime_error("The no-payload control incorrectly forced Oricorio.");
  }
}

}  // namespace

int main() {
  test_appletun_preserves_burnet_by_recovering_oricorio();
  test_no_deck_payload_does_not_invent_the_burnet_route();
  std::cout << "Issue 1315 Heavy Ball Appletun/Burnet tests passed.\n";
  return 0;
}
''',
    )


if __name__ == "__main__":
    main()
