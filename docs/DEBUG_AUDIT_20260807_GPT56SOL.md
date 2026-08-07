# Debug-mode current-main CI audit

This file is an audit-only CI trigger. It changes no simulator policy, card logic, deck recipe, rules interpretation, scenario semantics, action selector, or probability calculation.

Audit requirements for this branch:

- complete Release and strict C++20 validation;
- complete ASan/UBSan validation;
- repository-defined `--simulate-this` audits, with at least three independently inspected traces;
- canonical and paired 100,000-trial T2/T3 setup matrices;
- tested-head and simulator-policy-digest evidence.

The branch was created only after reviewing every currently open issue, its confirmation/claim state, and associated active PR. No active claim met the 12-hour stale threshold at audit start.

Close this PR without merge after evidence review unless the audit exposes a distinct defect.
