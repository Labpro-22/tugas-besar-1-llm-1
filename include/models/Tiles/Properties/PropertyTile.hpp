#pragma once
#include "Tile.hpp"
#include <string>
#include <vector>

using namespace std;

enum class PropertyStatus {
    BANK,
    OWNED,
    MORTGAGED
};

enum class PropertyType {
    STREET,
    RAILROAD,
    UTILITY
};

/// @brief A property tile on the board.
class PropertyTile : public Tile {
private:
    /// @brief The player who owns this property. Null if the property is owned by the bank.
    Player* owner;

    /// @brief The status of the property (owned, mortgaged, or bank-owned).
    PropertyStatus status = PropertyStatus::BANK;

    /// @brief The explicit type of this property.
    PropertyType type;

    /// @brief The price of the property.
    int price;

    /// @brief Mortgage value of this property.
    int mortgageValue;

    /// @brief Rent values indexed by property level/ownership tier.
    vector<int> rentTable;

public:
    /// @brief Creates a property tile with ownership defaults and pricing data.
    /// @param id The unique identifier of the tile.
    /// @param code The unique 3-character code of the tile.
    /// @param name The display name of the tile.
    /// @param type The explicit type of the property (Street/Railroad/Utility).
    /// @param price The purchase price of the property.
    /// @param mortgageValue The mortgage value of the property.
    /// @param rentTable Rent table used by concrete property types.
    PropertyTile(const int id, const string& code, const string& name, PropertyType type,
                 const int price, int mortgageValue, const vector<int>& rentTable = {});
    ~PropertyTile() override;

    /// @brief Handles the default property landing behavior (rent payment if applicable).
    /// @param player The player who landed on this property.
    void landedOn(Player& player) override;

    /// @brief Gets the owner of this property.
    /// @return Owner pointer, nullptr if bank-owned.
    Player* getOwner() const;

    /// @brief Sets the owner of this property.
    /// @param newOwner New owner pointer, nullptr to return ownership to bank.
    void setOwner(Player* newOwner);

    /// @brief Returns this property to bank ownership.
    void releaseToBank();

    /// @brief Gets current ownership status.
    /// @return Property status.
    PropertyStatus getStatus() const;

    /// @brief Sets the status of this property.
    /// @param newStatus New property status.
    void setStatus(PropertyStatus newStatus);

    /// @brief Gets purchase price.
    /// @return Property purchase price.
    int getPrice() const;

    /// @brief Gets property type.
    /// @return Explicit property type.
    PropertyType getType() const;

    /// @brief Gets mortgage value.
    /// @return Property mortgage value.
    int getMortgageValue() const;

    /// @brief Gets rent table.
    /// @return Rent table.
    const vector<int>& getRentTable() const;

    /// @brief Gets rent value at an index from rent table.
    /// @param index Rent level index.
    /// @return Rent value at index or 0 if index is out of range.
    int getRentByIndex(int index) const;

    /// @brief Checks whether this property can currently collect rent from landing player.
    /// @param player The landing player.
    /// @return True if rent should be collected.
    bool canCollectRentFrom(const Player& player) const;

    /// @brief Checks whether this property is currently bank-owned.
    /// @return True if status is BANK.
    bool isBankOwned() const;

    /// @brief Checks whether this property is currently mortgaged.
    /// @return True if status is MORTGAGED.
    bool isMortgaged() const;

    /// @brief Apply mortgage state.
    void mortgage();

    /// @brief Remove mortgage state.
    void unmortgage();

    /// @brief Calculates the amount of rent needed to be paid when the player lands on this
    /// property.
    /// @param player The player who landed on the property.
    /// @return The amount of rent to be paid, 0 if the property is not owned by another player or
    /// is mortgaged.
    virtual int calculateRent(const Player& player) const = 0;
};
