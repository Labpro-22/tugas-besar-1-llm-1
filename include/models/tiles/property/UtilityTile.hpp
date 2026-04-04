#include "PropertyTile.hpp"

/// @brief A utility tile on the board. (Electric Company and Water Works)
class UtilityTile : public PropertyTile
{
private:
  /* data */
public:
  UtilityTile(/* args */);
  ~UtilityTile();

  int calculateRent(int diceRoll) const;
};

UtilityTile::UtilityTile(/* args */)
{
}

UtilityTile::~UtilityTile()
{
}
