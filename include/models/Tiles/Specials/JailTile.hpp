#pragma once
#include "SpecialTile.hpp"

/// @brief A tile where players are placed when in jail.
class JailTile : public SpecialTile {
private:
    /* data */
public:
    /// @brief Creates a jail tile with a configured release fine.
    /// @param id The unique identifier of the tile.
    /// @param code The unique 3-character code of the tile.
    /// @param name The display name of the tile.
    /// @param fineAmount The fine required for a player to leave jail.
    JailTile(const int id, const std::string& code, const std::string& name, int fineAmount);
    ~JailTile();

    /// @brief The amount of fine a player must pay to get out of jail.
    int fineAmount;
};

JailTile::JailTile(const int id, const std::string& code, const std::string& name, int fineAmount)
    : SpecialTile(id, code, name), fineAmount(fineAmount) {}

JailTile::~JailTile() {}
