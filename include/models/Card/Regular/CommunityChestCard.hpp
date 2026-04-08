#include "Card.hpp"

/// @brief A community chest card that players can draw when they land on a Community Chest tile.
class CommunityChestCard : public Card
{
private:
  /* data */
public:
  /// @brief Creates a community chest card with the given name and description.
  /// @param name The name of the community chest card.
  /// @param description The description of the community chest card's effect.
  CommunityChestCard(const std::string &name, const std::string &description);
  ~CommunityChestCard();

  /// @brief Executes the action associated with the card.
  /// @param player The player who drew the card.
  void executeAction(Player &player) override;
};

CommunityChestCard::CommunityChestCard(const std::string &name, const std::string &description) : Card(name, description)
{
}

CommunityChestCard::~CommunityChestCard()
{
}
