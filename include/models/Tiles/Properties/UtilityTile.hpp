#pragma once
#include "PropertyTile.hpp"

/// @brief A utility tile on the board. (Electric Company and Water Works)
class UtilityTile : public PropertyTile
{
private:
  /* data */
public:
  /// @brief Creates a utility tile with the given identity and pricing data.
  /// @param id The unique identifier of the tile.
  /// @param code The unique 3-character code of the tile.
  /// @param name The display name of the tile.
  /// @param price The purchase price of the utility tile.
  UtilityTile(const int id, const std::string &code, const std::string &name, const int price);
  ~UtilityTile();

  /// @brief Calculates the rent for the utility tile based on the dice roll.
  /// @param diceRoll The sum of the dice roll.
  /// @return The amount of rent to be paid.
  int calculateRent(int diceRoll) const;
};

UtilityTile::UtilityTile(const int id, const std::string &code, const std::string &name, const int price) : PropertyTile(id, code, name, price)
{
}

UtilityTile::~UtilityTile()
{
}

