#pragma once
#include "PropertyTile.hpp"
#include <vector>

/// @brief A street tile on the board. (Real estate)
class StreetTile : public PropertyTile {
private:
    /// @brief The color group of the street, which determines rent and building rules.
    std::string color;

    /// @brief The base rent price for the street when no houses or hotels are built.
    int rentBasePrice;
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
    /// @param rentBasePrice The base rent when no house or hotel is built.
    /// @param housePrice The cost to build one house on this street.
    /// @param hotelPrice The cost to build one hotel on this street.
    StreetTile(const int id, const std::string& code, const std::string& name, const int price,
               std::string color, int rentBasePrice, int housePrice, int hotelPrice);
    ~StreetTile();

    /// @brief Calculates the rent for this street based on its current level of development and any
    /// active festivals.
    /// @return The total rent amount that must be paid when another player lands on this street.
    int calculateRent(const Player& player) const override;
};

StreetTile::StreetTile(const int id, const std::string& code, const std::string& name,
                       const int price, std::string color, int rentBasePrice, int housePrice,
                       int hotelPrice)
    : PropertyTile(id, code, name, price), color(color), rentBasePrice(rentBasePrice),
      housePrice(housePrice), hotelPrice(hotelPrice) {}

StreetTile::~StreetTile() {}
