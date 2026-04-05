#include "PropertyTile.hpp"

/// @brief A utility tile on the board. (Electric Company and Water Works)
class UtilityTile : public PropertyTile
{
private:
  /* data */
public:
  UtilityTile(/* args */);
  ~UtilityTile();

  /// @brief Calculates the rent for the utility tile based on the dice roll.
  /// @param diceRoll The sum of the dice roll.
  /// @return The amount of rent to be paid.
  int calculateRent(int diceRoll) const;
};

UtilityTile::UtilityTile(/* args */) : PropertyTile(/* args */)
{
}

UtilityTile::~UtilityTile()
{
}
