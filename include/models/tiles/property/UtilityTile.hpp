#include "PropertyTile.hpp"

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
