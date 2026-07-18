# Simulator Audit: `fix-sims`

## Status

This branch corrects material card-rule, hidden-information, policy, and traceability defects in the simulator merged to `main`. The active executable is `src/regidrago_sim.cpp`; it is split into `src/trace_engine_v2/` only to make GitHub contents-API commits reviewable.

`fix-sims` now contains three complementary review surfaces:

1. `--simulate-this` deterministic hand traces.
2. `docs/RULES_TRACEABILITY.md` and `docs/RULES_REVERIFICATION.md`, separating card text/core procedure from policy decisions.
3. `regidrago_unified_tests regidrago_policy_tests`, a 57-case exact-state fixture suite documented in `docs/OPTIMAL_POLICY_FIXTURES.md`.

The current `main` branch contains these audited repairs.

## Recent fixture-driven repairs

- Strict JIT now requires a payload to be both discarded during the ready turn and still in discard at the ready check. A recovered payload no longer counts.
- Star Alchemy and Steven's Resolve use useful legal fallback targets after a preferred target is absent from deck or prized.
- The policy now stops spending Supporters after all setup axes are complete, preventing incidental Serena use.
- Forest Seal Stone, Crispin, Earthen Vessel, Gladion, Heavy Ball, Oricorio, Tapu Lele-GX, Latias ex, Tate & Liza, first-turn restrictions, evolution timing, manual attachment, Item lock, and connector domination each have exact-state coverage.

## Validation

The exact staged source was compiled and tested locally in Release and AddressSanitizer/UndefinedBehaviorSanitizer configurations. The current CI runs the complete unified CTest inventory, six deterministic trace regressions, the aggregate smoke test, and sanitizer coverage. The recorded fixture output is in `results/policy_fixture_test_output.txt`.

## Scope boundary

The simulator remains a single-player setup policy model. It does not model opponent damage, Knock Outs, Prize taking, hand disruption, gust, complete Stadium sequencing, Return Label, Lysandre Prism Star, Surprise Box, Mimikyu Copycat, or exhaustive Expanded legality.
