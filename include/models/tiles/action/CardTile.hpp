#include "ActionTile.hpp"

class CardTile : public ActionTile
{
private:
  CardType type;
public:
  CardTile(/* args */);
  ~CardTile();
};

CardTile::CardTile(/* args */)
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