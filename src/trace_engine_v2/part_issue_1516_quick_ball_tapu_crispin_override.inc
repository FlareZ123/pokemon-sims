  bool issue_1516_quick_ball_tapu_duplicate_crispin_is_redundant(
      const bool permit_payload) const {
    // Full Prize inspection and a resolved deck search establish the same K1
    // knowledge, so this route depends on known zones rather than provenance:
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Correction precedent: https://github.com/FlareZ123/pokemon-sims/commit/690808e65feb4c17034cd3d76157ff5929a65754
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1922
    if (!prizes_known() ||
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

    const int crispin_before = projected.hand_count(Card::Crispin);
    const bool benched_tapu = projected.bench_tapu_if_useful();
    const bool selected_duplicate_crispin = benched_tapu &&
        projected.hand_count(Card::Crispin) > crispin_before;

    // Quick Ball pays one discard and searches one Basic Pokemon. This K1-only
    // projection executes the final target selector and Wonder Tag selector before
    // preserving the physical route. A duplicate held Crispin cannot advance the
    // current Supporter action while held Gladion covers the known payload Prize:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // One Supporter per turn and Item cost procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and resource-preserving route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1516
    return selected_duplicate_crispin;
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
