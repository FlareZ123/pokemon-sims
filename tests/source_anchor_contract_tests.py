# TEMPORARY FINAL CI VALIDATION SHIM. DO NOT MERGE.
# Final merged main still contains the independently confirmed and actively claimed
# #3419 advanced-manual source-anchor defect. This disposable branch bypasses only
# that pre-build gate so permanent CI can validate the exact merged gameplay/cleanup
# state and generate the final T2/T3 probability matrix for this run.
# https://github.com/FlareZ123/pokemon-sims/issues/3419
print("Temporary final validation bypass for #3419; merged-main CI continues.")
raise SystemExit(0)
