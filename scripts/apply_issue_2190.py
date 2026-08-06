from pathlib import Path

path = Path("tests/issue_1118_multi_deck_secret_box_tests.cpp")
source = path.read_text(encoding="utf-8")
old = """  const auto must_reject = [](sim::State state, const sim::LockMode lock,
                              const char* message) {
    Fixture fixture(lock, sim::DciProfile::MatchupFlexJit);
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
"""
new = """  // Keep the candidate state distinct from the outer completed-route fixture:
  // https://github.com/FlareZ123/pokemon-sims/issues/2190
  const auto must_reject = [](sim::State candidate_state,
                              const sim::LockMode lock,
                              const char* message) {
    Fixture fixture(lock, sim::DciProfile::MatchupFlexJit);
    sim::EngineTestAccess::set_state(fixture.engine,
                                     std::move(candidate_state));
"""
if source.count(old) != 1:
    raise SystemExit("issue-2190 anchor count was not exactly one")
path.write_text(source.replace(old, new, 1), encoding="utf-8")
