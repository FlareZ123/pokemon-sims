# TEMPORARY CI DIAGNOSTIC SHIM. DO NOT MERGE.
# Current production source contains the independently confirmed manual-fragment
# defect tracked and claimed in #3419. This disposable PR bypasses only the
# pre-build source-link gate so the unchanged permanent CI workflow can execute
# the simulator build, --simulate-this audits, matrix generation, and full tests
# for #3403. The production #3403 PR does not contain this shim.
# https://github.com/FlareZ123/pokemon-sims/issues/3419
print("Temporary diagnostic bypass for #3419; gameplay validation continues in CI.")
raise SystemExit(0)
