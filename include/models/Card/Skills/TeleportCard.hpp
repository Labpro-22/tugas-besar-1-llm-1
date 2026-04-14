#pragma once
#include "SkillCard.hpp"

/// @brief A card that allows the player to teleport to any location on the board.
class TeleportCard : public SkillCard
{
private:
  /* data */
public:
  /// @brief Creates a teleport card with the given name and description.
  /// @param name The name of the teleport card.
  /// @param description The description of the teleport card's effect.
  TeleportCard(const std::string &name, const std::string &description);
  ~TeleportCard();
};

TeleportCard::TeleportCard(const std::string &name, const std::string &description) : SkillCard(name, description)
{
}

TeleportCard::~TeleportCard()
{
}

