#include "SkillCard.hpp"

/// @brief A card that allows the player to move a randomized number of steps in any direction.
class MoveCard : public SkillCard
{
private:
  /// @brief The number of steps the player can move when using this card.
  int steps;
public:
  MoveCard(/* args */);
  ~MoveCard();
};

MoveCard::MoveCard(/* args */) : SkillCard(/* args */)
{
}

MoveCard::~MoveCard()
{
}
