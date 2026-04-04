#include "SkillCard.hpp"

/// @brief A card that protects the player from rent and other negative effects for 1 turn.
class ShieldCard : public SkillCard
{
private:
  int duration;
public:
  ShieldCard(/* args */);
  ~ShieldCard();
};

ShieldCard::ShieldCard(/* args */)
{
}

ShieldCard::~ShieldCard()
{
}
