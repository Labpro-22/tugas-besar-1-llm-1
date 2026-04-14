#pragma once
#include "SpecialTile.hpp"

/// @brief The Go tile on the board where players collect salary when they pass or land on it.
class GoTile : public SpecialTile
{
private:
  /* data */
public:
  /// @brief Creates a go tile with a configured salary reward.
  /// @param id The unique identifier of the tile.
  /// @param code The unique 3-character code of the tile.
  /// @param name The display name of the tile.
  /// @param salary The amount awarded when a player passes or lands on this tile.
  GoTile(const int id, const std::string &code, const std::string &name, int salary);
  ~GoTile();
  
  /// @brief The amount of salary a player collects when they pass or land on the Go tile.
  int salary;
};

GoTile::GoTile(const int id, const std::string &code, const std::string &name, int salary)
  : SpecialTile(id, code, name), salary(salary)
{
}

GoTile::~GoTile()
{
}

