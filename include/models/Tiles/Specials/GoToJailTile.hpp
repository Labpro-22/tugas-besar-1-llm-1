#include "SpecialTile.hpp"

/// @brief A tile that sends the player to jail when they land on it.
class GoToJailTile : public SpecialTile
{
private:
  /* data */
public:
  /// @brief Creates a go-to-jail tile with the given identity and display data.
  /// @param id The unique identifier of the tile.
  /// @param code The unique 3-character code of the tile.
  /// @param name The display name of the tile.
  GoToJailTile(const int id, const std::string &code, const std::string &name);
  ~GoToJailTile();
};

GoToJailTile::GoToJailTile(const int id, const std::string &code, const std::string &name) : SpecialTile(id, code, name)
{
}

GoToJailTile::~GoToJailTile()
{
}
