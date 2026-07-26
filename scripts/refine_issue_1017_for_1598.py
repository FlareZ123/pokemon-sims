from pathlib import Path
from textwrap import dedent

path = Path("tests/issue_1017_late_steven_vstar_vessel_tests.cpp")
source = path.read_text(encoding="utf-8")
old = dedent(
    '''\
      // Wonder Tag must select the complete Steven route, then Vessel must discard the
      // held Dragon after the Active Regidrago V evolves on T4:
      // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
      // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
      // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://github.com/FlareZ123/pokemon-sims/issues/1017
      expect(trace_contains(trace, "T3 | WONDER TAG | rules: R-TAPU-01 | Searched and revealed Steven's Resolve"),
             "Seed 14 must choose Steven instead of Burnet.");
      expect(trace_contains(trace, "T3 | PLAY SUPPORTER | rules: R-STEVEN-01"),
             "Seed 14 must resolve late Steven on T3.");
      expect(trace_contains(trace, "T4 | DISCARD | rules: R-EV-01 | Mega Dragonite ex"),
             "Seed 14 must use Vessel as the strict-JIT payload outlet.");
      expect(trace_contains(trace, "T4 | READY"),
             "Seed 14 must reach readiness on T4.");
      expect(outcome.ready_by_4 && outcome.first_ready_turn == 4,
             "Seed 14 must be ready by exactly T4.");
    '''
)
new = dedent(
    '''\
      // Issue #1598 supersedes the older T4 Steven expectation for this exact seed.
      // Gladion reveals and recovers Mysterious Treasure on T2. The held Fire and
      // Dragon then make the T3 Treasure search, evolution, and strict-JIT payload
      // deterministic, one turn earlier than the preserved Steven-Vessel fallback:
      // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2-166
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Core Prize, Supporter, Item, discard, search, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Original fallback regression: https://github.com/FlareZ123/pokemon-sims/issues/1017
      // Confirmed faster route: https://github.com/FlareZ123/pokemon-sims/issues/1598
      expect(trace_contains(trace, "T2 | PLAY SUPPORTER | rules: R-GLADION-01") &&
                 trace_contains(trace, "exchanged Gladion for Mysterious Treasure"),
             "Seed 14 must recover Mysterious Treasure with Gladion on T2.");
      expect(trace_contains(trace, "T3 | DISCARD | rules: R-MT-01 | Mega Dragonite ex") &&
                 trace_contains(trace, "T3 | EVOLVE") &&
                 trace_contains(trace, "T3 | READY"),
             "Seed 14 must use the deterministic T3 Treasure route.");
      expect(outcome.ready_by_3 && outcome.first_ready_turn == 3,
             "Seed 14 must be ready by exactly T3.");
    '''
)
if source.count(old) != 1:
    raise SystemExit(f"issue-1017 refinement anchor count: {source.count(old)}")
path.write_text(source.replace(old, new, 1), encoding="utf-8")
