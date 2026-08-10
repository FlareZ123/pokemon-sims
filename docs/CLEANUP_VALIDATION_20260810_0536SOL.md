# Cleanup validation marker

This PR exists only to run pull-request CI against current `main` after the direct cleanup composition changes.

Validated cleanup commits:

- `0afa646b1f1e4599983d5f69c00c0a3cbc943da4` inlines the opening Bench alias composition.
- `14643f0b1e13cc3a1f5d607b8271d1b9fa803c77` removes the merged Bench alias wrapper.
- `0906216c4b6da8d78b3122f9c6d3e5088329a1f0` inlines the Tapu availability composition.
- `8c838cd6106c98a716d2f054aba5a41c2b02f9a0` removes the merged Tapu availability wrapper.

C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
