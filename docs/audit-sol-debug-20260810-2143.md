# Source-neutral debug audit

This branch is current `main@94189f0751f25ede814d9028932daae856c28998` plus this inert documentation marker.

Purpose: run the repository pull-request CI against the current simulator, inspect at least three independent `--simulate-this` traces for earliest legal and resource-realistic play, run the full Release/sanitizer suite, and capture the source-bound T2/T3 setup matrices. This marker does not alter simulator behavior and must not be merged.
