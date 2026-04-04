#include "Tile.hpp"

class ActionTile : public Tile
{
private:
  /* data */
public:
  ActionTile(/* args */);
  ~ActionTile();
  virtual void action(Player &player) = 0;
};