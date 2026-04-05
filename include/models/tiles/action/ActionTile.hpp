#include "Tile.hpp"

/// @brief A tile that triggers an action when landed on.
class ActionTile : public Tile
{
private:
  /* data */
public:
  ActionTile(/* args */);
  ~ActionTile();
  
  /// @brief Executes the action when a player lands on this tile.
  /// @param player The player who landed on the tile.
  virtual void executeAction(Player &player) = 0;
};

ActionTile::ActionTile(/* args */) : Tile(/* args */)
{
}

ActionTile::~ActionTile()
{
}