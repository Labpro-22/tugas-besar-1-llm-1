#pragma once
#include "SpecialTile.hpp"

/// @brief A free tile on the board where nothing happens.
class FreeTile : public SpecialTile {
private:
    /* data */
public:
    /// @brief Creates a free tile with the given identity and display data.
    /// @param id The unique identifier of the tile.
    /// @param code The unique 3-character code of the tile.
    /// @param name The display name of the tile.
    FreeTile(const int id, const std::string& code, const std::string& name);
    ~FreeTile();
};

FreeTile::FreeTile(const int id, const std::string& code, const std::string& name)
    : SpecialTile(id, code, name) {}

FreeTile::~FreeTile() {}
