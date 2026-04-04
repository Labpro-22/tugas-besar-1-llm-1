#include "PropertyTile.hpp"
#include <vector>

/// @brief A street tile on the board. (Real estate)
class StreetTile : public PropertyTile
{
private:
  std::string color;

  int tilePrice;
  int level;
  int housePrice;
  int hotelPrice;
  std::vector<int> rentPrices;

  int festivalMult = 1;
  int festivalDur = 0;

public:
  StreetTile(/* args */);
  ~StreetTile();

  int calculateRent() const;
};

StreetTile::StreetTile(/* args */) : PropertyTile(/* args */)
{
}

StreetTile::~StreetTile()
{
}