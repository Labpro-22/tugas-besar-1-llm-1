#pragma once
#include "Card.hpp"

/// @brief A card that can be used by the player to gain an advantage in game. 
class SkillCard : public Card
{
private:
  
public:
  /// @brief Creates a skill card with the given name and description.
  /// @param name The name of the skill card.
  /// @param description The description of the skill card's effect.
  SkillCard(const std::string &name, const std::string &description);
  ~SkillCard();

  /// @brief Executes the action associated with the card.
  /// @param player The player who drew the card.
  virtual void executeAction(Player &player) override;
  
  /// @brief Checks if the card can be used by the player.
  /// @param player The player who drew the card.
  /// @return True if the card is usable, false otherwise.
  bool usable(Player &player);
};

SkillCard::SkillCard(const std::string &name, const std::string &description) : Card(name, description)
{
}

SkillCard::~SkillCard()
{
}

