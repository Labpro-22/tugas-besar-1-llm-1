#include "PropertyTile.hpp"

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
