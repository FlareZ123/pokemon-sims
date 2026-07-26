  bool issue_1516_quick_ball_tapu_duplicate_crispin_is_redundant(
      const bool permit_payload) const {
    if (!deck_seen_ || !prizes_known() ||
        scenario_.dci != DciProfile::NoDiscardControl ||
        hand_count(Card::QuickBall) == 0 ||
        hand_count(Card::Crispin) == 0 ||
        hand_count(Card::Gladion) == 0 ||
        !need_energy()) {
      return false;
    }

    constexpr std::array<Card, 5> kAcceptedPayloads{
        Card::Dragapult, Card::MegaDragonite, Card::DialgaGX,
        Card::GoodraVstar, Card::Appletun};
    const bool known_prized_payload = std::any_of(
        kAcceptedPayloads.begin(), kAcceptedPayloads.end(),
        [this](const Card card) {
          return prize_count_after_reveal(card) > 0;
        });
    if (!known_prized_payload) return false;

    std::mt19937_64 projected_rng = rng_;
    Engine projected(scenario_, recipe_, projected_rng);
    projected.state_ = state_;
    projected.deck_seen_ = deck_seen_;
    projected.prizes_revealed_ = prizes_revealed_;
    const int tapu_before = projected.hand_count(Card::TapuLeleGX);
    if (!projected.play_quick_ball_issue1516_original(permit_payload) ||
        projected.hand_count(Card::TapuLeleGX) <= tapu_before) {
      return false;
    }

    const bool existing_no_marginal_route =
        projected.wonder_tag_duplicate_held_crispin_has_no_marginal_route();
    const int crispin_before = projected.hand_count(Card::Crispin);
    const bool benched_tapu = projected.bench_tapu_if_useful();
    const bool selected_duplicate_crispin = benched_tapu &&
        projected.hand_count(Card::Crispin) > crispin_before;

    // Quick Ball pays one discard and searches one Basic Pokemon. The projection
    // executes that exact final selector and then the actual Wonder Tag selector
    // before suppressing the real play. When it selects a duplicate held Crispin
    // while held Gladion covers a known no-control payload Prize, the searched
    // Supporter adds no earlier setup axis and every physical resource is preserved:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // One Supporter per turn and Item cost procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and resource-preserving route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Prior duplicate-Crispin guard: https://github.com/FlareZ123/pokemon-sims/issues/1038
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1516
    return selected_duplicate_crispin &&
        (existing_no_marginal_route || known_prized_payload);
  }

  bool play_quick_ball(const bool permit_payload) {
    if (issue_1516_quick_ball_tapu_duplicate_crispin_is_redundant(
            permit_payload)) {
      if (trace_ != nullptr) {
        trace_->add_policy_once(
            state_, "issue-1516-qb-tapu-duplicate-crispin", state_.turn,
            "HOLD QUICK BALL",
            "R-QB-01; R-TAPU-01; R-CRISPIN-01; R-GLADION-01; R-GAME-SUPPORTER; P-CONNECTOR-01; P-KNOWLEDGE-01",
            "Preserved Quick Ball, its discard, Tapu Lele-GX, Wonder Tag, and the Bench slot because held Gladion covers the known no-control payload Prize and held Crispin covers the later Energy turn.");
      }
      return false;
    }
    return play_quick_ball_issue1516_original(permit_payload);
  }
