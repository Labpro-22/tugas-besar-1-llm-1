#include "string"
#include "vector"
#include "PropertyTile.hpp"
#include "SkillCard.hpp"

/// @brief Represents a player in the game.
class Player
{
private:
  /// @brief The unique identifier for the player, generated at the start of the game.
  int id;

  /// @brief The username chosen by the player.
  std::string username;

  /// @brief The current amount of money the player has.
  int money;

  /// @brief The current status of the player (active, bankrupt, or jailed).
  PlayerStatus status;

  /// @brief The current position of the player on the board (0 to Tile Count - 1).
  int position;

  /// @brief The number of turns the player has been in jail.
  int jailTurns;

  /// @brief The skill cards currently held by the player, with a maximum of 3 cards.
  std::vector<SkillCard *> hand;

  /// @brief The properties currently owned by the player.
  std::vector<PropertyTile *> properties;

  /// @brief Flag to track if the player has used a skill card this turn.
  bool hasUsedSkillThisTurn;

  /// @brief Flag to track if the player has rolled the dice this turn.
  bool hasRolledDice;

  /// @brief The number of consecutive doubles rolled by the player,
  /// @brief resets to 0 if a non-double is rolled or if the player goes to jail after rolling 3 doubles.
  int consecutiveDoubles;

public:
  /// @brief Adds money to the player's balance.
  /// @param amount The amount of money to add.
  /// @return A reference to the updated player.
  Player &operator+=(int amount);

  /// @brief Subtracts money from the player's balance.
  /// @param amount The amount of money to subtract.
  /// @return A reference to the updated player.
  Player &operator-=(int amount);

  /// @brief Compares the player's money with another player's money.
  /// @param other The other player to compare with.
  /// @return True if the player has less money than the other player, false otherwise.
  bool operator<(const Player &other) const;

  /// @brief Compares the player's money with another player's money.
  /// @param other The other player to compare with.
  /// @return True if the player has more money than the other player, false otherwise.
  bool operator>(const Player &other) const;

  /// @brief Calculates total wealth (cash + property values)
  /// @return Total wealth of the player
  int getTotalWealth() const;

  /// @brief Gets the liquid wealth (cash) of the player
  /// @return Liquid wealth of the player
  int getLiquidWealth() const;

  /// @brief Gets the total value of the player's properties
  /// @return Total property value of the player
  int getPropertyValue() const;

  /// @brief Adds a property to the player's portfolio
  /// @param property The property tile to be added
  void addProperty(PropertyTile *property);

  /// @brief Removes a property from the player's portfolio
  /// @param property The property tile to be removed
  void removeProperty(PropertyTile *property);

  /// @brief Adds a skill card to the player's hand up to a maximum of 3 cards
  /// @param card The skill card to be added
  void addCard(SkillCard *card);

  /// @brief Removes a skill card from the player's hand
  /// @param card The skill card to be removed
  void removeCard(SkillCard *card);

  /// @brief Checks if the player has a monopoly on a specific color group
  /// @param group The color group to check
  /// @return True if the player has a monopoly on the color group, false otherwise
  bool hasMonopoly(ColorGroup group) const;
};

enum class PlayerStatus
{
  ACTIVE,
  BANKRUPT,
  JAILED
};