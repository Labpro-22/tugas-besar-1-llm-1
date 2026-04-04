#include "Tile.hpp"

class SpecialTile : public Tile
{
private:
  /* data */
public:
  SpecialTile(/* args */);
  ~SpecialTile();
  virtual void action(Player &player) = 0;
};  

SpecialTile::SpecialTile(/* args */)
{
}

SpecialTile::~SpecialTile()
{
}
