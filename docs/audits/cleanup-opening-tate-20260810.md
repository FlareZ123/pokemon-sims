# Opening/Tate cleanup CI audit

Validation-only marker for `main@6e37a86922a8f1014d645e52a100ebfc5de21622`.

The source changes under validation are the four direct-main cleanup commits that inline the one-use opening Bench alias wrapper, remove that wrapper, make the opening composition own the adjacent legacy Tate composition, and remove the redundant root include.

This marker changes no simulator, rules, policy, deck, test, or generated-result logic. Do not merge this validation-only pull request.
