#include "ActionTile.hpp"

/// @brief A tile that allows the player to draw a card when they land on it.
class CardTile : public ActionTile
{
private:
  CardType type;
public:
  CardTile(/* args */);
  ~CardTile();
};

CardTile::CardTile(/* args */) : ActionTile(/* args */)
{
}

CardTile::~CardTile()
{
}

enum class CardType
{
  Chance,
  CommunityChest
};