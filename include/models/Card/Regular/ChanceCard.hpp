#include "Card.hpp"

/// @brief A chance card that players can draw when they land on a Chance tile.
class ChanceCard : public Card
{
private:
  /* data */
public:
  /// @brief Creates a chance card with the given name and description.
  /// @param name The name of the chance card.
  /// @param description The description of the chance card's effect.
  ChanceCard(const std::string &name, const std::string &description);
  ~ChanceCard();

  /// @brief Executes the action associated with the card.
  /// @param player The player who drew the card.
  void executeAction(Player &player) override;
};

ChanceCard::ChanceCard(const std::string &name, const std::string &description) : Card(name, description)
{
}

ChanceCard::~ChanceCard()
{
}
