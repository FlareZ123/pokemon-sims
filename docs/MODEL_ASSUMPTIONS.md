# Model Assumptions, Bounds, and Interpretation

## What the probability means

Every percentage is the estimated chance that a randomized opening sequence reaches the model’s setup-ready predicate using the included observable-information policy. It is a policy estimate under stated assumptions. It is not a match-win percentage, a meta share prediction, or a proof that every legal play sequence was searched exhaustively.

The fixed seed makes each report reproducible. The Monte Carlo standard error shown in the baseline CSV measures sampling variation, not modeling error.

## Fully modeled parts

- Exact 60-card counts.
- Opening seven-card hand.
- Mulligans.
- Active selection.
- Opening Bench selection.
- Six exact Prize cards after opening placement.
- Start-of-turn draw.
- Going-first and going-second first-turn differences.
- One Supporter per turn.
- One manual Energy attachment per turn.
- Regidrago VSTAR evolution timing.
- Basic Bench limit.
- Regidrago V, Regidrago VSTAR, Tapu Lele-GX, Latias ex, Oricorio, and core search-card state transitions.
- Regidrago V’s Celestial Roar top-three discard and Basic Energy attachment in its modeled going-second turn-one use: https://api.pokemontcg.io/v2/cards/swsh12-135
- Mysterious Treasure, Quick Ball, Earthen Vessel, Brilliant Blender, Arven, Crispin, Professor Burnet, Serena draw, Tate & Liza draw, Steven’s Resolve, Gladion, Hisuian Heavy Ball, and Forest Seal Stone routes relevant to setup.
- Path-style Rule Box Ability suppression.
- Item-lock restrictions.
- Strict same-turn payload tracking.
- Matched-seed deck swap comparisons.

## Deliberate simplifications

### Opponent actions

No opposing deck is played. The model does not execute damage, Knock Outs, prize-taking, gust, bench pressure, hand disruption, Return Label, Lysandre Prism Star, Surprise Box, Girafarig, or any other opponent card. Lock scenarios are injected as exogenous constraints.

This means the no-lock baseline is a setup goldfish. It is useful for comparing connector quality. It is not a safe prediction in a competitive game.

### Card text outside initial setup

Many card effects have correct text recorded in `RULES_AND_INTERACTIONS.md`, though the current engine stops at first attack readiness. It does not resolve:

- Phantom Dive damage allocation.
- Ryuno Glide damage and the later Powerglass loop.
- Timeless-GX extra-turn game state.
- Rolling Iron damage reduction against a modeled attack.
- Mawile-GX opponent-hand and Bench effects.
- Guzma’s board state after the setup phase.
- Channeler, recovery loops, Field Blower targets, Chaotic Swell replacement, or Professor Turo after setup.

Their discrete value is still preserved in the DCI profiles and swap discussion. This prevents a speed-only model from calling a strong matchup card “worthless.”

### Policy versus future-card oracle

A player can inspect the deck when a search effect resolves. A player cannot inspect the future top cards before selecting an action. The code allows exact legal card selection from the remaining deck, then uses a shuffled deck for draws. The policy selects its action from hand, public board, remaining deck composition, Prize cards only when an effect reveals them, and the scenario configuration. It does not branch on future draw identity.

This avoids a common simulation error where a solver chooses Serena because it already knows Serena will draw the missing VSTAR.

### Draw Supporters

Serena and Tate & Liza use simple fallback priorities. They are not evaluated by a full expected-value belief-state calculation. Their performance is therefore a conservative policy estimate, with further refinement planned in `SIM-PLAN.md`.

### Strict JIT definition

Strict JIT requires one A/S payload to enter discard during the present turn. The model does not require the payload to be selected by Blender specifically. Mysterious Treasure, Quick Ball, Earthen Vessel, Burnet, or Serena may establish a legal current-turn payload where their other conditions are met.

### “A or S payload” definition

The requested success predicate treats the following as valid:

