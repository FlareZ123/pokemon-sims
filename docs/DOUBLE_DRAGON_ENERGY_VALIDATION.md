# Double Dragon Energy validation

Issue: https://github.com/FlareZ123/pokemon-sims/issues/2238

Double Dragon Energy source: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/

Regidrago VSTAR / Apex Dragon source: https://api.pokemontcg.io/v2/cards/swsh12-136

Regidrago V / Celestial Roar source: https://api.pokemontcg.io/v2/cards/swsh12-135

Core rules: https://www.pokemon.com/us/pokemon-tcg/rules

The validation-only `regidrago-dde-model` recipe replaces one Grass Energy and one Fire Energy in the canonical shell with two Double Dragon Energy. It is intentionally not registered in `deck_registry()` and therefore does not alter `--all-decks` or the canonical shell/Pineco comparison.

The final implementation was rebased onto current `main` at `c112d9e7971d53c84dc664eb83b3732388031f54`. Rebased validation run `31156804077` built the current-main tree, passed the focused DDE and compatibility regressions, generated the DDE matrix, regenerated both canonical provenance surfaces, passed the complete Release suite, and produced the validation artifact used for the replay audit below.

## Five `--simulate-this` audits

These deterministic full-game traces were replayed on the exact current-main validated binary and manually inspected around every DDE-relevant decision. "Optimal" follows the repository policy definition: best legal action among modeled routes and player-known state, without using future draw order as an oracle.

| Scenario | Seed | DDE decision | Audit result |
|---|---:|---|---|
| `strict-jit/go-first` | 3 | Manual DDE compression | DDE on T3 plus Fire on T4 reaches GGF in two attachment windows. The visible Basic-only sequence would require a third attachment window; choosing DDE before Fire is at least tied with the reverse order and preserves the earliest T4 Energy finish. |
| `strict-jit/go-first` | 4 | Preserve DDE on an equal finish | Regidrago VSTAR already has GG. Fire and DDE both finish GGF immediately, so the engine attaches Fire and preserves the more flexible two-unit DDE. |
| `strict-jit/go-second` | 22 | Celestial Roar attaches DDE | Celestial Roar processes DDE among its top three and legally attaches it to Dragon-type Regidrago V. The attachment is required by the modeled attack resolution and contributes to T2 readiness. |
| `strict-jit/go-first` | 90 | Star Alchemy searches DDE | At T1 K0 no future draws may be assumed. Star Alchemy legally establishes K1; inspection proves Crispin and Brilliant Blender are prized while a DDE remains in deck. DDE is therefore the shortest deterministic Energy connector, and DDE plus the next Grass completes the Energy axis on T2. |
| `matchup-flex-jit/go-first` | 11 | Manual DDE compression | DDE on T1 plus Grass on T2 completes GGF; a Basic-only line needs another attachment. Legacy Star is then spent only to establish the current-turn payload axis and does not waste DDE recovery. |

Focused regressions additionally require: DDE + Grass, DDE + Fire, and two DDE to pay GGF; one DDE alone not to pay a three-Energy attack; Basic-only searches to exclude DDE; Star Alchemy to choose DDE when it is strictly faster; Star Alchemy to preserve its VSTAR Power when held DDE already solves the sole missing Energy axis; Celestial Roar to attach DDE legally; and the active Legacy Star override to recover DDE when it is the strictly best legal Energy recovery.

## 100,000-trial DDE model matrix

Matrix seed: `20260807`. The current-main scenario registry contains 14 aggregate scenarios after retirement of the obsolete full-turn-one Item-lock rows.

| Scenario | Ready by T2 | Ready by T3 |
|---|---:|---:|
| `strict-jit/go-first` | 17.450% | 42.855% |
| `matchup-flex-jit/go-first` | 21.023% | 50.583% |
| `no-discard-control/go-first` | 23.218% | 57.419% |
| `strict-jit/go-second` | 31.897% | 54.514% |
| `matchup-flex-jit/go-second` | 38.780% | 62.339% |
| `no-discard-control/go-second` | 41.715% | 67.651% |

The complete 14-scenario DDE matrix passed in the rebased validation run. Canonical registered-deck simulation CSVs remain numerically unchanged by the DDE model; only simulator-source provenance digests are refreshed.
