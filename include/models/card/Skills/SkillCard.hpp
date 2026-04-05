#include "Card.hpp"

/// @brief A card that can be used by the player to gain an advantage in game. 
class SkillCard : public Card
{
private:
  
public:
  SkillCard(/* args */);
  ~SkillCard();

  /// @brief Executes the action associated with the card.
  /// @param player The player who drew the card.
  virtual void executeAction(Player &player) override;
  
  /// @brief Checks if the card can be used by the player.
  /// @param player The player who drew the card.
  /// @return True if the card is usable, false otherwise.
  bool usable(Player &player);
};

SkillCard::SkillCard(/* args */) : Card(/* args */)
{
}

SkillCard::~SkillCard()
{
}
