#!/usr/bin/env python3
"""Apply the confirmed issue-1461 production fix atomically."""
from __future__ import annotations

import fcntl
import os
import subprocess
import tempfile
from pathlib import Path

SOURCE = Path("src/trace_engine_v2/part_issue_1118_secret_box.inc")
LOCK = Path(".git/issue-1461-source.lock")

CURRENT_ADMISSION = '''    // Steven's K1 projection must survive the mandatory turn-two draw. Two
    // copies of each Energy type preserve both Crispin channels after any one
    // draw. Three eligible Treasure targets preserve one target after Steven
    // takes Dragapult ex and the next draw removes another eligible Pokémon:
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // One-Energy Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Future-card oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#policy-versus-future-card-oracle
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1461
    const bool direct_t2_treasure_route =
        deck_seen_ && scenario_.dci == DciProfile::StrictJit &&
        state_.turn == 1 && !scenario_.going_first &&
        !next_turn_item_locked && state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->grass == 1 && state_.active->fire == 0 &&
        hand_count(Card::MysteriousTreasure) > 0 &&
        hand_count(Card::RegidragoVstar) > 0 &&
        hand_count(Card::Fire) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::Dragapult) > 0 &&
        deck_count_after_search_started(Card::Grass) > 1 &&
        deck_count_after_search_started(Card::Fire) > 1 &&
        treasure_targets >= 3;
'''

NEW_ADMISSION = '''    const int searchable_grass =
        deck_count_after_search_started(Card::Grass);
    const int searchable_fire =
        deck_count_after_search_started(Card::Fire);
    const bool crispin_route_survives_next_draw =
        searchable_grass > 1 && searchable_fire > 1;
    const bool prior_turn_pineco =
        (state_.active && state_.active->card == Card::Pineco &&
         state_.active->entered_turn < state_.turn) ||
        std::any_of(state_.bench.begin(), state_.bench.end(),
                    [this](const Pokemon& pokemon) {
          return pokemon.card == Card::Pineco &&
              pokemon.entered_turn < state_.turn;
        });
    const bool forretress_fallback_survives_next_draw =
        prior_turn_pineco && hand_count(Card::ForretressEx) > 0 &&
        ability_available_for_pokemon(Card::ForretressEx) &&
        searchable_grass > 1;

    // Steven's K1 projection must survive every mandatory turn-two draw. Two
    // copies of both Energy types preserve the direct Crispin continuation.
    // A one-copy Fire channel is also safe when a prior-turn Pineco and held
    // Forretress ex preserve Grass through Exploding Energy while the held Fire
    // supplies the manual attachment. Three Treasure targets leave one legal
    // target after Steven takes Dragapult ex and the next draw removes another:
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // One-Energy Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
    // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Core draw, Ability, attachment, evolution, and Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Future-card oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#policy-versus-future-card-oracle
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1461
    const bool direct_t2_treasure_route =
        deck_seen_ && scenario_.dci == DciProfile::StrictJit &&
        state_.turn == 1 && !scenario_.going_first &&
        !next_turn_item_locked && state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->grass == 1 && state_.active->fire == 0 &&
        hand_count(Card::MysteriousTreasure) > 0 &&
        hand_count(Card::RegidragoVstar) > 0 &&
        hand_count(Card::Fire) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::Dragapult) > 0 &&
        (crispin_route_survives_next_draw ||
         forretress_fallback_survives_next_draw) &&
        treasure_targets >= 3;
'''

CURRENT_CONTINUATION = '''    if (issue_1420_direct_t2_treasure_route_) {
      // Steven's T1 K1 projection normally resolves the four actions that establish
      // strict-JIT readiness. Ordinary draws remain legal game state, so any changed
      // projection falls through to the general policy from the fully resolved state:
      // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
      // One-Energy Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Core draw, Energy, Supporter, Item, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Future-card oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#policy-versus-future-card-oracle
      // Confirmed bugs: https://github.com/FlareZ123/pokemon-sims/issues/1420
      // https://github.com/FlareZ123/pokemon-sims/issues/1461
      bool route_completed = false;
      if (state_.turn == 2 && !item_locked() && play_crispin()) {
        attach_manual();
        evolve_best_regi();
        route_completed = active_is_vstar() && !need_energy() &&
            play_mysterious_treasure(true);
      }
      if (route_completed) {
        trace("POLICY", "R-STEVEN-01; R-CRISPIN-01; R-MT-01; P-AXIS-01",
              "Completed the resource-preserving direct T2 Mysterious Treasure "
              "route without advancing the Pineco line: " + state_line());
        return;
      }

      issue_1420_direct_t2_treasure_route_ = false;
      trace("POLICY", "R-STEVEN-01; R-CRISPIN-01; R-MT-01; P-AXIS-01",
            "A legal next draw changed the banked T2 projection. Policy continued "
            "from the resolved public state: " + state_line());
    }
'''

