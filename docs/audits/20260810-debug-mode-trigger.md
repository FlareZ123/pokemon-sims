# Debug-mode CI trigger

This branch exists only to run the repository's pull-request CI from the reviewed current-main snapshot `bc835d973d72801f10fae46bb593c6419d469311`.

The CI workflow itself runs the permanent `--simulate-this` audits, canonical 100,000-trial T2/T3 matrix, paired matrix, Release suite, and sanitizer suite. No gameplay logic is changed by this audit trigger.
