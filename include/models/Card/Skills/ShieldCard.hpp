#pragma once
#include "SkillCard.hpp"

/// @brief A card that protects the player from rent and other negative effects for 1 turn.
class ShieldCard : public SkillCard
{
private:
  /// @brief The duration of the shield effect in turns.
  int duration;
public:
  /// @brief Creates a shield card with the given configuration.
  /// @param name The name of the shield card.
  /// @param description The description of the shield card's effect.
  /// @param duration The number of turns the shield remains active.
  ShieldCard(const std::string &name, const std::string &description, int duration);
  ~ShieldCard();
};

ShieldCard::ShieldCard(const std::string &name, const std::string &description, int duration)
    : SkillCard(name, description), duration(duration)
{
}

ShieldCard::~ShieldCard()
{
}

