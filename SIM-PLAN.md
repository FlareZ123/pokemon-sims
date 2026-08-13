# SIM-PLAN: Regidrago VSTAR Setup Simulator

## Purpose

The simulator estimates how often a registered Regidrago VSTAR deck reaches its setup-ready state under a fixed rules model and an observable-information action policy.

It is a single-player setup simulator. The output measures setup-policy performance under the registered assumptions. Match-win probability, opponent optimal play, and a complete Expanded game tree are outside the model.

## Registered deck surface

The main simulator registry contains:

- `regidrago-shell`, the default deck;
- `regidrago-pineco`, which includes the Pineco and Forretress ex setup package.

Each registered recipe contains 60 cards and is validated against deck-construction constraints represented by the simulator.

`--all-decks` evaluates both registered recipes across the same aggregate scenario list. Crobat V swap studies use a separate modeling registry and do not become registered decks.

## Trial lifecycle

A trial follows the modeled game state from setup through turn 5.

1. Start from the selected 60-card recipe.
2. Shuffle and draw seven cards.
3. Resolve mulligans until a legal Basic is available.
4. Choose the Active Pokémon and opening Bench through the setup policy.
5. Place six Prize cards from the remaining deck.
6. Begin each player turn with the normal draw.
7. Resolve legal setup actions through the current policy and rules engine.
8. Record the first turn that satisfies the setup-ready predicate.
9. Continue unresolved trials through T5 for diagnostic recovery reporting.

T2, T3, and T4 readiness are setup success. First readiness on T5 is recorded as diagnostic recovery and remains a setup failure.

## State represented by the engine

The simulation tracks exact card identities in the zones needed for setup decisions.

| Area | State |
|---|---|
| Deck | Remaining exact card identities and current shuffled order. |
| Hand | Exact card identities and legal discard candidates. |
| Prizes | Six exact identities, hidden from policy until a legal inspection reveals them. |
| Discard | Exact cards plus same-turn payload tracking used by JIT policy. |
| Active | Pokémon identity, entry timing, attached Energy, Tool state, and setup-relevant flags. |
| Bench | Pokémon identities and setup-relevant attached state. |
| Turn | Supporter use, manual attachment use, VSTAR Power use, lock state, turn end, and seat restrictions. |
| Outcome | First-ready turn, setup failure, mulligans, and aggregate route counters. |

The opponent's hand, attacks, damage, Knock Outs caused by an opposing deck, prize-taking, gust decisions, and post-setup combat are outside the aggregate goldfish model. A supported card may still resolve a mandatory self-Knock-Out when that effect is part of the setup line.

## Knowledge model

The policy uses information available through the modeled game state.

A deck search establishes deterministic knowledge of the remaining deck composition. Prize identities become available only after a legal Prize-inspection effect. Future draw order remains unavailable to the action policy.

Debug trace output may print hidden information for auditing. That output is separate from the policy's decision inputs.

This distinction is central to connector realism. A search card may be legally playable while still lacking a strategically usable target or discard cost under the information state available before resolution.

## Setup-ready predicate

A trial is ready on turn `t` when the engine's current readiness contract is satisfied.

The core requirements are:

1. Regidrago VSTAR is Active.
2. Apex Dragon's `[G][G][R]` attack cost is payable from legally attached Energy.
3. A recipe-permitted modeled Dragon payload is in the discard pile.
4. Strict JIT and matchup-flex JIT require a qualifying payload to have entered the discard pile during turn `t`.
5. `t` is at least 2.

The exact accepted payload set and Energy accounting are maintained in source and documented in [`docs/MODEL_ASSUMPTIONS.md`](docs/MODEL_ASSUMPTIONS.md).

## DCI profiles

Discard-cost intelligence is state-dependent. The policy evaluates whether a card is a legal and strategically acceptable payment at the moment a cost is considered.

### Strict JIT

Payloads remain protected until the turn they can satisfy the ready-state requirement. Key singletons and setup resources remain protected when spending them would damage the current route. Surplus Energy, duplicates, and a payload that immediately completes the current ready turn may become valid discard costs.

### Matchup-flex JIT

Same-turn payload timing remains in force. Additional cards may become discardable when their matchup-facing or recovery value is not required by the scenario and current setup state.

### No discard control

Payloads may enter the discard pile before the ready turn. This profile is an optimistic setup reference for measuring the cost of discard protection.

The detailed policy belongs in [`docs/POLICY_DECISIONS.md`](docs/POLICY_DECISIONS.md).

## Connector model

The engine treats search cards, Supporters, Abilities, Tools, Energy acceleration, switching, and discard outlets as actions with costs and contentions.

Important setup relationships include:

```text
Mysterious Treasure -> Psychic or Dragon Pokémon, with a hand discard cost
Quick Ball -> Basic Pokémon, with a hand discard cost
Earthen Vessel -> Basic Energy access, with a hand discard cost
Arven -> Item plus Tool access
Forest Seal Stone -> one card through the shared VSTAR Power resource
Tapu Lele-GX -> Supporter access when Wonder Tag is legal
Oricorio -> Basic Energy access when Vital Dance is legal
Crispin -> two differently typed Basic Energy destinations when both are searched
Steven's Resolve -> three-card setup package followed by turn end
Brilliant Blender -> selected deck cards moved to discard
Professor Burnet -> Dragon payload access through deck-to-discard search
Latias ex -> zero-retreat Active-position correction for eligible Basic Pokémon
Forretress ex -> setup Energy acceleration through its modeled self-Knock-Out line
```

