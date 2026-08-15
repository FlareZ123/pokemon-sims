// Scoped #3199 public-projection depth guard. Keep this include at the historical
// Tate held-route wrapper boundary so declaration and destruction order stay fixed.
// Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
// Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
// Future-card-oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
// Confirmed semantic bug: https://github.com/FlareZ123/pokemon-sims/issues/3199
  struct PublicProjectionDepthGuard {
    int& depth;

    explicit PublicProjectionDepthGuard(int& value) : depth(value) { ++depth; }
    ~PublicProjectionDepthGuard() { --depth; }

    PublicProjectionDepthGuard(const PublicProjectionDepthGuard&) = delete;
    PublicProjectionDepthGuard& operator=(const PublicProjectionDepthGuard&) = delete;
  };
