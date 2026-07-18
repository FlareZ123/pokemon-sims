# T5 Diagnostic Horizon and Setup Failure

## Decision

Standard aggregate scenarios simulate unresolved games through turn 5.

- A game ready by the end of T4 is a setup success.
- A game not ready by the end of T4 is a setup loss.
- A game that first becomes ready on T5 is recorded as a diagnostic late recovery and remains a setup loss.
- A game still unresolved after T5, including a terminal start-of-turn deck-out, is a setup loss.

The engine stops early when readiness occurs during T2 through T4. It reaches T5 only when the T4 success deadline was missed.

## Aggregate fields

The CSV distinguishes the deadline result from the diagnostic continuation:

| Field | Meaning |
|---|---|
| `ready_by_t4_pct` | Setup-success percentage. |
| `ready_by_t5_pct` | Cumulative readiness through the diagnostic T5 continuation. |
| `ready_on_t5_pct` | Games whose first ready turn was exactly T5. These remain failures. |
| `setup_failure_pct` | Every game not ready by the end of T4, equal to `100 - ready_by_t4_pct`. |

The corresponding standard-error columns report Monte Carlo sampling error in percentage points.

## Interpretation

`setup_failure_pct` is a simulator benchmark for unacceptable setup speed. It is not automatically an official Pokémon TCG match loss. A game-rule loss, such as failing the required start-of-turn draw, remains separately represented by the applicable game rule.

The policy identifier `P-T5-FAIL-01` is registered in [`RULES_TRACEABILITY.md`](RULES_TRACEABILITY.md). The implementation is in `Engine::run`, `record_ready`, `simulate`, and the aggregate CSV writer.