The printed card effect and the strategic reason to use it are separate concerns. A connector is admitted only when its rules prerequisites, costs, resource contentions, and route value are satisfied by the current state.

## Resource contention

The policy explicitly accounts for shared resources.

A Supporter action can serve only one Supporter route in a turn. Forest Seal Stone uses the same once-per-game VSTAR Power resource as Pokémon VSTAR Powers. Bench space constrains on-play connectors. Manual Energy attachment is limited to one per turn. Discard-cost cards compete for the same hand resources. Item and Ability locks remove actions before route comparison.

These constraints are evaluated before the simulator credits a route with setup progress.

## Evolution and turn timing

The rules engine enforces setup timing relevant to the modeled decks.

Regidrago VSTAR cannot evolve during the player's first turn and cannot evolve during the Pokémon's first turn in play. The starting player follows the first-turn Supporter and attack restrictions represented by the rules engine. One Supporter and one manual Energy attachment are available per turn unless another effect or scenario prevents them.

Opening placement does not trigger an Ability whose condition requires a Pokémon to be played from hand onto the Bench during a turn.

## Lock scenarios

Aggregate scenarios exercise the setup policy under several external constraints.

### Baseline

Items and supported Abilities operate normally.

### Turn-2 Item lock

The first player turn has normal Item access. Item cards are unavailable from that player's second turn onward.

### Rule Box Ability lock

Rule Box Pokémon Abilities are suppressed while the modeled lock remains active. Trainer cards and other legal actions continue according to their own rules.

### Combined lock

Rule Box Ability suppression applies with the persistent Item restriction beginning on turn 2.

Focused regressions may construct narrower synthetic lock states when a rules helper needs direct testing. Aggregate scenario registration remains defined by the current source and [`docs/MODEL_ASSUMPTIONS.md`](docs/MODEL_ASSUMPTIONS.md).

## Prize handling

Six Prize cards are modeled as exact hidden identities.

Hisuian Heavy Ball and Gladion can expose or recover Prize information according to their supported effects. The policy cannot use Prize identities before such an effect establishes the corresponding knowledge state.

This allows the simulator to represent prizing collapse, recovery-card prizing, and the difference between deck knowledge and Prize knowledge without granting an oracle.

## Supported action scope

The engine models card effects and interactions required by the registered setup routes and their validation fixtures. This includes core search Items, setup Supporters, Prize recovery, selected switching, Forest Seal Stone, relevant Abilities, lock removal where supported, Pineco and Forretress ex setup behavior, and the discard routes needed by Apex Dragon readiness.

Card text that matters only after the first successful setup attack may be recorded for policy value while remaining outside the state transition engine. Examples include later damage races and opponent-specific attack consequences.

[`docs/RULES_TRACEABILITY.md`](docs/RULES_TRACEABILITY.md) maps implemented behavior to rules identifiers. [`docs/RULE_SOURCES.md`](docs/RULE_SOURCES.md) contains the source registry.

## CLI modes

### Deterministic trace

`--simulate-this` executes one seed and prints the complete traceable setup line.

### Ready-seed search

`--find-ready N` scans deterministic seeds and prints the first `N` trials that reach a ready state, optionally constrained by `--require-ready-by`.

### Aggregate simulation

`--trials`, `--seed`, `--deck`, and `--out` produce a CSV for the selected registered deck. `--all-decks` emits every registered deck over the same scenario matrix.

### Crobat V modeling

`--model-crobat` produces the dedicated Crobat V swap matrix. `--model-variant` allows a deterministic trace through one modeling recipe.

### Self-test

`--self-test` runs the simulator's built-in parser and core contract checks.

The CLI rejects incompatible mode combinations rather than silently changing their meaning.

## Statistical output

Aggregate rows record readiness by T2 through T5, first-readiness timing, setup failure, Monte Carlo standard error, opening-state metrics, and route-specific counters used by the current reports.

The displayed Monte Carlo standard error for a binary outcome is:

```text
100 * sqrt((x / n) * (1 - x / n) / n)
```

Fixed seeds make committed reports reproducible. Source-bound manifests tie aggregate evidence to the simulator inputs that produced it.

## Result generation

The canonical selected-deck baseline is generated with:

```text
scripts/regenerate_setup_baselines.py
```

The paired registered-deck matrix is generated with:

```text
scripts/generate_multi_deck_comparison.py
```

Generated documentation is refreshed through the corresponding update scripts in `scripts/`.

Result writers use locking and atomic replacement so an interrupted process does not leave a partially written canonical artifact.

## Validation contract

Changes to rules, card resolution, route selection, knowledge transitions, readiness, or aggregate inputs should be validated through the permanent repository surface.

The expected coverage includes:

- C++20 compilation;
- Release tests;
- sanitizer tests;
- exact-state regressions for affected behavior;
- deterministic trace review;
- CLI contract tests;
- aggregate matrix extraction;
- source-bound evidence checks.

When simulator inputs change, committed statistical artifacts must be regenerated from the settled source before their provenance contracts can pass.

## Interpretation

The simulator answers a narrow question: given the registered deck, scenario, policy profile, and modeled rules, how often does the policy assemble an Apex-ready Regidrago VSTAR by each setup turn?

Use the output to compare setup routes, card packages, locks, and policy choices inside that model. Use [`docs/MODEL_ASSUMPTIONS.md`](docs/MODEL_ASSUMPTIONS.md) for the exact boundaries of any probability claim.
