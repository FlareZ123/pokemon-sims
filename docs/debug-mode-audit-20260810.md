# Debug-mode CI audit, 2026-08-10

This temporary PR-only marker triggers source-bound CI from current `main` for repository-wide debug review. It does not change simulator policy or gameplay behavior.

The audit reviews the permanent `--simulate-this` traces against repository policy, the top-level advanced manual, and authoritative card data before deciding whether a new bug exists.

- Advanced manual: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
- Decision policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md
- Model assumptions: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md
