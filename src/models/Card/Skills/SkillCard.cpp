#include "SkillCard.hpp"
#include "Player.hpp"

SkillCard::SkillCard(const std::string &name, const std::string &description)
    : Card(name, description)
{
}

SkillCard::~SkillCard() {}

bool SkillCard::usable(const Player &player) const
{
    return !player.getHasUsedSkillThisTurn() && !player.getHasRolledDice();
}
