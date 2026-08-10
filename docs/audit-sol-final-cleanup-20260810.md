# Final cleanup current-main audit

Behavior-neutral CI trigger for `main@36cf37a9ce0cde793004ba9663448bbb78a61d40` after the composition centralization and root composition-map cleanup.

Expected result: Release and sanitizer builds compile cleanly, permanent `--simulate-this` audits remain unchanged, `setup_docs_generator_contract` returns to its pre-cleanup state, and only the already-owned stale generated-result provenance contracts tracked by #2725 / PR #2822 remain red. Close this PR without merge after evidence inspection.
