  static bool use_fss(Engine& engine) { return engine.use_fss(); }
  static bool bench_from_hand(Engine& engine, Card card, bool entry) { return engine.bench_from_hand(card, entry); }
  // The preceding split fragment already exposes bench_tapu_if_useful:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/tests/tier2_parts/part_000a1.inc#L21-L24
  static bool play_earthen_vessel(Engine& engine, bool payload) { return engine.play_earthen_vessel(payload); }
  static bool play_brilliant_blender(Engine& engine) { return engine.play_brilliant_blender(); }
  static bool evolve_best_regi(Engine& engine) { return engine.evolve_best_regi(); }
};
}

namespace {
using namespace sim;

struct Fixture {
  DeckRecipe recipe{baseline_recipe()};
  Scenario scenario;
  std::mt19937_64 rng{7331};
  TraceLog trace{true, {}};
  Engine engine;
  Fixture(Scenario sc, State s) : scenario(std::move(sc)), engine(scenario, recipe, rng, &trace) {
    EngineTestAccess::set_state(engine, std::move(s));
  }
};