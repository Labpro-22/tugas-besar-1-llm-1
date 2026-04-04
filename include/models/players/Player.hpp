#include "string"
#include "vector"
#include "PropertyTile.hpp"
#include "SkillCard.hpp"

/// @brief Represents a player in the game.
class Player
{
private:
  string username;
  int money;
  PlayerStatus status;

  int position;
  int jailTurns;
  std::vector<SkillCard *> hand;
  std::vector<PropertyTile *> properties;
  bool hasUsedSkillThisTurn;
  bool hasRolledDice;
  int consecutiveDoubles;

public:
  Player &operator+=(int amount);
  Player &operator-=(int amount);
  bool operator<(const Player &other) const;
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