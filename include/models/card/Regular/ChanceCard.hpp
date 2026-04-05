#include "Card.hpp"

/// @brief A chance card that players can draw when they land on a Chance tile.
class ChanceCard : public Card
{
private:
  /* data */
public:
  ChanceCard(/* args */);
  ~ChanceCard();

  /// @brief Executes the action associated with the card.
  /// @param player The player who drew the card.
  void executeAction(Player &player) override;
};

ChanceCard::ChanceCard(/* args */) : Card(/* args */)
{
}

ChanceCard::~ChanceCard()
{
}
