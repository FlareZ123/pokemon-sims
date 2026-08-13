# pokemon-sims

A traceable Pokémon TCG setup simulator for Regidrago VSTAR in Expanded-style card pools.

The program models opening setup, legal card sequencing, deck and Prize knowledge, discard-cost policy, Energy development, evolution timing, setup locks, and the first turn on which Regidrago VSTAR can use Apex Dragon with a modeled payload. It is a single-player setup model. Opponent actions after setup are outside its scope.

## Repository map

- `src/` contains the simulator, card metadata, card-resolution modules, rules adapters, and trace-engine policy.
- `tests/` contains exact-state regressions, CLI contracts, trace audits, and statistical contracts.
- `docs/` contains model assumptions, policy decisions, rules traceability, card sources, audit notes, and generated reports.
- `results/` contains reproducible aggregate matrices, manifests, and reviewed traces.
- `scripts/` contains result-generation and documentation-update tooling.
- `data/` contains repository data inputs.
- `tools/` contains maintenance utilities.
- [`SIM-PLAN.md`](SIM-PLAN.md) defines the current simulation contract.
- [`CARD_CLASS_CLEANUP.md`](CARD_CLASS_CLEANUP.md) defines the current card-class architecture and cleanup rules.
- [`EN_advanced_manual-2025-transcription-structured.md`](EN_advanced_manual-2025-transcription-structured.md) is the searchable Advanced Player's Rulebook transcription used by rule-sensitive code and tests.

Exact rule and card sources are registered in [`docs/RULE_SOURCES.md`](docs/RULE_SOURCES.md) and mapped to simulator behavior in [`docs/RULES_TRACEABILITY.md`](docs/RULES_TRACEABILITY.md).

<a id="named-deck-recipes"></a>
## Registered decks

Aggregate simulation exposes two named 60-card recipes:

- `regidrago-shell`, which is the default when `--deck` is omitted.
- `regidrago-pineco`, which adds the Pineco and Forretress ex setup line and its associated Secret Box package.

`--all-decks` evaluates every registered deck against the same scenario matrix with matched scenario-derived seeds.

Crobat V swap studies use a separate modeling surface through `--model-crobat` and `--model-variant`. Those modeling recipes do not participate in `deck_registry()` or `--all-decks`.

<a id="ready-state-and-t5-policy"></a>
## What counts as ready

A trial becomes setup-ready when the modeled state satisfies all current readiness requirements:

1. Regidrago VSTAR is Active.
2. Apex Dragon's Energy requirement is payable from legally attached Energy.
3. The discard pile contains a modeled Apex Dragon payload permitted by the selected recipe.
4. Strict JIT and matchup-flex JIT require a qualifying payload to enter the discard pile during the ready turn.
5. Readiness begins on turn 2.

Appletun is eligible only in a recipe that contains it. Card and attack sources: https://api.pokemontcg.io/v2/cards/sv8-140 https://api.pokemontcg.io/v2/cards/swsh12-136

T2, T3, and T4 readiness count as setup success. T5 is retained as a diagnostic recovery turn and remains part of setup failure reporting.

The exact predicate and interpretation boundaries are documented in [`docs/MODEL_ASSUMPTIONS.md`](docs/MODEL_ASSUMPTIONS.md) and [`docs/POLICY_DECISIONS.md`](docs/POLICY_DECISIONS.md).

## Knowledge and decision policy

The simulator separates legal information from debug information.

At K0, Prize identities are unavailable to policy. A legal deck inspection establishes K1, where the fixed 60-card recipe plus known zones lets policy deduce the complementary Prize zone; a full legal Prize inspection establishes the same K1 state directly. K1 contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states

Trace output may print debug-only Prize information for auditing, while action selection cannot use that debug channel.

Discard choices are state-dependent. The DCI profiles determine which cards may be spent under the current hand, board, turn, knowledge state, and setup objective. Connector choices account for Supporter contention, Bench space, lock state, once-per-game resources, and whether another legal route dominates the proposed action.

The available policy profiles are:

- `strict-jit`, which protects setup payloads until they can support the current ready turn.
- `matchup-flex-jit`, which keeps same-turn payload timing while allowing additional context-sensitive discard candidates.
- `no-discard-control`, which provides an optimistic setup reference by allowing earlier payload disposal.

## Lock scenarios

Registered scenarios include the baseline setup condition, persistent Item restriction beginning on the player's second turn, Rule Box Ability suppression, and the combined form of those two constraints. Focused tests also exercise synthetic lock states when a narrow rule contract needs them.

