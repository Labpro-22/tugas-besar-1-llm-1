#include "string"
#include "Player.hpp"

/// @brief A base class for all cards in the game.
class Card
{
private:
  std::string name;
  std::string description;

public:
  Card(/* args */);
  ~Card();
  virtual void executeAction(Player &player) = 0;
};

Card::Card(/* args */)
{
}

Card::~Card()
{
}
