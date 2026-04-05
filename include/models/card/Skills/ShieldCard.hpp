#include "SkillCard.hpp"

/// @brief A card that protects the player from rent and other negative effects for 1 turn.
class ShieldCard : public SkillCard
{
private:
  /// @brief The duration of the shield effect in turns.
  int duration;
public:
  ShieldCard(/* args */);
  ~ShieldCard();
};

ShieldCard::ShieldCard(/* args */) : SkillCard(/* args */)
{
}

ShieldCard::~ShieldCard()
{
}
