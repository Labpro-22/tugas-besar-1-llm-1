#include "LassoCard.hpp"
#include "Player.hpp"
#include <iostream>

using namespace std;

LassoCard::LassoCard(const std::string &name, const std::string &description)
    : SkillCard(name, description)
{
}

LassoCard::~LassoCard() {}

void LassoCard::executeAction(Player &player)
{
    cout << "LassoCard diaktifkan! Memilih lawan yang akan ditarik..." << endl;
    player.setUsedSkillThisTurn(true);
}
