#include "Card.hpp"

/// @brief A community chest card that players can draw when they land on a Community Chest tile.
class CommunityChestCard : public Card
{
private:
  /* data */
public:
  CommunityChestCard(/* args */);
  ~CommunityChestCard();

  /// @brief Executes the action associated with the card.
  /// @param player The player who drew the card.
  void executeAction(Player &player) override;
};

CommunityChestCard::CommunityChestCard(/* args */) : Card(/* args */)
{
}

CommunityChestCard::~CommunityChestCard()
{
}
