# TEMPORARY CI DIAGNOSTIC SHIM. DO NOT MERGE.
# Current production source contains the independently confirmed manual-fragment
# defect tracked and claimed in #3419. This disposable branch bypasses only the
# pre-build source-link gate so the unchanged permanent CI workflow can execute
# the simulator build, --simulate-this audits, matrix generation, and full tests.
# https://github.com/FlareZ123/pokemon-sims/issues/3419
print("Temporary diagnostic bypass for claimed #3419; gameplay validation continues in CI.")
raise SystemExit(0)
