#pragma once
#include "Tile.hpp"

/// @brief A property tile on the board.
class PropertyTile : public Tile {
private:
    /// @brief The player who owns this property. Null if the property is owned by the bank.
    Player* owner;

    /// @brief The status of the property (owned, mortgaged, or bank-owned).
    PropertyStatus status = PropertyStatus::BANK;

    /// @brief The price of the property.
    int price;

public:
    /// @brief Creates a property tile with ownership defaults and pricing data.
    /// @param id The unique identifier of the tile.
    /// @param code The unique 3-character code of the tile.
    /// @param name The display name of the tile.
    /// @param price The purchase price of the property.
    PropertyTile(const int id, const std::string& code, const std::string& name, const int price);
    ~PropertyTile();

    /// @brief Calculates the amount of rent needed to be paid when the player lands on this
    /// property.
    /// @param player The player who landed on the property.
    /// @return The amount of rent to be paid, 0 if the property is not owned by another player or
    /// is mortgaged.
    virtual int calculateRent(const Player& player) const = 0;
};

PropertyTile::PropertyTile(const int id, const std::string& code, const std::string& name,
                           const int price)
    : Tile(id, code, name), price(price) {}

PropertyTile::~PropertyTile() {}

enum class PropertyStatus {
    BANK,
    OWNED,
    MORTGAGED
};
