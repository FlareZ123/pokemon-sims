void zero_target_crispin_still_establishes_knowledge() {
  State s = st(2, Card::RegidragoV, 1, 1, 0);
  s.hand = {Card::Crispin};
  s.prizes = {Card::RegidragoVstar, Card::Grass, Card::Fire};
  Fixture f(sc("tier2-zero-target"), s);
  expect(EngineTestAccess::play_crispin(f.engine), "Crispin should resolve even with no Energy in deck.");
  expect(EngineTestAccess::prizes_known(f.engine), "A zero-target deck search should still establish K1.");
}

using Test = std::pair<const char*, std::function<void()>>;
}

int main() {
  const std::vector<Test> tests{
    {"crispin_beats_arven", crispin_beats_arven},
    {"steven_beats_arven", steven_beats_arven},
    {"search_then_known_gladion_beats_supporters", search_then_known_gladion_beats_supporters},
    {"k0_does_not_peek_prizes", k0_does_not_peek_prizes},
    {"blind_gladion_is_critical_only", blind_gladion_is_critical_only},
    {"fss_uses_oricorio_compression", fss_uses_oricorio_compression},
    {"fss_uses_vessel_over_arven", fss_uses_vessel_over_arven},
    {"fss_uses_direct_arven_not_tapu_chain", fss_uses_direct_arven_not_tapu_chain},
    {"fss_uses_blender_for_payload", fss_uses_blender_for_payload},
    {"known_lusamine_is_delayed_not_direct", known_lusamine_is_delayed_not_direct},
    {"search_effects_persist_knowledge", search_effects_persist_knowledge},
    {"zero_target_crispin_still_establishes_knowledge", zero_target_crispin_still_establishes_knowledge},
  };
  int failures = 0;
  for (const auto& [name, run] : tests) {
    try { run(); std::cout << "PASS " << name << '\n'; }
    catch (const std::exception& e) { ++failures; std::cerr << "FAIL " << name << ": " << e.what() << '\n'; }
  }
  std::cout << "Tier Two policy tests: " << (tests.size() - static_cast<std::size_t>(failures)) << '/' << tests.size() << " passed\n";
  return failures == 0 ? 0 : 1;
}
