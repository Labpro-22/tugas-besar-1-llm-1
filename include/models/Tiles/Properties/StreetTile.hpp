#pragma once
#include "PropertyTile.hpp"

using namespace std;

/// @brief A street tile on the board. (Real estate)
class StreetTile : public PropertyTile {
private:
    /// @brief The color group of the street, which determines rent and building rules.
    string color;

    /// @brief Whether this street is currently treated as part of a monopoly.
    bool monopolyOwned = false;

    /// @brief The current number of houses built on this street (0-4), 5 indicates a hotel is
    /// built.
    int propertyLevel = 0;

    /// @brief The price to build a house on this street.
    int housePrice;
    /// @brief The price to build a hotel on this street.
    int hotelPrice;

    /// @brief The multiplier for rent when a festival is active on this street.
    int festivalMultiplier = 1;
    /// @brief The duration of the festival in turns. When greater than 0, the festival is active.
    int festivalDur = 0;

public:
    /// @brief Creates a street tile with pricing and development configuration.
    /// @param id The unique identifier of the tile.
    /// @param code The unique 3-character code of the tile.
    /// @param name The display name of the tile.
    /// @param price The purchase price of the street tile.
    /// @param color The color group this street belongs to.
    /// @param mortgageValue The mortgage value of this street.
    /// @param housePrice The cost to build one house on this street.
    /// @param hotelPrice The cost to build one hotel on this street.
    /// @param rentByLevel Rent table indexed by building level (0..5).
    StreetTile(const int id, const string& code, const string& name, const int price,
               const string& color, int mortgageValue, int housePrice, int hotelPrice,
               const vector<int>& rentByLevel);
    ~StreetTile() override;

    /// @brief Handles landing behavior on street tiles.
    /// @param player The player who landed on this street.
    void landedOn(Player& player) override;

    /// @brief Gets the street color group.
    /// @return Color group string.
    const string& getColor() const;

    /// @brief Gets current building level.
    /// @return Current property level (0-5).
    int getPropertyLevel() const;

    /// @brief Sets building level.
    /// @param level New level, clamped to range [0, 5].
    void setPropertyLevel(int level);

    /// @brief Attempts to build one house.
    /// @return True if level increased, false otherwise.
    bool buildHouse();

    /// @brief Attempts to build hotel.
    /// @return True if level set to hotel, false otherwise.
    bool buildHotel();

    /// @brief Gets house price.
    /// @return House price.
    int getHousePrice() const;

    /// @brief Gets hotel price.
    /// @return Hotel price.
    int getHotelPrice() const;

    /// @brief Activates festival effect for this street.
    /// Each activation doubles rent multiplier up to a maximum of 8x,
    /// and resets duration back to 3 turns.
    /// @param multiplier Unused parameter kept for compatibility.
    /// @param duration Unused parameter kept for compatibility.
    void activateFestival(int multiplier, int duration);

    /// @brief Decrements festival duration.
    void tickFestival();

    /// @brief Checks if owner monopolizes this street's color group.
    /// @return True if owner controls all streets of this color.
    bool isMonopolyOwned() const;

    /// @brief Sets monopoly state from external game logic.
    /// @param value Monopoly state value.
    void setMonopolyOwned(bool value);

    /// @brief Calculates the rent for this street based on its current level of development and any
    /// active festivals.
    /// @return The total rent amount that must be paid when another player lands on this street.
    int calculateRent(const Player& player) const override;
};
