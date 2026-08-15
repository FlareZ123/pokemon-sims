#include "part_issue_1724_crobat_stadium_compression.inc"
#include "core/routes/issue_1673_secret_box_payload_deadline_policy.inc"

  void run_turn_issue1674_original() {
    run_turn_issue1673_original();
    if (complete_issue_1673_secret_box_payload_deadline()) {
      trace("POLICY", "P-AXIS-01; R-SECRET-BOX-01",
            "Completed the deadline Secret Box payload route: " + state_line());
    }
    complete_issue_1724_crobat_stadium_compression();
  }

#include "part_issue_1674_turo_late_promotion_override.inc"
