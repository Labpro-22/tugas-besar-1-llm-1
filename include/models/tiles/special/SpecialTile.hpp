#include "Tile.hpp"

/// @brief Represents a special tile on the board where specific actions occur when players land on it.
class SpecialTile : public Tile
{
private:
  /* data */
public:
  SpecialTile(/* args */);
  ~SpecialTile();
  virtual void executeAction(Player &player) = 0;
};  

SpecialTile::SpecialTile(/* args */)
{
}

SpecialTile::~SpecialTile()
{
}
