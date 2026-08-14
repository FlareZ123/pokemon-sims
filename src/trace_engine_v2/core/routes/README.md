# Core route policies

This directory owns simulator route-policy fragments whose primary responsibility is route admission, projection, or decision logic. It is distinct from intrinsic card metadata and printed card resolution.

## Current owners

- `issue_962_route.inc` owns the issue-962 Quick Ball -> Tapu Lele-GX -> Crispin admission, shadow projection, and connector-domination decision. Its three sections are composed at one Engine member boundary by `part_014a_issue_962_eligibility.inc`.
- `issue_1447_vessel_hold_policy.inc` owns the issue-1447 Earthen Vessel hold and ready-turn policy. Its historical `part_014a_issue_1447_vessel_hold.inc` path is a boundary forwarder only.

Compatibility `part_*.inc` files may forward into this directory when a proven Engine member boundary still depends on the historical include site. Keep those forwarders thin, preserve macro order, and retire them only after repository-wide reference checks and CI prove the boundary-safe replacement.

Route policy must continue to use existing Engine legality and resource helpers instead of recreating card legality, K0/K1, DCI, AMR, or connector-cost logic locally. Policy source: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md

C++ textual include semantics: https://eel.is/c++draft/cpp.include

Architecture and cleanup contract: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
