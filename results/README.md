# Simulation outputs

- `simulation_results.csv` contains the final 100,000-trial baseline scenarios.
- `variant_results.csv` contains the final 25,000-trial matched-seed swap screen.

Both files were generated with seed `20260704` after the final policy revisions documented in `SIM-PLAN.md`. The C++ writer uses a destination lock and atomic replacement. The schemas are verified by the CTest smoke test.
