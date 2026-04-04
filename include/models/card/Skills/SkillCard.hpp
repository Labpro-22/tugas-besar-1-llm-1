#include "Card.hpp"

/// @brief A card that can be used by the player to gain an advantage in game. 
class SkillCard : public Card
{
private:
  
public:
  SkillCard(/* args */);
  ~SkillCard();
  virtual void executeAction(Player &player) override;
  bool usable(Player &player);
};

SkillCard::SkillCard(/* args */)
{
}

SkillCard::~SkillCard()
{
}