Scenario definitions and their exact semantics live in [`docs/MODEL_ASSUMPTIONS.md`](docs/MODEL_ASSUMPTIONS.md).

## Build

CMake 3.22 or newer and a C++20 compiler are required. Python 3 is required because CMake generates the unified test runner during configuration.

Windows with a Visual Studio generator:

```bat
cmake -S . -B build
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

With a single-config generator:

```bat
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

The build creates `regidrago_sim` and `regidrago_unified_tests`.

<a id="run-one-readable-hand"></a>
<a id="find-known-good-trace-seeds"></a>
## Read one deterministic game

On a Visual Studio Release build:

```bat
build\Release\regidrago_sim.exe --simulate-this --deck regidrago-shell --scenario strict-jit/go-second --seed 1 --require-ready-by 3
build\Release\regidrago_sim.exe --simulate-this --deck regidrago-pineco --scenario strict-jit/go-second --seed 35 --require-ready-by 2
```

A trace includes the selected deck, opening state, costs, searches, attachments, evolution, switching, VSTAR Power use, attacks, knowledge transitions, and ready-state checks. State-changing trace lines carry rule identifiers used by the traceability documentation.

Use `--find-ready` when you need deterministic examples that satisfy a scenario and optional deadline:

```bat
build\Release\regidrago_sim.exe --find-ready 3 --start-seed 1 --scenario strict-jit/go-second --require-ready-by 3
```

<a id="generate-the-paired-two-deck-matrices"></a>
<a id="run-aggregate-smoke-test"></a>
## Aggregate simulation

Generate the selected deck's canonical setup matrix:

```bat
python scripts\regenerate_setup_baselines.py --exe build\Release\regidrago_sim.exe --out-dir results --trials 100000 --matrix-seed 20260705
python scripts\update_setup_docs.py --repo-root .
```

Generate the paired registered-deck comparison:

```bat
python scripts\generate_multi_deck_comparison.py --exe build\Release\regidrago_sim.exe --out-dir results --trials 100000 --matrix-seed 20260705
python scripts\update_multi_deck_docs.py --repo-root .
```

The simulator also exposes aggregate mode directly:

```bat
build\Release\regidrago_sim.exe --all-decks --trials 100000 --seed 20260705 --out results\multi_deck_comparison.csv
```

Generated result files use source-bound manifests so committed probability claims stay tied to the simulator inputs that produced them. Result writers use locking and atomic replacement.

<a id="model-crobat-v-swaps"></a>
## Crobat V modeling

Crobat V variants measure setup effects of temporary shell swaps while keeping the registered deck list unchanged:

```bat
build\Release\regidrago_sim.exe --model-crobat --trials 100000 --seed 20260723 --out results\crobat_variant_model.csv
python scripts\update_crobat_modeling_docs.py --csv results\crobat_variant_model.csv --out docs\CROBAT_MODEL_REPORT.md
build\Release\regidrago_sim.exe --simulate-this --model-variant crobat1-erika --scenario strict-jit/go-second --seed 1 --require-ready-by 5
```

The resulting interpretation is documented in [`docs/CROBAT_MODEL_REPORT.md`](docs/CROBAT_MODEL_REPORT.md).

The current source-bound validation covers the 18.2-million-game Crobat matrix: 13 current variants across 14 registered aggregate scenarios at 100,000 trials per condition. Final current-main validation evidence: https://github.com/FlareZ123/pokemon-sims/actions/runs/31164362259 and https://github.com/FlareZ123/pokemon-sims/actions/runs/31164362295. Source-bound scope: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CROBAT_MODEL_REPORT.md

## Validation

The permanent test surface includes:

- strict C++20 compilation;
- Release behavior tests;
- sanitizer builds and tests;
- exact-state rule and policy regressions;
- CLI mode contracts;
- deterministic `--simulate-this` traces;
- matrix extraction and source-bound result contracts;
- fixed-seed aggregate smoke tests.

Run the full configured suite with `ctest`. A focused unified case can be run by passing its case name to `regidrago_unified_tests`.

## Scope

The simulator estimates setup-policy performance under its registered assumptions. It does not calculate match-win rate, opponent optimal play, post-setup damage races, or every legal card interaction in Expanded. Supported card effects are modeled to the extent required by the setup engine and its validated scenarios.

For model boundaries, start with [`SIM-PLAN.md`](SIM-PLAN.md) and [`docs/MODEL_ASSUMPTIONS.md`](docs/MODEL_ASSUMPTIONS.md). For rule provenance, use [`docs/RULES_TRACEABILITY.md`](docs/RULES_TRACEABILITY.md) and [`docs/RULE_SOURCES.md`](docs/RULE_SOURCES.md).
