#include "PropertyTile.hpp"

/// @brief A railroad tile on the board.
class RailroadTile : public PropertyTile
{
private:
  /* data */
public:
  RailroadTile(/* args */);
  ~RailroadTile();

  int calculateRent() const;
};

RailroadTile::RailroadTile(/* args */)
{
}

RailroadTile::~RailroadTile()
{
}
