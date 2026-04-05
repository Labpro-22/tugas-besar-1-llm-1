#include "PropertyTile.hpp"

/// @brief A railroad tile on the board.
class RailroadTile : public PropertyTile
{
private:
  /* data */
public:
  RailroadTile(/* args */);
  ~RailroadTile();

  /// @brief Calculates the rent for the railroad tile based on the number of railroads owned.
  /// @return The amount of rent to be paid.
  int calculateRent() const;
};

RailroadTile::RailroadTile(/* args */) : PropertyTile(/* args */)
{
}

RailroadTile::~RailroadTile()
{
}
