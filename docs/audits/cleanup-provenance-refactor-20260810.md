# Cleanup provenance refactor validation

This audit branch exists only to trigger pull-request CI after three behavior-neutral cleanup commits landed directly on `main`:

- `a4eed8b3d4617f869530a4ab9129106423ce3b0f` removes the one-line provenance manifest wrapper.
- `9782e14aeb089a7b4aa91ce2a0529e6c6d70d24b` makes `_SourceDigestBuilder` own digest finalization.
- `f021bd4e9548705061f17d94bc8b3e17648b9577` makes `_SourceDigestBuilder` own framed path hashing.

The refactor does not change the source set, stable ordering, path framing, byte framing, or SHA-256 algorithm used by `simulator_policy_source_digest()`.
