# Double Dragon Energy validation

Issue: https://github.com/FlareZ123/pokemon-sims/issues/2238

Double Dragon Energy source: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/

Regidrago VSTAR / Apex Dragon source: https://api.pokemontcg.io/v2/cards/swsh12-136

Regidrago V / Celestial Roar source: https://api.pokemontcg.io/v2/cards/swsh12-135

Core rules: https://www.pokemon.com/us/pokemon-tcg/rules

The validation-only `regidrago-dde-model` recipe replaces one Grass Energy and one Fire Energy in the canonical shell with two Double Dragon Energy. It is intentionally not registered in `deck_registry()` and therefore does not alter `--all-decks` or the canonical shell/Pineco comparison.

## Five `--simulate-this` audits

These deterministic full-game traces were replayed on the final validated binary and manually inspected around every DDE-relevant decision.

| Scenario | Seed | DDE decision | Audit result |
|---|---:|---|---|
| `strict-jit/go-first` | 3 | Manual DDE compression | DDE is strictly faster than either Basic Energy from the zero-Energy Regidrago line; DDE plus the next Fire reaches GGF one attachment window earlier than a Basic-only sequence. |
| `strict-jit/go-first` | 4 | Preserve DDE on an equal finish | Regidrago VSTAR already has GG. Fire and DDE both finish GGF immediately, so the engine attaches Fire and preserves the more flexible two-unit DDE. |
| `strict-jit/go-second` | 22 | Celestial Roar attaches DDE | Celestial Roar processes DDE among its top three and legally attaches it to Dragon-type Regidrago V. The resulting energy acceleration contributes to T2 readiness. |
| `strict-jit/go-first` | 90 | Star Alchemy searches DDE | No DDE is already held. Star Alchemy's unrestricted search takes DDE because it gives two flexible units and shortens the Energy route versus a single Basic Energy. |
| `matchup-flex-jit/go-first` | 11 | Manual DDE compression | DDE on T1 plus Grass on T2 completes GGF; a Basic-only T1 attachment would leave another Energy attachment outstanding. Legacy Star is then used only for the current-turn payload axis. |

A separate focused regression exercises the active Legacy Star override directly and requires it to recover DDE when DDE is the strictly best legal Energy recovery. Another focused regression requires Star Alchemy to preserve its one-use VSTAR Power when a held DDE already completes the sole missing Energy axis.

## 100,000-trial DDE model matrix

Matrix seed: `20260807`.

| Scenario | Ready by T2 | Ready by T3 |
|---|---:|---:|
| `strict-jit/go-first` | 17.572% | 42.218% |
| `matchup-flex-jit/go-first` | 21.064% | 49.865% |
| `no-discard-control/go-first` | 23.218% | 57.419% |
| `strict-jit/go-second` | 31.877% | 54.317% |
| `matchup-flex-jit/go-second` | 38.879% | 62.127% |
| `no-discard-control/go-second` | 41.715% | 67.651% |

The complete 16-scenario DDE matrix is validated in CI. Canonical registered-deck simulation CSVs remain numerically unchanged; only source-provenance digests are refreshed.
