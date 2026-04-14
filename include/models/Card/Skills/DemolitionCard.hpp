#pragma once
#include "SkillCard.hpp"

/// @brief A card that allows the player to destroy an opponent's property in the game.
class DemolitionCard : public SkillCard
{
private:
  /* data */
public:
  /// @brief Creates a demolition card with the given name and description.
  /// @param name The name of the demolition card.
  /// @param description The description of the demolition card's effect.
  DemolitionCard(const std::string &name, const std::string &description);
  ~DemolitionCard();
};

DemolitionCard::DemolitionCard(const std::string &name, const std::string &description) : SkillCard(name, description)
{
}

DemolitionCard::~DemolitionCard()
{
}

