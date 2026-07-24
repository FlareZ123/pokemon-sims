import os
from pathlib import Path


def atomic_replace(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if text.count(old) != 1:
        raise SystemExit(f"Expected one replacement block in {path}")
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(text.replace(old, new, 1), encoding="utf-8")
    os.replace(temporary, path)


base_path = Path(
    "src/trace_engine_v2/part_issue_991_wonder_tag_burnet_legacy_star_override_base.inc"
)
atomic_replace(
    base_path,
    """    const bool known_prized_payload_axis =
        scenario_.dci == DciProfile::NoDiscardControl &&
        std::any_of(
            std::array{Card::Dragapult, Card::MegaDragonite, Card::DialgaGX,
                       Card::GoodraVstar, Card::Appletun}.begin(),
            std::array{Card::Dragapult, Card::MegaDragonite, Card::DialgaGX,
                       Card::GoodraVstar, Card::Appletun}.end(),
            [this](const Card card) {
              return prize_count_after_reveal(card) > 0;
            });
""",
    """    constexpr std::array<Card, 5> kAcceptedPayloads{
        Card::Dragapult, Card::MegaDragonite, Card::DialgaGX,
        Card::GoodraVstar, Card::Appletun};
    const bool known_prized_payload_axis =
        scenario_.dci == DciProfile::NoDiscardControl &&
        std::any_of(kAcceptedPayloads.begin(), kAcceptedPayloads.end(),
                    [this](const Card card) {
                      return prize_count_after_reveal(card) > 0;
                    });
""",
)
atomic_replace(
    base_path,
    "https://api.pokemontcg.io/v2/cards/sv6pt5-130 https://api.pokemontcg.io/v2/cards/me1-132",
    "https://api.pokemontcg.io/v2/cards/sv6-130 https://api.pokemontcg.io/v2/cards/me2pt5-152",
)

override_path = Path(
    "src/trace_engine_v2/part_issue_1516_quick_ball_tapu_crispin_override.inc"
)
atomic_replace(
    override_path,
    """    const bool known_prized_payload = std::any_of(
        std::array{Card::Dragapult, Card::MegaDragonite, Card::DialgaGX,
                   Card::GoodraVstar, Card::Appletun}.begin(),
        std::array{Card::Dragapult, Card::MegaDragonite, Card::DialgaGX,
                   Card::GoodraVstar, Card::Appletun}.end(),
        [this](const Card card) {
          return prize_count_after_reveal(card) > 0;
        });
""",
    """    constexpr std::array<Card, 5> kAcceptedPayloads{
        Card::Dragapult, Card::MegaDragonite, Card::DialgaGX,
        Card::GoodraVstar, Card::Appletun};
    const bool known_prized_payload = std::any_of(
        kAcceptedPayloads.begin(), kAcceptedPayloads.end(),
        [this](const Card card) {
          return prize_count_after_reveal(card) > 0;
        });
""",
)
