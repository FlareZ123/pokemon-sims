# Audit Status

## Current source and test inventory

- Core exact-state policy fixtures: **39** in `regidrago_policy_tests`.
- Tier Two choice-differentiation fixtures: **26** in `regidrago_tier2_tests`.
- Deterministic scenario trace regressions: six CTest cases.
- Aggregate scenario smoke test: one CTest case.

The current audit added or corrected source-linked regressions for:

1. Heavy Ball K0 Prize information and Tapu Lele-GX chaining.
2. Heavy Ball K1 deduction, including known-dead Crispin and known no-Basic Prize retention.
3. Final prized Energy via Gladion, without preempting a Burnet Item-lock payload bridge.
4. Steven's Resolve selecting Gladion after it sees a prized VSTAR.
5. Forest Seal Stone K1 target selection for the same final-Energy line.
6. Celestial Roar moving attached Energy out of discard.
7. Full-Bench holds for Basic search connectors.
8. Tate & Liza's net-positive four-card draw mode.
9. Serena's printed one-to-three discard draw mode, including three safe discards in strict JIT.
10. Heavy Ball selecting Oricorio over Tapu when that preserves in-hand Burnet under Item lock.

## Build evidence and limit

An isolated manual C++ reconstruction of the corrected rule-sensitive algorithms was compiled under AddressSanitizer and UndefinedBehaviorSanitizer in this runtime. It covered Prize-to-deck K1 deduction, Celestial Roar final-zone transfer, strict Serena safe discards, the prohibition on zero-discard Serena, and Steven's Resolve's K1 Gladion selection.

This is not an exact branch build. The private repository cannot be cloned from this environment because `github.com` does not resolve here, and the available GitHub connector cannot materialize the complete split source tree into the local compiler workspace.

## GitHub Actions visibility

The available connector can read the workflow definition and classic combined commit statuses. It cannot list push-triggered workflow runs or retrieve historical job logs for this branch. The current combined-status surface is empty. The workflow now emits `LastTest.log` and uploads CTest diagnostic artifacts on future failures.

## Remaining explicit model assumption

Forest Seal Stone activation while the `FullRuleBoxAbility` scenario is active remains a documented model assumption. The audit did not find a direct published ruling source for that exact interaction. It must not be treated as a verified tournament ruling.
