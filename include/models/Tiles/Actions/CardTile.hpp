#include "ActionTile.hpp"

enum class CardType
{
  Chance,
  CommunityChest
};

/// @brief A tile that allows the player to draw a card when they land on it.
class CardTile : public ActionTile
{
private:
  /// @brief The type of card deck this tile draws from (Chance or Community Chest).
  CardType type;
public:
  /// @brief Creates a card tile with a specific card deck type.
  /// @param id The unique identifier of the tile.
  /// @param code The unique 3-character code of the tile.
  /// @param name The display name of the tile.
  /// @param type The type of card deck this tile draws from.
  CardTile(const int id, const std::string &code, const std::string &name, CardType type);
  ~CardTile();
};

CardTile::CardTile(const int id, const std::string &code, const std::string &name, CardType type)
    : ActionTile(id, code, name), type(type)
{
}

CardTile::~CardTile()
{
}