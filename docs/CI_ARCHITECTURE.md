# CI architecture

The default branch intentionally keeps a small, explicit workflow set. New issue fixes should normally add tests, scripts, or trace assertions to the existing validation surfaces instead of installing a new workflow.

## Permanent workflows

### `ci.yml`

The authoritative validation for pull requests targeting `main` and pushes to `main`.

It owns:

- Release configuration and build;
- strict C++20 compilation;
- permanent `--simulate-this` regression traces;
- the paired fixed-seed setup matrices and committed-evidence comparisons;
- the complete Release test suite;
- ASan/UBSan validation;
- source-bound validation artifacts.

A bug fix that will eventually merge to `main` must pass this workflow. Do not add a second full CI workflow for one issue.

### `stacked-pr-validation.yml`

A lighter validation surface for pull requests whose base is another working branch instead of `main`.

It exists so dependent changes can receive useful feedback before the stack is retargeted to `main`. It deliberately avoids the full paired matrix, sanitizer duplication, and source-bound report checks. Those become authoritative after the final PR targets `main` and runs `ci.yml`.

### `workflow-policy.yml`

The workflow hygiene guard.

It checks `docs/PERMANENT_WORKFLOWS.txt` and rejects:

- workflow files that are not in the permanent inventory;
- stale inventory entries;
- permanent workflows with `contents: write`;
- hard-coded `github.head_ref == ...` recovery logic.

The explicit inventory replaces the older `issue-*` filename convention, which could not catch temporary workflows with other names.

## Rules for new validation work

1. Prefer a CTest/unified regression for deterministic behavior.
2. Preserve durable correctness findings as normal regression tests under `tests/` so they run with the unified suite.
3. Treat A/B comparisons, exploratory analyses, and one-time experiments as temporary work. After the experiment is complete, remove its dedicated workflow and experiment-only analyzers or harnesses from `main`.
4. Add a permanent trace to `ci.yml` only when a full-game route needs source-bound behavioral coverage.
5. Use the stacked workflow only for dependent PRs. It is not a second main CI.
6. Do not commit one-off merge, materialization, recovery, audit, experiment, or branch-specific workflows to `main` after their owning work is complete.
7. Permanent workflows stay read-only. Repository mutation belongs in explicit human/agent GitHub operations rather than ambient PR validation.
8. If a genuinely new permanent workflow is needed, add it to `docs/PERMANENT_WORKFLOWS.txt` in the same reviewed PR and explain why an existing workflow cannot own the job.

GitHub references:

- Pull-request event and branch filtering: https://docs.github.com/en/actions/using-workflows/events-that-trigger-workflows#pull_request
- Workflow syntax: https://docs.github.com/en/actions/reference/workflows-and-actions/workflow-syntax
- Token permissions: https://docs.github.com/en/actions/security-for-github-actions/security-guides/automatic-token-authentication
- Reusable workflows, when a future shared job genuinely benefits from them: https://docs.github.com/en/actions/how-tos/reuse-automations/reuse-workflows

Repository history:

- Workflow-sprawl incident and cleanup contract: https://github.com/FlareZ123/pokemon-sims/issues/1360
- Stacked regression-selector scope: https://github.com/FlareZ123/pokemon-sims/issues/2152
