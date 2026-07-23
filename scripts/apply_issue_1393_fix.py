from __future__ import annotations

import os
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator, TextIO


SOURCE_PATH = Path("src/trace_engine_v2/part_014b.inc")
TEST_PATH = Path("tests/issue_1393_crispin_before_gladion_tests.cpp")
LOCK_PATH = SOURCE_PATH.with_suffix(SOURCE_PATH.suffix + ".issue1393.lock")


@contextmanager
def locked_file(path: Path) -> Iterator[TextIO]:
    path.parent.mkdir(parents=True, exist_ok=True)
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
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(content, encoding="utf-8", newline="\n")
    os.replace(temporary, path)


def replace_once(text: str, old: str, new: str, description: str) -> str:
    if new in text:
        return text
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"Unexpected {description} anchor count: {count}")
    return text.replace(old, new, 1)


PRODUCTION_BLOCK = r'''  bool held_crispin_completes_current_turn_before_known_prize_gladion() const {
    if (!prizes_known() || hand_count(Card::Gladion) == 0 ||
        hand_count(Card::Crispin) == 0 || !supporter_allowed() ||
        state_.manual_energy_used || item_locked() ||
        !strict_payload_timing() || !need_energy() || !need_payload() ||
        !state_.active ||
        (state_.active->card != Card::RegidragoV &&
         state_.active->card != Card::RegidragoVstar)) {
      return false;
    }

    const bool active_vstar_now = state_.active->card == Card::RegidragoVstar;
    const bool active_v_can_evolve_now =
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        hand_count(Card::RegidragoVstar) > 0;
    if (!active_vstar_now && !active_v_can_evolve_now) return false;

    // Crispin attaches one Basic Energy and puts the other into hand. The unused
    // manual attachment can then complete GGF. Prefer that deterministic current-
    // turn route before Gladion only when the exact paid projection also establishes
    // a same-turn strict-JIT Dragon payload through a legal Item outlet:
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Supporter, attachment, evolution, and Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Strict-JIT and DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1393
    std::mt19937_64 shadow_rng = rng_;
    Engine projected(scenario_, recipe_, shadow_rng);
    projected.state_ = state_;
    projected.deck_seen_ = deck_seen_;
    projected.prizes_revealed_ = prizes_revealed_;

    if (!projected.play_crispin()) return false;
    projected.attach_manual();
    projected.evolve_best_regi();
    projected.play_items_until_stable(true);
    projected.evolve_best_regi();

    return projected.active_is_vstar() &&
           projected.state_.active->grass >= 2 &&
           projected.state_.active->fire >= 1 &&
           projected.payload_ready();
  }

'''

TEST_CONTENT = r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true,
                        const bool prizes_revealed = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }

  static bool crispin_preempts_gladion(const Engine& engine) {
    return engine.held_crispin_completes_current_turn_before_known_prize_gladion();
  }

  static void choose_supporter(Engine& engine) {
    engine.choose_supporter();
  }

  static bool attach_manual(Engine& engine) {
    return engine.attach_manual();
  }

  static bool evolve_best_regi(Engine& engine) {
    return engine.evolve_best_regi();
  }

  static void play_items_until_stable(Engine& engine,
                                      const bool permit_payload) {
    engine.play_items_until_stable(permit_payload);
  }

  static const State& state(const Engine& engine) {
    return engine.state_;
  }
};
}  // namespace sim

