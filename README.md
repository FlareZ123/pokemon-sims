# pokemon-sims

A reproducible, rules-grounded state-machine simulator and research report for the supplied Pokémon TCG Live Expanded Regidrago VSTAR deck.

## Scope

The simulator estimates how often the deck reaches an attack-ready Regidrago VSTAR by the player’s turn 2, 3, or 4. It models the supplied 60 cards, setup, mulligans, six prizes, the opening draw, turn draws, active/bench state, evolution timing, manual Energy attachment, Supporter contention, key card searches, hand-discard costs, Brilliant Blender’s deck discard, Forest Seal Stone’s shared VSTAR-Power limit, strict just-in-time payload timing, and the requested lock stress tests.

It is deliberately a *single-player goldfish model*. It does not claim to simulate an opponent’s damage, hand disruption, prizes taken, deck knowledge, or every historical Expanded card. Lock configurations are exogenous scenario constraints. The action policy does not inspect deck order before a draw or shuffle.

## Reproduce

```bash
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build --output-on-failure
./build/regidrago_sim --trials 100000 --variant-trials 25000 --seed 20260704 --out results/simulation_results.csv --variants-out results/variant_results.csv
```

See `SIM-PLAN.md`, `docs/RULES_AND_INTERACTIONS.md`, `docs/CARD_AUDIT.md`, `docs/MODEL_ASSUMPTIONS.md`, and `docs/REPORT.md`.
