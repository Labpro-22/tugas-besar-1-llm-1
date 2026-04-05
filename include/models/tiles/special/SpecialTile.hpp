#include "Tile.hpp"

/// @brief A special tile on the board where specific actions occur when players land on it.
class SpecialTile : public Tile
{
private:
  /* data */
public:
  SpecialTile(/* args */);
  ~SpecialTile();

  /// @brief Executes the action when a player lands on this tile.
  /// @param player The player who landed on the tile.
  virtual void executeAction(Player &player) = 0;
};  

SpecialTile::SpecialTile(/* args */) : Tile(/* args */)
{
}

SpecialTile::~SpecialTile()
{
}