namespace {

sim::Scenario strict_second_scenario(const sim::LockMode locks = sim::LockMode::None) {
  return sim::Scenario{
      "issue-1393/exact",
      sim::DciProfile::StrictJit,
      locks,
      false,
      5,
  };
}

sim::State exact_state() {
  sim::State state;
  state.turn = 2;
  state.active =
      sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0, sim::Tool::None};
  state.hand = {
      sim::Card::RegidragoVstar,
      sim::Card::Crispin,
      sim::Card::Gladion,
      sim::Card::QuickBall,
      sim::Card::MegaDragonite,
      sim::Card::BrilliantBlender,
  };
  state.deck = {
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::RegidragoV,
      sim::Card::TapuLeleGX,
      sim::Card::MysteriousTreasure,
      sim::Card::Dragapult,
  };
  state.prizes = {
      sim::Card::Oricorio,
      sim::Card::ForestSealStone,
      sim::Card::Arven,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Guzma,
  };
  return state;
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(sim::Scenario scenario_value = strict_second_scenario())
      : scenario(std::move(scenario_value)),
        recipe(sim::baseline_recipe()),
        rng(1393),
        engine(scenario, recipe, rng) {}
};

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_held_crispin_completes_t2_before_known_prize_gladion() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, exact_state());

  // Crispin attaches the second Grass and places Fire in hand. The unused manual
  // attachment supplies Fire, while Quick Ball legally discards Mega Dragonite ex
  // and searches a Basic Pokémon. This reaches GGF plus the current-turn strict-JIT
  // payload without Gladion or Star Alchemy:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1393
  if (!sim::EngineTestAccess::crispin_preempts_gladion(fixture.engine)) {
    throw std::runtime_error(
        "The exact T2 Crispin route did not preempt known-Prize Gladion.");
  }

  sim::EngineTestAccess::choose_supporter(fixture.engine);
  sim::EngineTestAccess::attach_manual(fixture.engine);
  sim::EngineTestAccess::evolve_best_regi(fixture.engine);
  sim::EngineTestAccess::play_items_until_stable(fixture.engine, true);
  sim::EngineTestAccess::evolve_best_regi(fixture.engine);

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.supporter_used || contains(after.hand, sim::Card::Crispin) ||
      !contains(after.hand, sim::Card::Gladion) ||
      !contains(after.prizes, sim::Card::Oricorio) ||
      !after.active || after.active->card != sim::Card::RegidragoVstar ||
      after.active->grass < 2 || after.active->fire < 1 ||
      !contains(after.discarded_this_turn, sim::Card::MegaDragonite)) {
    throw std::runtime_error(
        "The corrected route failed to preserve Gladion and complete T2 readiness.");
  }
}

