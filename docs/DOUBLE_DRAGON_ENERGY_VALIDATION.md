# Double Dragon Energy paper-Expanded validation

Mechanics issue: https://github.com/FlareZ123/pokemon-sims/issues/2238

TCG Live card-pool scope issue: https://github.com/FlareZ123/pokemon-sims/issues/2332

Double Dragon Energy source: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/

Pokémon TCG Live card-pool statement: https://support.pokemon.com/hc/en-us/articles/6489934466708-Pok%C3%A9mon-TCG-Live-Migration-FAQ-from-the-Pok%C3%A9mon-TCG-Online

Regidrago VSTAR / Apex Dragon source: https://api.pokemontcg.io/v2/cards/swsh12-136

Regidrago V / Celestial Roar source: https://api.pokemontcg.io/v2/cards/swsh12-135

Core rules: https://www.pokemon.com/us/pokemon-tcg/rules

Double Dragon Energy is an XY—Roaring Skies card. Pokémon Support currently states that XY cards are not playable in Pokémon TCG Live. The `regidrago-dde-model` therefore exists only as a paper-Expanded mechanics model. It is intentionally absent from `deck_registry()`, `--all-decks`, and canonical Pokémon TCG Live matrices.

The paper model replaces one Grass Energy and one Fire Energy in the canonical shell with two Double Dragon Energy. CLI access requires an explicit paper-only opt-in:

```text
regidrago_sim --paper-expanded-model --deck regidrago-dde-model ...
```

A plain `--deck regidrago-dde-model` request is rejected at the Pokémon TCG Live card-pool boundary. Registered shell and Pineco recipes continue through the Live validation path.

## Historical five `--simulate-this` paper audits

These deterministic traces validate the paper-Expanded DDE mechanics retained from issue #2238. They are excluded from Pokémon TCG Live probability claims.

| Scenario | Seed | DDE decision | Audit result |
|---|---:|---|---|
| `strict-jit/go-first` | 3 | Manual DDE compression | DDE on T3 plus Fire on T4 reaches GGF in two attachment windows. The visible Basic-only sequence would require a third attachment window. |
| `strict-jit/go-first` | 4 | Preserve DDE on an equal finish | Regidrago VSTAR already has GG. Fire and DDE both finish GGF immediately, so the engine attaches Fire and preserves the more flexible two-unit DDE. |
| `strict-jit/go-second` | 22 | Celestial Roar attaches DDE | Celestial Roar processes DDE among its top three and legally attaches it to Dragon-type Regidrago V. |
| `strict-jit/go-first` | 90 | Star Alchemy searches DDE | Star Alchemy establishes K1; the trace shows Crispin and Brilliant Blender prized while DDE remains in deck, making DDE the shortest deterministic Energy connector in that paper model. |
| `matchup-flex-jit/go-first` | 11 | Manual DDE compression | DDE on T1 plus Grass on T2 completes GGF; Legacy Star is then reserved for the current-turn payload axis. |

Focused mechanics regressions retain DDE + Grass, DDE + Fire, and two-DDE GGF payment; one DDE alone failing a three-Energy attack cost; Basic-only searches excluding DDE; Star Alchemy and Legacy Star DDE routing; and Celestial Roar attachment legality.

## Historical 100,000-trial paper model matrix

Matrix seed: `20260807`. These numbers describe the explicit paper-Expanded DDE model and must not be reported as Pokémon TCG Live Expanded setup probabilities.

| Scenario | Ready by T2 | Ready by T3 |
|---|---:|---:|
| `strict-jit/go-first` | 17.500% | 43.054% |
| `matchup-flex-jit/go-first` | 21.092% | 51.000% |
| `no-discard-control/go-first` | 23.218% | 57.419% |
| `strict-jit/go-second` | 32.078% | 54.659% |
| `matchup-flex-jit/go-second` | 38.847% | 62.590% |
| `no-discard-control/go-second` | 41.715% | 67.651% |

Canonical registered-deck simulation CSVs remain Pokémon TCG Live scoped and exclude this model.
