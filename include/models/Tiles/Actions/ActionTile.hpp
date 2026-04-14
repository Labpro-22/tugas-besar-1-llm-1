#pragma once
#include "Tile.hpp"

/// @brief A tile that triggers an action when landed on.
class ActionTile : public Tile
{
private:
  /* data */
public:
  /// @brief Creates an action tile with the given identity and display data.
  /// @param id The unique identifier of the tile.
  /// @param code The unique 3-character code of the tile.
  /// @param name The display name of the tile.
  ActionTile(const int id, const std::string &code, const std::string &name);
  ~ActionTile();
  
  /// @brief Executes the action when a player lands on this tile.
  /// @param player The player who landed on the tile.
  virtual void executeAction(Player &player) = 0;
};

ActionTile::ActionTile(const int id, const std::string &code, const std::string &name) : Tile(id, code, name)
{
}

ActionTile::~ActionTile()
{
}
