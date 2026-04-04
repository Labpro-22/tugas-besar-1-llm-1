#include "SkillCard.hpp"

/// @brief A card that provides a random discount on purchases for 1 turn.
class DiscountCard : public SkillCard
{
private:
  int discountPercentage;
  int duration;
public:
  DiscountCard(/* args */);
  ~DiscountCard();
};

DiscountCard::DiscountCard(/* args */)
{
}

DiscountCard::~DiscountCard()
{
}
