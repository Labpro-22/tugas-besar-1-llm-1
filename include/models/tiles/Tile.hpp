#include <string>
#include "Player.hpp"

/// @brief Represents a tile on the board.
class Tile
{
private:
  int id;
  std::string code;
  std::string name;

public:
  Tile(/* args */);
  ~Tile();

  /// @brief Handles the event when a player lands on this tile.
  /// @param player The player who landed on the tile.
  virtual void landedOn(Player &player) = 0;
};

Tile::Tile(/* args */)
{
}

Tile::~Tile()
{
}
