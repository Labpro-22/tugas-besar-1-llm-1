#include "SkillCard.hpp"

/// @brief A card that allows the player to move a randomized number of steps in any direction.
class MoveCard : public SkillCard
{
private:
  int steps;
public:
  MoveCard(/* args */);
  ~MoveCard();
};

MoveCard::MoveCard(/* args */)
{
}

MoveCard::~MoveCard()
{
}
