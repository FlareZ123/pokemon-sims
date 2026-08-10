# Post-cleanup CI audit

Behavior-neutral marker for pull-request CI from the cleaned `main` tree. Do not merge this marker.

Audit targets: permanent `--simulate-this` traces, Pineco traces, fixed-seed 100k T2/T3 matrix, full Release tests, strict C++20 compilation, and ASan/UBSan validation.
