#include "SpecialTile.hpp"

/// @brief A free tile on the board where nothing happens.
class FreeTile : public SpecialTile
{
private:
  /* data */
public:
  FreeTile(/* args */);
  ~FreeTile();
};

FreeTile::FreeTile(/* args */)
{
}

FreeTile::~FreeTile()
{
}