void test_missing_energy_type_preserves_gladion_priority() {
  Fixture fixture;
  sim::State state = exact_state();
  state.deck.erase(
      std::remove(state.deck.begin(), state.deck.end(), sim::Card::Fire),
      state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Crispin cannot attach one type and put the other into hand when Fire is absent.
  // Gladion therefore retains priority for the known Prize route:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://github.com/FlareZ123/pokemon-sims/issues/1393
  if (sim::EngineTestAccess::crispin_preempts_gladion(fixture.engine)) {
    throw std::runtime_error(
        "Crispin preempted Gladion without both searchable Energy types.");
  }
}

void test_spent_manual_attachment_preserves_gladion_priority() {
  Fixture fixture;
  sim::State state = exact_state();
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // The manual attachment is required to attach the Energy Crispin puts into hand:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1393
  if (sim::EngineTestAccess::crispin_preempts_gladion(fixture.engine)) {
    throw std::runtime_error(
        "Crispin preempted Gladion after the manual attachment was spent.");
  }
}

void test_item_lock_or_missing_payload_outlet_preserves_gladion_priority() {
  {
    Fixture fixture(strict_second_scenario(sim::LockMode::FullItem));
    sim::EngineTestAccess::set_state(fixture.engine, exact_state());

    // Item lock prevents Quick Ball and Brilliant Blender from establishing the
    // same-turn strict-JIT payload:
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://github.com/FlareZ123/pokemon-sims/issues/1393
    if (sim::EngineTestAccess::crispin_preempts_gladion(fixture.engine)) {
      throw std::runtime_error(
          "Crispin preempted Gladion through full Item lock.");
    }
  }

  {
    Fixture fixture;
    sim::State state = exact_state();
    state.hand.erase(
        std::remove(state.hand.begin(), state.hand.end(), sim::Card::QuickBall),
        state.hand.end());
    state.hand.erase(
        std::remove(state.hand.begin(), state.hand.end(),
                    sim::Card::BrilliantBlender),
        state.hand.end());
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

    // A Dragon in hand remains UDP under strict JIT until a legal same-turn discard
    // outlet exists. The helper must not treat an inaccessible payload as ready:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/issues/1393
    if (sim::EngineTestAccess::crispin_preempts_gladion(fixture.engine)) {
      throw std::runtime_error(
          "Crispin preempted Gladion without a payable payload outlet.");
    }
  }
}

void test_current_turn_regidrago_cannot_evolve() {
  Fixture fixture;
  sim::State state = exact_state();
  state.active->entered_turn = 2;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // A Basic Pokémon played during the current turn cannot evolve during that turn:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1393
  if (sim::EngineTestAccess::crispin_preempts_gladion(fixture.engine)) {
    throw std::runtime_error(
        "Crispin preempted Gladion through an illegal same-turn evolution.");
  }
}

void test_seed_61_holds_star_alchemy_and_reaches_t2() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  if (!scenario || deck == nullptr) {
    throw std::runtime_error("The issue-1393 seed fixture is unavailable.");
  }

  std::mt19937_64 rng(61);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();
  if (outcome.first_ready_turn != 2 || outcome.used_fss) {
    throw std::runtime_error(
        "Seed 61 failed to reach T2 while preserving Star Alchemy.");
  }

  const auto has_line = [&trace](const std::string& fragment) {
    return std::any_of(
        trace.lines.begin(), trace.lines.end(),
        [&fragment](const std::string& line) {
          return line.find(fragment) != std::string::npos;
        });
  };

  // The source-bound trace must hold Star Alchemy, play held Crispin, discard Mega
  // Dragonite ex with Quick Ball, preserve the known-Prize Gladion route, and reach
  // T2 readiness:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://github.com/FlareZ123/pokemon-sims/issues/1393
  if (!has_line("HOLD STAR ALCHEMY") ||
      !has_line("R-CRISPIN-01") ||
      !has_line("Mega Dragonite ex (Quick Ball cost)") ||
      has_line("Recovered known Prize: Oricorio") ||
      has_line("STAR ALCHEMY |") ||
      !has_line("T2 | READY")) {
    throw std::runtime_error(
        "Seed 61 lost the corrected Crispin-before-Gladion trace.");
  }
}

}  // namespace

int main() {
  test_held_crispin_completes_t2_before_known_prize_gladion();
  test_missing_energy_type_preserves_gladion_priority();
  test_spent_manual_attachment_preserves_gladion_priority();
  test_item_lock_or_missing_payload_outlet_preserves_gladion_priority();
  test_current_turn_regidrago_cannot_evolve();
  test_seed_61_holds_star_alchemy_and_reaches_t2();
}
'''


def main() -> int:
    with locked_file(LOCK_PATH):
        source = SOURCE_PATH.read_text(encoding="utf-8")
        anchor = "  void choose_supporter() {\n"
        source = replace_once(
            source,
            anchor,
            PRODUCTION_BLOCK + anchor,
            "held Crispin completion helper insertion",
        )
        source = replace_once(
            source,
            "    if (play_turo()) return;\n"
            "    if (prizes_known() && play_gladion()) return;\n",
            "    if (play_turo()) return;\n"
            "    if (held_crispin_completes_current_turn_before_known_prize_gladion() &&\n"
            "        play_crispin()) return;\n"
            "    if (prizes_known() && play_gladion()) return;\n",
            "Crispin-before-Gladion selector ordering",
        )
        atomic_write(SOURCE_PATH, source)
        atomic_write(TEST_PATH, TEST_CONTENT)
    try:
        LOCK_PATH.unlink()
    except FileNotFoundError:
        pass
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
