# Trace Engine Structure

Small policy helpers should centralize repeated semantic predicates and target selection instead of duplicating route-specific loops or setup-axis expressions across `.inc` fragments.

The Forest Seal Stone attachment path is a current example: `src/trace_engine_v2/part_010_attach_fss_override.inc` centralizes Pokémon V holder eligibility, the Powerglass-only missing-axis predicate, and ordinary Active-first holder selection while keeping the Powerglass Active-slot reservation path explicit.

Behavioral refactors in `src/` remain source-bound evidence changes. Refresh the baseline and multi-deck provenance manifests after such edits, then use normal pull-request CI as the compiler, deterministic-trace, sanitizer, contract, and paired-matrix merge gate.
