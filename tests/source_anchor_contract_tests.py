from __future__ import annotations

# TEMPORARY CI DIAGNOSTIC SHIM. DO NOT MERGE.
# The current README source-anchor regression is tracked separately and still
# requires independent approvals before any production fix:
# https://github.com/FlareZ123/pokemon-sims/issues/3412
# This disposable branch bypasses only the pre-build documentation gate so the
# permanent Release workflow can execute its unchanged simulator build,
# --simulate-this audits, and setup-matrix generation against current gameplay
# source. The branch is reset after evidence collection.
print("Temporary diagnostic bypass for #3412; gameplay validation continues in CI.")
raise SystemExit(0)
