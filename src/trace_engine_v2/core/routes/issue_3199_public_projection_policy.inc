// Central #3199 guard for model-internal public-information projections.
// Hidden future deck order cannot make Legacy Star actionable inside these scopes.
// Future-card-oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
// Confirmed semantic boundary: https://github.com/FlareZ123/pokemon-sims/issues/3199
class Issue3199PublicProjectionPolicy final {
 public:
  class Scope final {
   public:
    Scope() { ++Issue3199PublicProjectionPolicy::depth_; }
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
    ~Scope() { --Issue3199PublicProjectionPolicy::depth_; }
  };

  [[nodiscard]] static bool active() { return depth_ > 0; }

 private:
  inline static thread_local int depth_{0};
};
