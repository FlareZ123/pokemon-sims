# pokemon-sims

A reproducible, rules-grounded state-machine simulator and research report for the supplied Pokémon TCG Live Expanded Regidrago VSTAR deck.

## Audit status

`fix-sims` contains the audited implementation, regenerated result artifacts, and the full remediation record in [`docs/SIMULATOR_AUDIT.md`](docs/SIMULATOR_AUDIT.md). It corrects material card-rule, hidden-information, search, and connector-domination errors from the earlier simulator. `main` remains unchanged while the fixes are reviewed.

## Scope

The simulator estimates how often the deck reaches an attack-ready Regidrago VSTAR by the player’s turn 2, 3, or 4. It models the supplied 60 cards, setup, mulligans, six prizes, the opening draw, turn draws, active and Bench state, evolution timing, manual Energy attachment, Supporter contention, key card searches, hand-discard costs, Brilliant Blender’s deck discard, Forest Seal Stone’s shared VSTAR-Power limit, strict just-in-time payload timing, and the requested lock stress tests.

It is a single-player goldfish model. It does not simulate opposing damage, prize taking, hand disruption, full discard control, or every historical Expanded card. Lock configurations are explicit scenario constraints. The action policy does not inspect future deck order before a draw or shuffle.

## Reproduce

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
./build/regidrago_sim --trials 100000 --variant-trials 25000 --seed 20260705 --out results/simulation_results.csv --variants-out results/variant_results.csv
```

See `SIM-PLAN.md`, `docs/SIMULATOR_AUDIT.md`, `docs/RULES_AND_INTERACTIONS.md`, `docs/CARD_AUDIT.md`, `docs/MODEL_ASSUMPTIONS.md`, and `docs/REPORT.md`.
