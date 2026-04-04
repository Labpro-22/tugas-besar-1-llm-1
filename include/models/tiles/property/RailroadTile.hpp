#include "PropertyTile.hpp"

/// @brief Represents a railroad tile on the board.
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
