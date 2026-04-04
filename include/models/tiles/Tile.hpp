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

  virtual void landedOn(Player &player) = 0;
};

Tile::Tile(/* args */)
{
}

Tile::~Tile()
{
}
