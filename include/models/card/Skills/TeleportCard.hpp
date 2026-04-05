#include "SkillCard.hpp"

/// @brief A card that allows the player to teleport to any location on the board.
class TeleportCard : public SkillCard
{
private:
  /* data */
public:
  TeleportCard(/* args */);
  ~TeleportCard();
};

TeleportCard::TeleportCard(/* args */) : SkillCard(/* args */)
{
}

TeleportCard::~TeleportCard()
{
}
