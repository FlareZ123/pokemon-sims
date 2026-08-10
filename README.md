# pokemon-sims

A traceable, rules-mapped goldfish simulator for two named Pokémon TCG Live Expanded Regidrago VSTAR recipes.

## Audit status

The repository contains a rules-mapped simulator, deterministic `simulate-this` traces, core exact-state policy fixtures, Tier Two choice-differentiation fixtures, fixed-seed statistical matrices, source provenance, and sanitizer validation. Exact rule and card sources are registered in [`docs/RULE_SOURCES.md`](docs/RULE_SOURCES.md) and [`docs/RULES_TRACEABILITY.md`](docs/RULES_TRACEABILITY.md).

The model is deliberately a single-player setup model. It does not resolve every Expanded card or opponent-dependent game state.

<a name="registered-decks"></a>

## Named deck recipes

The validated registry contains exactly two decks:

- `regidrago-shell`: the historical repository list with Klara `swsh6-145` permanently replacing Roseanne's Backup. This remains the default when `--deck` is omitted.
- `regidrago-pineco`: the Secret Box list with two Pineco, two Forretress ex, two Dawn, two Forest of Vitality, four Tapu Lele-GX, and Appletun `sv8-140` as a discard-only Dragon payload.

The withdrawn Pineco Brilliant Blender proposal is not registered. Every registered recipe must contain exactly 60 cards, obey the four-copy limit outside Basic Energy, contain at most one ACE SPEC, and include the lower stage for modeled in-play evolution lines.

Direct identities: Secret Box https://api.pokemontcg.io/v2/cards/sv6-163, Appletun https://api.pokemontcg.io/v2/cards/sv8-140, Pineco https://api.pokemontcg.io/v2/cards/sv4pt5-1, Forretress ex https://api.pokemontcg.io/v2/cards/sv4pt5-2, Dawn https://api.pokemontcg.io/v2/cards/me2-87, and Forest of Vitality https://api.pokemontcg.io/v2/cards/me1-117.


## Klara recovery policy

The registered shell uses Klara `swsh6-145`. It can recover up to two Pokémon and up to two Basic Energy from discard directly to hand in one Supporter action. Setup policy preserves the sole current-turn strict-JIT Dragon, admits an older-Dragon recovery only when a legal Item can discard it again that turn, and uses optional additional targets for safe discard-control denial and immediate Basic Energy recovery. `regidrago-pineco` did not contain Roseanne's Backup, so that registered recipe is unchanged. Direct sources: https://api.pokemontcg.io/v2/cards/swsh6-145 https://api.pokemontcg.io/v2/cards/sv5-146 https://github.com/FlareZ123/pokemon-sims/issues/1773

## Battle VIP Pass behavior

Battle VIP Pass `swsh8-225` is fully modeled as a first-turn-only Item. A legal resolution inspects the deck, establishes K1, puts up to two setup-advancing Basic Pokémon directly onto the Bench, and shuffles. Direct deck-to-Bench placement does not trigger Wonder Tag, Vital Dance, or Dark Asset because those Abilities require a play from hand. Static in-play effects such as Latias ex's Skyliner apply normally after placement. Copies remaining after turn one are stable dead-card DCI. The card is supported by the engine and tests without changing either registered 60-card recipe: https://api.pokemontcg.io/v2/cards/swsh8-225 https://github.com/FlareZ123/pokemon-sims/issues/1647

The validation matrix in [`docs/BATTLE_VIP_PASS_VALIDATION.md`](docs/BATTLE_VIP_PASS_VALIDATION.md) uses an unregistered temporary shell derivative with `-1 Quick Ball`, `-1 Mysterious Treasure`, and `+2 Battle VIP Pass`. The temporary recipe is excluded from `deck_registry()` and `--all-decks`.

## Professor's Letter paper Expanded model

Professor's Letter `xy1-123` is modeled as a paper-Expanded Item that searches up to two Basic Energy into hand and has no discard cost. It establishes K1 through the physical deck search and is held when the Energy axis is already complete. It is intentionally absent from both registered Pokémon TCG Live recipes. CI evaluates an unregistered temporary `regidrago-shell` derivative with exactly `-2 Earthen Vessel` and `+2 Professor's Letter`, using matched scenario seeds and checkpoint metrics for Regidrago, VSTAR, Active position, semantic Apex Energy, current-turn payload, and K1. Professor's Letter: https://api.pokemontcg.io/v2/cards/xy1-123 ; enhancement: https://github.com/FlareZ123/pokemon-sims/issues/2509

## Run one readable hand

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
./build/regidrago_sim --simulate-this --deck regidrago-shell --scenario strict-jit/go-second --seed 1 --require-ready-by 3
./build/regidrago_sim --simulate-this --deck regidrago-pineco --scenario strict-jit/go-second --seed 35 --require-ready-by 2
```

Each trace prints the selected deck ID, opening player-known state, debug-only Prize cards, every cost, search, attachment, evolution, retreat, VSTAR Power, attack, and ready-state check. Policy never reads debug Prize output before a legal deck or Prize inspection.

## Find known-good trace seeds

Use `--find-ready N` to locate deterministic seeds that satisfy a scenario and optional readiness deadline. `--start-seed` selects the first candidate seed:

