#include "Tile.hpp"

/// @brief Represents a tile that triggers an action when landed on.
class ActionTile : public Tile
{
private:
  /* data */
public:
  ActionTile(/* args */);
  ~ActionTile();
  virtual void executeAction(Player &player) = 0;
};

ActionTile::ActionTile(/* args */)
{
}

ActionTile::~ActionTile()
{
}