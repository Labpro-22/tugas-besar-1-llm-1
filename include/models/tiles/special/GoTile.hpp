#include "SpecialTile.hpp"

/// @brief The Go tile on the board where players collect salary when they pass or land on it.
class GoTile : public SpecialTile
{
private:
  /* data */
public:
  GoTile(/* args */);
  ~GoTile();
  
  /// @brief The amount of salary a player collects when they pass or land on the Go tile.
  int salary;
};

GoTile::GoTile(/* args */) : SpecialTile(/* args */)
{
}

GoTile::~GoTile()
{
}
