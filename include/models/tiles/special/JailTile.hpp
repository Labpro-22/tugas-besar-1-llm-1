#include "SpecialTile.hpp"

/// @brief Represents a tile where players are placed when in jail.
class JailTile : public SpecialTile
{
private:
  /* data */
public:
  JailTile(/* args */);
  ~JailTile();
  int fineAmount;
};  

JailTile::JailTile(/* args */)
{
}

JailTile::~JailTile()
{
}
