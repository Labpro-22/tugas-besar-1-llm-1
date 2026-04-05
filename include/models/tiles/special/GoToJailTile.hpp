#include "SpecialTile.hpp"

/// @brief A tile that sends the player to jail when they land on it.
class GoToJailTile : public SpecialTile
{
private:
  /* data */
public:
  GoToJailTile(/* args */);
  ~GoToJailTile();
};

GoToJailTile::GoToJailTile(/* args */) : SpecialTile(/* args */)
{
}

GoToJailTile::~GoToJailTile()
{
}
