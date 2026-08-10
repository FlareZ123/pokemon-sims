# Post-cleanup CI audit

Behavior-neutral marker for pull-request CI from `main@cb5579985b898cf20210c6fc8dd5a8df0a05d1a4`. Do not merge this marker.

Audit targets: source-anchor validation, permanent `--simulate-this` traces, Pineco traces, fixed-seed paired 100k T2/T3 matrix, full Release tests, strict C++20 compilation, and ASan/UBSan validation. The resulting matrix artifact is also the single aggregate source for issue #2923 evidence refresh.
