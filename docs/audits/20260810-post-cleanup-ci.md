# Post-cleanup CI trigger

This branch exists only to validate `main@b3c6d7e122a1b59e68aa9d6d87d7a0766a3202eb` after the three requested cleanup commits.

The pull-request CI runs the repository's permanent `--simulate-this` audits, canonical 100,000-trial T2/T3 matrix, paired matrix, Release suite, strict C++20 build, and sanitizers. No gameplay logic is changed by this trigger.
