#include "SpecialTile.hpp"

/// @brief Represents the Go tile on the board where players collect salary when they pass or land on it.
class GoTile : public SpecialTile
{
private:
  /* data */
public:
  GoTile(/* args */);
  ~GoTile();
  int salary;
};

GoTile::GoTile(/* args */)
{
}

GoTile::~GoTile()
{
}
