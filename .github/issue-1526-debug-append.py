from pathlib import Path

path = Path("tests/issue_1526_tate_dark_asset_tests.cpp")
source = path.read_text(encoding="utf-8")
needle = """  static const State& state(const Engine& engine) {
    return engine.state_;
  }
"""
replacement = """  static const State& state(const Engine& engine) {
    return engine.state_;
  }

  static void print_tate_projection(const Engine& engine) {
    std::mt19937_64 shadow_rng = engine.rng_;
    TraceLog trace;
    trace.enabled = true;
    Engine projected(engine.scenario_, engine.recipe_, shadow_rng, &trace);
    projected.state_ = engine.state_;
    projected.state_.dark_asset_used = true;
    if (engine.deck_seen_ || engine.prizes_revealed_) {
      projected.deck_seen_ = engine.deck_seen_;
      projected.prizes_revealed_ = engine.prizes_revealed_;
    } else {
      projected.state_.deck = engine.tate_k0_public_projection_deck();
      projected.state_.prizes.clear();
      projected.deck_seen_ = false;
      projected.prizes_revealed_ = false;
    }
    while (Engine::remove_one(projected.state_.hand, Card::TateLiza)) {
    }
    projected.state_.supporter_used = true;
    projected.run_turn();
    std::cerr << "projection=" << projected.state_line()
              << " active_vstar=" << projected.active_is_vstar()
              << " payload_ready=" << projected.payload_ready()
              << " legacy=" << projected.outcome_.used_legacy_star << '\\n';
    for (const std::string& line : trace.lines) std::cerr << line << '\\n';
  }
"""
if source.count(needle) != 1:
    raise SystemExit("EngineTestAccess insertion point missing")
source = source.replace(needle, replacement, 1)
needle = """  expect(sim::EngineTestAccess::tate_route_completes(engine),
         \"Public held VSTAR completion was suppressed by an unused Crobat V\");
"""
replacement = """  sim::EngineTestAccess::print_tate_projection(engine);
  expect(sim::EngineTestAccess::tate_route_completes(engine),
         \"Public held VSTAR completion was suppressed by an unused Crobat V\");
"""
if source.count(needle) != 1:
    raise SystemExit("Public completion assertion missing")
path.write_text(source.replace(needle, replacement, 1), encoding="utf-8")
