#include "SkillCard.hpp"

/// @brief A card that pulls the nearest opponent in front of the player towards the player.
class LassoCard : public SkillCard
{
private:
  /* data */
public:
  /// @brief Creates a lasso card with the given name and description.
  /// @param name The name of the lasso card.
  /// @param description The description of the lasso card's effect.
  LassoCard(const std::string &name, const std::string &description);
  ~LassoCard();
};

LassoCard::LassoCard(const std::string &name, const std::string &description) : SkillCard(name, description)
{
}

LassoCard::~LassoCard()
{
}
