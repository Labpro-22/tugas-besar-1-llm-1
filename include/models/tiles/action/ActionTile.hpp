#include "Tile.hpp"

/// @brief A tile that triggers an action when landed on.
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