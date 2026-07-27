# Battle VIP Pass validation

## Temporary comparison recipe

The validation recipe is derived from `regidrago-shell` by removing one Quick Ball and one Mysterious Treasure and adding two Battle VIP Pass. It exists only in the validation harness and is absent from `deck_registry()`, `--all-decks`, and the committed named recipes.

Card sources: Battle VIP Pass https://api.pokemontcg.io/v2/cards/swsh8-225, Quick Ball https://api.pokemontcg.io/v2/cards/swsh1-179, Mysterious Treasure https://api.pokemontcg.io/v2/cards/sm6-113.

## Fixed-seed matrix

| Scenario | Baseline T2 | VIP T2 | Δ T2 | Baseline T3 | VIP T3 | Δ T3 | Baseline T4 | VIP T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| `strict-jit/go-first` | 11.977% | 11.689% | -0.288 pp | 38.745% | 39.579% | +0.834 pp | 55.512% | 56.334% | +0.822 pp |
| `matchup-flex-jit/go-first` | 16.295% | 14.853% | -1.442 pp | 47.322% | 45.866% | -1.456 pp | 63.313% | 61.531% | -1.782 pp |
| `no-discard-control/go-first` | 19.986% | 17.840% | -2.146 pp | 55.868% | 54.203% | -1.665 pp | 71.928% | 70.108% | -1.820 pp |
| `strict-jit-turn2-item-lock/go-first` | 4.601% | 4.330% | -0.271 pp | 10.191% | 10.298% | +0.107 pp | 17.704% | 18.321% | +0.617 pp |
| `strict-jit-full-item-lock/go-first` | 2.825% | 2.818% | -0.007 pp | 7.752% | 7.737% | -0.015 pp | 15.070% | 15.030% | -0.040 pp |
| `strict-jit-rulebox-ability-lock/go-first` | 4.420% | 3.830% | -0.590 pp | 25.911% | 24.808% | -1.103 pp | 38.953% | 37.697% | -1.256 pp |
| `strict-jit-combined-lock/go-first` | 0.291% | 0.312% | +0.021 pp | 3.268% | 3.317% | +0.049 pp | 7.254% | 7.222% | -0.032 pp |
| `strict-jit/go-second` | 29.402% | 30.457% | +1.055 pp | 52.755% | 53.855% | +1.100 pp | 63.927% | 65.027% | +1.100 pp |
| `matchup-flex-jit/go-second` | 37.322% | 36.963% | -0.359 pp | 60.735% | 60.102% | -0.633 pp | 71.007% | 70.037% | -0.970 pp |
| `no-discard-control/go-second` | 39.926% | 38.521% | -1.405 pp | 66.904% | 66.314% | -0.590 pp | 78.044% | 76.934% | -1.110 pp |
| `strict-jit-turn2-item-lock/go-second` | 14.086% | 14.365% | +0.279 pp | 27.941% | 29.154% | +1.213 pp | 35.593% | 37.358% | +1.765 pp |
| `strict-jit-full-item-lock/go-second` | 10.531% | 10.471% | -0.060 pp | 22.928% | 22.992% | +0.064 pp | 30.088% | 30.168% | +0.080 pp |
| `strict-jit-rulebox-ability-lock/go-second` | 18.076% | 17.059% | -1.017 pp | 34.639% | 33.263% | -1.376 pp | 44.766% | 43.518% | -1.248 pp |
| `strict-jit-combined-lock/go-second` | 2.368% | 2.411% | +0.043 pp | 11.389% | 11.509% | +0.120 pp | 15.503% | 15.480% | -0.023 pp |
| `strict-jit-supporter-lock/go-first` | 0.003% | 0.004% | +0.001 pp | 15.289% | 12.735% | -2.554 pp | 21.545% | 18.438% | -3.107 pp |
| `strict-jit-supporter-lock/go-second` | 8.122% | 7.932% | -0.190 pp | 19.457% | 17.707% | -1.750 pp | 25.347% | 22.900% | -2.447 pp |

## Behavioral coverage

Focused tests cover first-turn legality, Item lock, five-Pokémon Bench capacity, at-most-two direct placements, K1, shuffle resolution, played-from-hand Ability suppression, Latias ex static Skyliner continuity, Pineco route priority, no-op preservation, and post-turn-one DCI.
