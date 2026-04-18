#pragma once
#include "Player.hpp"
#include <string>

/// @brief Represents a tile on the board.
class Tile {
private:
    /// @brief Unique identifier for the tile.
    int id;

    /// @brief Unique 3-character code for the tile.
    std::string code;

    /// @brief Name of the tile.
    std::string name;

public:
    /// @brief Creates a tile with the given id, code, and name.
    /// @param id The unique identifier for the tile.
    /// @param code The unique 3-character code for the tile.
    /// @param name The name of the tile.
    Tile(const int id, const std::string& code, const std::string& name);
    virtual ~Tile();

    /// @brief Gets unique identifier of the tile.
    /// @return Tile id.
    int getId() const;

    /// @brief Gets unique tile code.
    /// @return Tile code.
    const std::string& getCode() const;

    /// @brief Gets display name.
    /// @return Tile name.
    const std::string& getName() const;

    /// @brief Handles the event when a player lands on this tile.
    /// @param player The player who landed on the tile.
    virtual void landedOn(Player& player) = 0;
};