NEW_CONTINUATION = '''    if (issue_1420_direct_t2_treasure_route_) {
      // Read the exact post-draw K1 state before spending Crispin, the manual
      // attachment, or the held payload. When both Energy types remain, resolve
      // the resource-preserving direct route. When the final copy of one type was
      // drawn, clear the banked flag and let the already proved Forretress route
      // continue from the untouched public state:
      // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
      // One-Energy Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
      // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Core draw, Energy, Supporter, Ability, Item, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Future-card oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#policy-versus-future-card-oracle
      // Confirmed bugs: https://github.com/FlareZ123/pokemon-sims/issues/1420
      // https://github.com/FlareZ123/pokemon-sims/issues/1461
      const bool both_crispin_types_remain =
          deck_count_after_search_started(Card::Grass) > 0 &&
          deck_count_after_search_started(Card::Fire) > 0;
      bool route_completed = false;
      if (state_.turn == 2 && !item_locked() && both_crispin_types_remain &&
          play_crispin()) {
        attach_manual();
        evolve_best_regi();
        route_completed = active_is_vstar() && !need_energy() &&
            play_mysterious_treasure(true);
      }
      if (route_completed) {
        trace("POLICY", "R-STEVEN-01; R-CRISPIN-01; R-MT-01; P-AXIS-01",
              "Completed the resource-preserving direct T2 Mysterious Treasure "
              "route without advancing the Pineco line: " + state_line());
        return;
      }

      issue_1420_direct_t2_treasure_route_ = false;
      trace("POLICY", "R-STEVEN-01; R-CRISPIN-01; R-MT-01; P-AXIS-01",
            both_crispin_types_remain
                ? "The banked continuation changed after legal resolution. Policy "
                  "continued from the resolved public state: " + state_line()
                : "The mandatory draw removed a Crispin Energy type. Policy "
                  "preserved the untouched hand for the proved Forretress route: " +
                  state_line());
    }
'''


def write_atomic(path: Path, content: str) -> None:
    with tempfile.NamedTemporaryFile(
        mode="w", encoding="utf-8", newline="\n", dir=path.parent, delete=False
    ) as temporary:
        temporary.write(content)
        temporary.flush()
        os.fsync(temporary.fileno())
        temporary_path = Path(temporary.name)
    os.replace(temporary_path, path)


def replace_once(content: str, old: str, new: str, label: str) -> str:
    if new in content:
        return content
    if content.count(old) != 1:
        raise RuntimeError(f"{label} anchor was missing or ambiguous")
    return content.replace(old, new, 1)


def commit_ci_change() -> None:
    head_ref = os.environ.get("GITHUB_HEAD_REF")
    if os.environ.get("GITHUB_ACTIONS") != "true" or not head_ref:
        return
    subprocess.run(["git", "add", str(SOURCE)], check=True)
    staged = subprocess.run(
        ["git", "diff", "--cached", "--quiet"], check=False
    ).returncode
    if staged == 0:
        return
    subprocess.run(
        ["git", "commit", "-m", "Preserve the Steven route through final Energy draws"],
        check=True,
    )
    subprocess.run(
        ["git", "push", "origin", f"HEAD:{head_ref}"],
        check=True,
    )


def main() -> int:
    LOCK.parent.mkdir(parents=True, exist_ok=True)
    with LOCK.open("w", encoding="utf-8") as lock_file:
        fcntl.flock(lock_file.fileno(), fcntl.LOCK_EX)
        source = SOURCE.read_text(encoding="utf-8")
        source = replace_once(
            source, CURRENT_ADMISSION, NEW_ADMISSION, "issue-1461 admission"
        )
        source = replace_once(
            source, CURRENT_CONTINUATION, NEW_CONTINUATION,
            "issue-1461 continuation"
        )
        write_atomic(SOURCE, source)
    commit_ci_change()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
