from pathlib import Path
import fcntl
import os
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def atomic_write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    lock_path = path.with_name(path.name + ".lock")
    with lock_path.open("w", encoding="utf-8") as lock:
        fcntl.flock(lock.fileno(), fcntl.LOCK_EX)
        with tempfile.NamedTemporaryFile(
            "w", encoding="utf-8", dir=path.parent, delete=False
        ) as tmp:
            tmp.write(text)
            tmp.flush()
            os.fsync(tmp.fileno())
            tmp_name = tmp.name
        os.replace(tmp_name, path)
        fcntl.flock(lock.fileno(), fcntl.LOCK_UN)
    lock_path.unlink(missing_ok=True)


# The active Pineco policy is the #1439 wrapper. Resolve #2301 at that outer
# entry so its T3 Quick Ball, Bench, and manual Grass form one route choice before
# the delegated policy can spend Crispin or Earthen Vessel. The T4 completion also
# must run before any competing Secret Box route.
wrapper_path = ROOT / "src/trace_engine_v2/part_issue_1439_treasure_tapu_crispin_override.inc"
wrapper = wrapper_path.read_text(encoding="utf-8")
anchor = "  void run_secret_box_turn() {\n"
insertion = r'''  void run_secret_box_turn() {
    // #2301 must be compared at the outer Pineco-policy boundary. At K0 the
    // narrow Quick Ball selector uses only public state and fixed-list
    // plausibility. The legal Quick Ball search then establishes K1, where the
    // exact T4 continuation is proved before this turn is banked. Returning here
    // prevents the delegated policy from spending reserved Crispin or Vessel.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Official Item, search, Bench, Energy, and Supporter procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K0/K1 and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    if (state_.turn == 3 &&
        issue_2301_quick_ball_timer_cost().has_value() &&
        play_issue_2301_quick_ball_timer_route() &&
        issue_2301_banked_t4_route_ && finish_issue_2301_t3_bank()) {
      trace("POLICY", "P-AXIS-01; P-KNOWLEDGE-01; P-JIT-01",
            "End Pineco policy after banking issue-2301 T4 route: " + state_line());
      return;
    }

    // Resolve the already K1-proven T4 continuation before any generic Secret
    // Box policy can consume its reserved cards. This completion uses the T4
    // Vessel Dragon discard as the strict-JIT payload and never requires the
    // mandatory draw as one of Secret Box's three costs.
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Strict-JIT and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    if (issue_2301_banked_t4_route_ && state_.turn == 4) {
      if (complete_issue_2301_banked_t4_route()) {
        trace("POLICY", "P-AXIS-01; P-JIT-01",
              "End Pineco policy after deterministic issue-2301 T4 completion: " +
                  state_line());
        return;
      }
      issue_2301_banked_t4_route_ = false;
    }
'''
if insertion not in wrapper:
    if wrapper.count(anchor) != 1:
        raise RuntimeError("#2301 active Pineco-policy wrapper anchor mismatch")
    wrapper = wrapper.replace(anchor, insertion, 1)
atomic_write(wrapper_path, wrapper)

# Keep trace/rule IDs aligned with the repository's active policy registry.
route_path = ROOT / "src/trace_engine_v2/part_issue_2301_pineco_quick_ball_vessel_route.inc"
route = route_path.read_text(encoding="utf-8")
route = route.replace("R-VESSEL-01", "R-EV-01")
route = route.replace("R-FOREST-01", "R-FOREST-VITALITY-01")
atomic_write(route_path, route)
