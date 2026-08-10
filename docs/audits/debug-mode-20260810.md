# Debug-mode CI audit

This file is a behavior-neutral CI trigger for the August 10, 2026 repository-wide debug review.

Base main commit: `b543bdaeacba7d096c0efb8844c836061262f671`.

The audit uses the permanent pull-request CI workflow to inspect independent `--simulate-this` traces and the fixed-seed 100,000-trial setup matrices. No simulator, policy, card, deck, or test behavior is changed by this marker.
