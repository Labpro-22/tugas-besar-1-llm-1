#pragma once
#include "PropertyTile.hpp"

/// @brief A railroad tile on the board.
class RailroadTile : public PropertyTile {
private:
    /* data */
public:
    /// @brief Creates a railroad tile with the given identity and pricing data.
    /// @param id The unique identifier of the tile.
    /// @param code The unique 3-character code of the tile.
    /// @param name The display name of the tile.
    /// @param price The purchase price of the railroad tile.
    RailroadTile(const int id, const std::string& code, const std::string& name, const int price);
    ~RailroadTile();

    /// @brief Calculates the rent for the railroad tile based on the number of railroads owned.
    /// @return The amount of rent to be paid.
    int calculateRent(const Player& player) const override;
};

RailroadTile::RailroadTile(const int id, const std::string& code, const std::string& name,
                           const int price)
    : PropertyTile(id, code, name, price) {}

RailroadTile::~RailroadTile() {}
