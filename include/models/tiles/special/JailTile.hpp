#include "SpecialTile.hpp"

/// @brief A tile where players are placed when in jail.
class JailTile : public SpecialTile
{
private:
  /* data */
public:
  JailTile(/* args */);
  ~JailTile();

  /// @brief The amount of fine a player must pay to get out of jail.
  int fineAmount;
};  

JailTile::JailTile(/* args */) : SpecialTile(/* args */)
{
}

JailTile::~JailTile()
{
}
