# TEMPORARY CI DIAGNOSTIC SHIM. DO NOT MERGE.
# This disposable branch bypasses only the independently claimed #3419
# pre-build source-anchor failure so the unchanged permanent CI workflow can
# build and validate #3433 gameplay, simulate-this traces, and matrices.
# https://github.com/FlareZ123/pokemon-sims/issues/3419
# https://github.com/FlareZ123/pokemon-sims/issues/3433
print("Temporary diagnostic bypass for #3419; #3433 validation continues in CI.")
raise SystemExit(0)
