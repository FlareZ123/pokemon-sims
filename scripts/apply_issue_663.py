from __future__ import annotations

import os
from pathlib import Path


class FileLock:
    def __init__(self, path: Path) -> None:
        self.path = path
        self.fd: int | None = None

    def __enter__(self) -> FileLock:
        self.fd = os.open(self.path, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
        return self

    def __exit__(self, exc_type: object, exc: object, traceback: object) -> None:
        if self.fd is not None:
            os.close(self.fd)
        self.path.unlink(missing_ok=True)


def atomic_write(path: Path, content: str) -> None:
    lock_path = path.with_name(path.name + ".issue-663.lock")
    temp_path = path.with_name(path.name + ".issue-663.tmp")
    with FileLock(lock_path):
        temp_path.write_text(content, encoding="utf-8")
        os.replace(temp_path, path)


def patch_source() -> None:
    source = Path("src/trace_engine_v2/part_005.inc")
    text = source.read_text(encoding="utf-8")
    marker = "no_lock_oricorio_dialga_are_only_opening_basics"
    if marker in text:
        return

    anchor = "    // Oricorio's Vital Dance triggers only when it is played from hand onto the\n"
    if anchor not in text:
        raise RuntimeError("opening selector anchor missing")

    insertion = '''    const bool no_lock_oricorio_dialga_are_only_opening_basics =
        hand_count(Card::Oricorio) > 0 && hand_count(Card::DialgaGX) > 0 &&
        hand_count(Card::RegidragoV) == 0 && hand_count(Card::LatiasEx) == 0 &&
        hand_count(Card::MawileGX) == 0 && hand_count(Card::TapuLeleGX) == 0;
    const bool no_lock_dialga_is_redundant_payload =
        hand_count(Card::DialgaGX) == 1 && payloads_in_hand > 1;
    const bool no_lock_vital_dance_has_energy_value = !both_manual_energy_types_held;
    const bool no_lock_public_regidrago_v_route =
        hand_count(Card::RegidragoVstar) > 0 &&
        (hand_count(Card::QuickBall) > 0 ||
         hand_count(Card::MysteriousTreasure) > 0 ||
         hand_count(Card::Arven) > 0 ||
         hand_count(Card::StevensResolve) > 0);

    // Setup may start a redundant Dialga-GX so Oricorio remains in hand and can
    // later be played onto the Bench for Vital Dance. Apply the no-lock route only
    // under strict JIT, while another held Dragon covers payload timing, at least
    // one GGF Energy type is missing, Regidrago VSTAR is held, and the public hand
    // exposes Regidrago V through a direct one-discard Item, Arven, or Steven's Resolve:
    // Setup procedure: https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
    // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Regression evidence: https://github.com/FlareZ123/pokemon-sims/issues/663
    if (no_lock_oricorio_dialga_are_only_opening_basics &&
        scenario_.locks == LockMode::None && scenario_.dci == DciProfile::StrictJit &&
        no_lock_dialga_is_redundant_payload && no_lock_vital_dance_has_energy_value &&
        no_lock_public_regidrago_v_route && remove_one(state_.hand, Card::DialgaGX)) {
      state_.active = Pokemon{Card::DialgaGX, 0};
      outcome_.started_regi = false;
      return;
    }

'''
    atomic_write(source, text.replace(anchor, insertion + anchor, 1))


def write_test() -> None:
    test_path = Path("tests/opening_oricorio_dialga_no_lock_tests.cpp")
    content = r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void choose_opening_active(Engine& engine) { engine.choose_opening_active(); }
};
}  // namespace sim

namespace {
bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State issue_663_hand() {
  sim::State state;
  state.hand = {sim::Card::QuickBall, sim::Card::TeamYellsCheer,
                sim::Card::MegaDragonite, sim::Card::DialgaGX,
                sim::Card::Oricorio, sim::Card::Powerglass,
                sim::Card::RegidragoVstar};
  return state;
}

void require_dialga_active_with_oricorio_held(const sim::State& state, const char* message) {
  if (!state.active || state.active->card != sim::Card::DialgaGX ||
      !contains(state.hand, sim::Card::Oricorio)) {
    throw std::runtime_error(message);
  }
}

void require_oricorio_active_with_dialga_held(const sim::State& state, const char* message) {
  if (!state.active || state.active->card != sim::Card::Oricorio ||
      !contains(state.hand, sim::Card::DialgaGX)) {
    throw std::runtime_error(message);
  }
}

void test_exact_issue_hand_preserves_vital_dance() {
  const sim::Scenario scenario{"issue-663-exact", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{663};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, issue_663_hand());

  // Setup can start Dialga-GX. Mega Dragonite ex preserves a second strict-JIT
  // payload, Quick Ball exposes Regidrago V, and held Oricorio remains a legal
  // later Vital Dance Energy connector when played from hand onto the Bench:
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://github.com/FlareZ123/pokemon-sims/issues/663
  sim::EngineTestAccess::choose_opening_active(engine);
  require_dialga_active_with_oricorio_held(
      sim::EngineTestAccess::state(engine),
      "The exact issue hand should start Dialga-GX and preserve Oricorio.");
}

void test_supporter_search_graph_preserves_vital_dance() {
  const sim::Scenario scenario{"issue-663-supporter-route", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{66301};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::Oricorio, sim::Card::RegidragoVstar,
                sim::Card::StevensResolve, sim::Card::Lusamine,
                sim::Card::Arven, sim::Card::GoodraVstar,
                sim::Card::DialgaGX};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Arven can expose a Basic-search Item and Steven's Resolve can search any three
  // cards, so the public route does not depend on literally holding Quick Ball:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://github.com/FlareZ123/pokemon-sims/issues/663
  sim::EngineTestAccess::choose_opening_active(engine);
  require_dialga_active_with_oricorio_held(
      sim::EngineTestAccess::state(engine),
      "A public Arven or Steven route should preserve Oricorio for Vital Dance.");
}

void test_unique_dialga_payload_remains_protected() {
  const sim::Scenario scenario{"issue-663-unique-payload", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{66302};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = issue_663_hand();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::MegaDragonite),
                   state.hand.end());
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Strict JIT protects the only held modeled payload before the ready turn:
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  sim::EngineTestAccess::choose_opening_active(engine);
  require_oricorio_active_with_dialga_held(
      sim::EngineTestAccess::state(engine),
      "The unique Dialga-GX payload must remain protected in hand.");
}

