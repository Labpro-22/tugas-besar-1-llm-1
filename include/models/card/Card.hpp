#include "string"
#include "Player.hpp"

/// @brief A base class for all cards in the game.
class Card
{
private:
  /// @brief The unique identifier for the card, gen
  int id;

  /// @brief The name of the card.
  std::string name;
  
  /// @brief A description of the card's effect.
  std::string description;

public:
  Card(/* args */);
  ~Card();

  /// @brief Executes the action associated with the card.
  /// @param player The player who drew the card.
  virtual void executeAction(Player &player) = 0;
};

Card::Card(/* args */)
{
}

Card::~Card()
{
}
