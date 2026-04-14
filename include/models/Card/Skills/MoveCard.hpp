#pragma once
#include "SkillCard.hpp"

/// @brief A card that allows the player to move a randomized number of steps in any direction.
class MoveCard : public SkillCard
{
private:
  /// @brief The number of steps the player can move when using this card.
  int steps;
public:
  /// @brief Creates a move card with the given configuration.
  /// @param name The name of the move card.
  /// @param description The description of the move card's effect.
  /// @param steps The number of steps the player can move when the card is used.
  MoveCard(const std::string &name, const std::string &description, int steps);
  ~MoveCard();
};

MoveCard::MoveCard(const std::string &name, const std::string &description, int steps)
    : SkillCard(name, description), steps(steps)
{
}

MoveCard::~MoveCard()
{
}

