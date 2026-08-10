# Debug-mode CI audit, 2026-08-10

This temporary PR-only marker triggers source-bound CI from current `main` for repository-wide debug review. It does not change simulator policy or gameplay behavior.

The audit reviews the permanent `--simulate-this` traces against repository policy, the top-level advanced manual, and authoritative card data before deciding whether a new bug exists.

A static review candidate is the completed #1478 Field Blower direct-Regidrago route. Its production predicate still requires literal `StrictJit`, while repository policy gives `StrictJit` and `MatchupFlexJit` the same same-ready-turn Dragon-payload timing. CI artifact traces will be used to verify whether the historical seed 290 state exposes a real matchup-flex regression before any issue is filed.

- Advanced manual: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
- Decision policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
- Model assumptions: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
- Completed strict-JIT route: https://github.com/FlareZ123/pokemon-sims/issues/1478
- Production route: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_issue_1476_redundant_burnet_route_override.inc