- S tier: Dragapult ex, Mega Dragonite ex.
- A tier: Dialga-GX, Hisuian Goodra VSTAR.

The model does not attempt matchup-dependent damage mathematics when choosing between those targets. It only verifies availability for an Apex attack.

## DCI implementation

The model assigns cards to strategic discard categories rather than treating every card in hand as equal fuel.

| Profile | Hand cards protected | Hand cards potentially usable as discard fuel |
|---|---|---|
| Strict JIT | payloads before the attack turn, key singletons, recovery, matchup cards | duplicates and surplus Energy, plus a same-turn payload when it immediately enables Apex |
| Matchup-flex JIT | current JIT payload timing still protected | visibly low-value anti-matchup cards, selected reactive Supporters, stadiums, recovery pieces, Powerglass, extra copies, and surplus Energy |
| No discard control | only resources needed immediately | the matchup-flex set plus pre-attack A/S payloads |

This is a principled approximation. DCI in a real match depends on opponent deck information, prizes, turn sequence, bench capacity, known hand, and current board. The source code centralizes its choices in `choose_discard` for later refinement.

## Lock interpretation

### Turn-2 Item lock

The player gets a normal first turn. Items are locked from their second turn onward. This models a lock that arrives after an initial setup opportunity and makes turn-2 Blender, Mysterious Treasure, Quick Ball, Earthen Vessel, Forest Seal Stone, and Hisuian Heavy Ball unavailable.

### Full Item lock

Items are unavailable for all modeled turns. Forest Seal Stone is also unavailable because playing the Tool is an Item action. Professor Burnet and Steven’s Resolve remain legal Supporter routes.

### Rule Box Ability lock

The model applies Path-to-the-Peak-style suppression to Rule Box Pokémon Abilities. Tapu Lele-GX, Latias ex, Mawile-GX, and Pokémon VSTAR Abilities are suppressed. Oricorio `sm2-55` has no Rule Box, so Vital Dance remains available: https://api.pokemontcg.io/v2/cards/sm2-55. Apex Dragon remains an attack. Forest Seal Stone’s Tool VSTAR Power remains available where Item lock is absent because its grant is printed on the Tool: https://api.pokemontcg.io/v2/cards/swsh12-156. Path to the Peak removes Abilities from Pokémon with Rule Boxes: https://api.pokemontcg.io/v2/cards/swsh6-148.

### Combined lock

This stacks full Item lock and Rule Box Ability lock. It is a deliberately severe stress test. It should be read as a measure of the deck’s natural raw draws and Supporter-only recovery rather than as a common board state.

## Prizing implementation

Six cards are taken after opening Basic placements. The model uses their exact identities. Two recovery routes are handled:

- Hisuian Heavy Ball can recover a prized Basic Pokémon and replaces that Prize with Heavy Ball.
- Gladion recovers one selected Prize and shuffles Gladion into the remaining Prizes.

This captures the central prizing asymmetry: a recovery card can itself be prized, and a one-use recovery resource can only solve one critical card at a time.

## Sampling and comparison method

The program uses a fixed 64-bit Mersenne Twister seed. Baseline scenarios use stable derived seeds. Variant rows use matched seeds for each scenario so percentage deltas are less noisy than independent-run deltas.

For a binary success count `x` over `n` trials, the displayed Monte Carlo standard error in percentage points is:

```text
100 * sqrt((x / n) * (1 - x / n) / n)
```

A point difference smaller than roughly two combined standard errors should be treated as inconclusive. A large speed gain can still be strategically unacceptable when it removes a high-discrete-value answer such as Mawile-GX, Guzma, or Field Blower.

## Windows build and result-write behavior

The project is designed for Windows 11 CMake builds. The C++ result writer uses an exclusive lock file and an atomic temporary-file replacement. The audit script uses the equivalent Windows lock path through `msvcrt`, with a POSIX fallback for the current automated verification environment.
