#pragma once
#include "ActionTile.hpp"

/// @brief A tile that represents a festival tile where players can boost owned property rent
/// prices.
class FestivalTile : public ActionTile {
private:
    /* data */
public:
    /// @brief Creates a festival tile with the given identity and display data.
    /// @param id The unique identifier of the tile.
    /// @param code The unique 3-character code of the tile.
    /// @param name The display name of the tile.
    FestivalTile(const int id, const std::string& code, const std::string& name);
    ~FestivalTile();
};

FestivalTile::FestivalTile(const int id, const std::string& code, const std::string& name)
    : ActionTile(id, code, name) {}

FestivalTile::~FestivalTile() {}
