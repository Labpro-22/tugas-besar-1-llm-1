#include "SkillCard.hpp"

/// @brief A card that provides a random discount on purchases for 1 turn.
class DiscountCard : public SkillCard
{
private:
  /// @brief The percentage of discount provided by the card.
  int discountPercentage;
  
  /// @brief The duration of the discount effect in turns.
  int duration;
public:
  DiscountCard(/* args */);
  ~DiscountCard();
};

DiscountCard::DiscountCard(/* args */) : SkillCard(/* args */)
{
}

DiscountCard::~DiscountCard()
{
}
