#pragma once
#include "Card.hpp"

class Player;

/// @brief A card that can be used by the player to gain an advantage in game.
class SkillCard : public Card {
public:
    /// @brief Creates a skill card with the given name and description.
    /// @param name The name of the skill card.
    /// @param description The description of the skill card's effect.
    SkillCard(const std::string& name, const std::string& description);
    virtual ~SkillCard();

    /// @brief Executes the action associated with the card.
    /// @param player The player who uses the card.
    virtual void executeAction(Player& player) override = 0;

    /// @brief Checks if the card can be used by the player.
    /// @param player The player attempting to use the card.
    /// @return True if the card is usable, false otherwise.
    bool usable(const Player& player) const;
};
