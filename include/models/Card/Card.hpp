#include "string"
#include "Player.hpp"

static int nextCardId = 1;

/// @brief A base class for all cards in the game.
class Card
{
private:
  /// @brief The unique identifier for the card, generated automatically.
  int id;

  /// @brief The name of the card.
  std::string name;

  /// @brief A description of the card's effect.
  std::string description;

public:
  /// @brief Creates a new card with the given name and description.
  /// @param name The name of the card.
  /// @param description The description of the card's effect.
  Card(const std::string &name, const std::string &description);
  ~Card();

  /// @brief Executes the action associated with the card.
  /// @param player The player who drew the card.
  virtual void executeAction(Player &player) = 0;
};

/// @brief Creates a new card with the given name and description.
/// @param name The name of the card.
/// @param description The description of the card's effect.
Card::Card(const std::string &name, const std::string &description) : id(nextCardId++), name(name), description(description)
{
}

Card::~Card()
{
}
