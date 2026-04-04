#include "Tile.hpp"

/// @brief Represents a property tile on the board.
class PropertyTile : public Tile
{
private:
  Player *owner;
  PropertyStatus status = PropertyStatus::BANK;
  int price;

public:
  PropertyTile(/* args */);
  ~PropertyTile();
};

PropertyTile::PropertyTile(/* args */) : Tile(/* args */)
{
}

PropertyTile::~PropertyTile()
{
}

enum class PropertyStatus
{
  BANK,
  OWNED,
  MORTGAGED
};