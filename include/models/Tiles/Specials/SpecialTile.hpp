#pragma once
#include "Tile.hpp"

/// @brief A special tile on the board where specific actions occur when players land on it.
class SpecialTile : public Tile
{
private:
  /* data */
public:
  /// @brief Creates a special tile with the given identity and display data.
  /// @param id The unique identifier of the tile.
  /// @param code The unique 3-character code of the tile.
  /// @param name The display name of the tile.
  SpecialTile(const int id, const std::string &code, const std::string &name);
  ~SpecialTile();

  /// @brief Executes the action when a player lands on this tile.
  /// @param player The player who landed on the tile.
  virtual void executeAction(Player &player) = 0;
};  

SpecialTile::SpecialTile(const int id, const std::string &code, const std::string &name) : Tile(id, code, name)
{
}

SpecialTile::~SpecialTile()
{
}

