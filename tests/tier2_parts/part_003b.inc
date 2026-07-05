void zero_target_crispin_still_establishes_knowledge() {
  State s = st(2, Card::RegidragoV, 1, 1, 0);
  s.hand = {Card::Crispin};
  s.prizes = {Card::RegidragoVstar, Card::Grass, Card::Fire};
  Fixture f(sc("tier2-zero-target"), s);
  expect(EngineTestAccess::play_crispin(f.engine), "Crispin should resolve even with no Energy in deck.");
  expect(EngineTestAccess::prizes_known(f.engine), "A zero-target deck search should still establish K1.");
}

void k1_singleton_crispin_supplies_manual_energy() {
  State s = st(2, Card::RegidragoV, 1, 1, 0);
  s.hand = {Card::MysteriousTreasure, Card::MegaDragonite, Card::Crispin, Card::RegidragoVstar};
  s.deck = {Card::TapuLeleGX, Card::Fire};
  Fixture f(sc("tier2-k1-singleton-crispin"), s);
  expect(EngineTestAccess::play_mysterious_treasure(f.engine, true), "Mysterious Treasure should legally inspect the deck.");
  expect(EngineTestAccess::prizes_known(f.engine), "The legal search should establish K1.");
  expect(EngineTestAccess::play_crispin(f.engine), "Known singleton Fire should still make Crispin productive.");
  State& a = EngineTestAccess::state(f.engine);
  expect(a.active->fire == 0 && has(a.hand, Card::Fire), "One-card Crispin search puts Fire into hand and attaches none.");
  expect(EngineTestAccess::attach_manual(f.engine), "The unused manual attachment should take the singleton Fire.");
  expect(EngineTestAccess::state(f.engine).active->fire == 1, "Manual attachment should complete the missing Fire component.");
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
    {"k1_singleton_crispin_supplies_manual_energy", k1_singleton_crispin_supplies_manual_energy},
    {"tapu_avoids_redundant_arven", tapu_avoids_redundant_arven},
    {"tapu_uses_gladion_for_known_prized_vstar", tapu_uses_gladion_for_known_prized_vstar},
    {"crispin_beats_known_prized_oricorio", crispin_beats_known_prized_oricorio},
    {"fss_holds_for_tate_switch", fss_holds_for_tate_switch},
    {"fss_uses_gladion_for_known_prized_vstar", fss_uses_gladion_for_known_prized_vstar},
    {"heavy_ball_oricorio_compresses_energy", heavy_ball_oricorio_compresses_energy},
  };
  int failures = 0;
  for (const auto& [name, run] : tests) {
    try { run(); std::cout << "PASS " << name << '\n'; }
    catch (const std::exception& e) { ++failures; std::cerr << "FAIL " << name << ": " << e.what() << '\n'; }
  }
  std::cout << "Tier Two policy tests: " << (tests.size() - static_cast<std::size_t>(failures)) << '/' << tests.size() << " passed\n";
  return failures == 0 ? 0 : 1;
}