```bash
./build/regidrago_sim --find-ready 3 --start-seed 1 --scenario strict-jit/go-second --require-ready-by 3
```

The mode is source-defined and rejects incompatible trace or aggregate options: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc#L238-L437 https://github.com/FlareZ123/pokemon-sims/blob/main/tests/run_cli_mode_contract.cmake#L58-L184 https://github.com/FlareZ123/pokemon-sims/issues/1542

## Run tests

```bash
ctest --test-dir build --output-on-failure
./build/regidrago_unified_tests issue_1118_multi_deck_secret_box_tests
```

The issue-specific suite covers recipe validation, unknown or withdrawn deck identities, Appletun stage and payload identity, the complete Secret Box route, strict DCI cost protection, Mysterious Treasure cost reservation, Bench limits, turn-one evolution, locks, legal Prize recovery, and reviewed seeded traces.

<a name="run-aggregate-smoke-test"></a>

## Generate the canonical shell baseline

```bash
python scripts/regenerate_setup_baselines.py --exe build/regidrago_sim --out-dir results --trials 100000 --matrix-seed 20260705
python scripts/update_setup_docs.py --repo-root .
```

The historical aggregate CLI remains backward compatible:

```bash
./build/regidrago_sim --trials 100000 --seed 20260705 --out results/simulation_results.csv
```

Turn-one full Item-lock scenarios are intentionally not registered for current-paper Expanded reporting and must not be reintroduced. The starting player cannot attack on the first turn, and Forest of Giant Plants, the historical immediate-evolution enabler for turn-one Vileplume-style locks, is banned in Expanded. Use `strict-jit-turn2-item-lock/*` for Item lock after the initial setup opportunity. `strict-jit-combined-lock/*` combines Rule Box Ability suppression with that same turn-two Item timing. `LockMode::FullItem` remains only for focused synthetic or historical fixtures. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

## Generate the paired two-deck matrices

```bash
python scripts/generate_multi_deck_comparison.py --exe build/regidrago_sim --out-dir results --trials 100000 --matrix-seed 20260705
python scripts/update_multi_deck_docs.py --repo-root .
```

The paired generator runs both decks across all 14 registered aggregate scenarios, producing 28 independent 100,000-trial rows and 2.8 million simulated games. It writes `results/multi_deck_comparison.csv`, `results/multi_deck_manifest.json`, reviewed named-deck traces, and [`docs/MULTI_DECK_REPORT.md`](docs/MULTI_DECK_REPORT.md). Both decks use the same derived seed for each scenario.

The equivalent direct command is:

```bash
./build/regidrago_sim --all-decks --trials 100000 --seed 20260705 --out results/multi_deck_comparison.csv
```

## Model Crobat V swaps

Crobat V modeling variants are temporary derivatives of `regidrago-shell`. They are not registered decks and never appear in `--all-decks`:

```bash
./build/regidrago_sim --model-crobat --trials 100000 --seed 20260723 --out results/crobat_variant_model.csv
python scripts/update_crobat_modeling_docs.py --csv results/crobat_variant_model.csv --out docs/CROBAT_MODEL_REPORT.md
./build/regidrago_sim --simulate-this --model-variant crobat1-erika --scenario strict-jit/go-second --seed 1 --require-ready-by 5
```

When `--out` or `--seed` is omitted, `--model-crobat` defaults to `results/crobat_variant_model.csv` and seed `20260723`, preserving the canonical baseline: https://github.com/FlareZ123/pokemon-sims/issues/1394

Dark Asset follows Crobat V `swsh3-104`: it triggers only from a hand-to-Bench play during the turn, draws until six, is limited to one use each turn, and is suppressed by Rule Box Ability lock. Results and discrete cut costs are recorded in [`docs/CROBAT_MODEL_REPORT.md`](docs/CROBAT_MODEL_REPORT.md): https://api.pokemontcg.io/v2/cards/swsh3-104

The current-main validation includes the full Release and sanitizer suites, three reviewed traces, the 18.2-million-game Crobat matrix, byte-for-byte registered-deck probability checks, safe CLI-default validation, and precise game-level Dark Asset statistics: https://github.com/FlareZ123/pokemon-sims/actions/runs/31164362259 https://github.com/FlareZ123/pokemon-sims/actions/runs/31164362295

## Ready-state and T5 policy

A ready state requires:

1. Active Regidrago VSTAR.
2. Semantic payment of Apex Dragon's `[Grass][Grass][Fire]` Energy cost, including legal Double Dragon Energy payment when attached to a Dragon Pokémon. Sources: https://api.pokemontcg.io/v2/cards/swsh12-136 https://api.pokemontcg.io/v2/cards/xy6-97 https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_004.inc https://github.com/FlareZ123/pokemon-sims/issues/2697
3. A modeled Dragon payload in discard. Appletun is eligible only in a recipe that contains it.
4. Strict and matchup-flex JIT require the payload to enter discard during the ready turn.

Readiness through T4 is setup success. T5 remains a diagnostic recovery turn and is counted inside setup failure.

See `docs/MODEL_ASSUMPTIONS.md`, `docs/POLICY_DECISIONS.md`, `docs/RULES_TRACEABILITY.md`, `docs/RULE_SOURCES.md`, `docs/TRACE_AUDIT.md`, `docs/REPORT.md`, and `docs/MULTI_DECK_REPORT.md`.