void test_complete_energy_hand_preserves_payload() {
  const sim::Scenario scenario{"issue-663-energy-complete", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{66303};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = issue_663_hand();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::TeamYellsCheer),
                   state.hand.end());
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::Powerglass),
                   state.hand.end());
  state.hand.push_back(sim::Card::Grass);
  state.hand.push_back(sim::Card::Fire);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Vital Dance has lower marginal value when both required Basic Energy types are
  // already public in hand, so the payload-preserving legacy order remains correct:
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  sim::EngineTestAccess::choose_opening_active(engine);
  require_oricorio_active_with_dialga_held(
      sim::EngineTestAccess::state(engine),
      "A complete GGF Energy hand should preserve Dialga-GX in hand.");
}

void test_no_public_regidrago_route_keeps_legacy_order() {
  const sim::Scenario scenario{"issue-663-no-public-route", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{66304};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::Oricorio, sim::Card::DialgaGX,
                sim::Card::RegidragoVstar, sim::Card::GoodraVstar,
                sim::Card::TeamYellsCheer, sim::Card::Powerglass,
                sim::Card::ChaoticSwell};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // A redundant payload alone does not erase the need for a public Regidrago V
  // connector. Without one, retain Dialga-GX and use the fixed legacy order:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  sim::EngineTestAccess::choose_opening_active(engine);
  require_oricorio_active_with_dialga_held(
      sim::EngineTestAccess::state(engine),
      "No public Regidrago V route should keep the legacy Oricorio-active order.");
}

void test_non_strict_profile_keeps_legacy_order() {
  const sim::Scenario scenario{"issue-663-profile-control", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{66305};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, issue_663_hand());
  sim::EngineTestAccess::choose_opening_active(engine);
  require_oricorio_active_with_dialga_held(
      sim::EngineTestAccess::state(engine),
      "Issue 663's strict-JIT selector must not change no-discard-control openings.");
}
}  // namespace

int main() {
  test_exact_issue_hand_preserves_vital_dance();
  test_supporter_search_graph_preserves_vital_dance();
  test_unique_dialga_payload_remains_protected();
  test_complete_energy_hand_preserves_payload();
  test_no_public_regidrago_route_keeps_legacy_order();
  test_non_strict_profile_keeps_legacy_order();
  return 0;
}
'''
    atomic_write(test_path, content)


def patch_cmake() -> None:
    cmake = Path("CMakeLists.txt")
    text = cmake.read_text(encoding="utf-8")
    target_name = "regidrago_opening_oricorio_dialga_no_lock_tests"
    if target_name not in text:
        target_anchor = '''add_executable(regidrago_opening_dialga_payload_tests tests/opening_active_dialga_payload_tests.cpp)
target_compile_options(regidrago_opening_dialga_payload_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
'''
        target_addition = '''
add_executable(regidrago_opening_oricorio_dialga_no_lock_tests tests/opening_oricorio_dialga_no_lock_tests.cpp)
target_compile_options(regidrago_opening_oricorio_dialga_no_lock_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
'''
        if target_anchor not in text:
            raise RuntimeError("CMake target anchor missing")
        text = text.replace(target_anchor, target_anchor + target_addition, 1)

    test_name = "NAME regidrago_opening_oricorio_dialga_no_lock "
    if test_name not in text:
        test_anchor = "add_test(NAME regidrago_opening_dialga_payload COMMAND regidrago_opening_dialga_payload_tests)\n"
        test_addition = "add_test(NAME regidrago_opening_oricorio_dialga_no_lock COMMAND regidrago_opening_oricorio_dialga_no_lock_tests)\n"
        if test_anchor not in text:
            raise RuntimeError("CMake test anchor missing")
        text = text.replace(test_anchor, test_anchor + test_addition, 1)

    atomic_write(cmake, text)


def main() -> None:
    patch_source()
    write_test()
    patch_cmake()


if __name__ == "__main__":
    main()
