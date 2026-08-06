// Execute the five sourced issue-2103 fallback regressions that the original
// case defines but does not call. The unified generator emits test cases in
// lexicographic filename order, so the issue-2103 case namespace is defined
// before this registration case:
// https://github.com/FlareZ123/pokemon-sims/blob/main/tests/generate_unified_tests.py
// Existing card, rulebook, K1, DCI, and policy sources remain beside each
// assertion in the original regression source:
// https://github.com/FlareZ123/pokemon-sims/blob/main/tests/issue_2103_prize_k1_tapu_gladion_prize_fallback_tests.cpp
// Confirmed test-registration bug: https://github.com/FlareZ123/pokemon-sims/issues/2172

#include <exception>
#include <iostream>

namespace sim {
struct EngineTestAccess {};
}  // namespace sim

int main() {
  try {
    ::case_issue_2103_prize_k1_tapu_gladion_prize_fallback_tests::
        test_quick_ball_fallback_uses_tapu_for_prized_regidrago_v();
    ::case_issue_2103_prize_k1_tapu_gladion_prize_fallback_tests::
        test_mysterious_treasure_fallback_uses_tapu_for_prized_vstar();
    ::case_issue_2103_prize_k1_tapu_gladion_prize_fallback_tests::
        test_wonder_tag_fetches_tate_for_the_only_missing_active_axis();
    ::case_issue_2103_prize_k1_tapu_gladion_prize_fallback_tests::
        test_gladion_recovers_prized_payload_for_earthen_vessel();
    ::case_issue_2103_prize_k1_tapu_gladion_prize_fallback_tests::
        test_gladion_recovers_prized_payload_for_quick_ball();
    std::cout << "Prize fallback registration tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
