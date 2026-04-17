#include "DemolitionCard.hpp"
#include "Player.hpp"
#include <iostream>

using namespace std;

DemolitionCard::DemolitionCard(const string &name, const string &description)
    : SkillCard(name, description)
{
}

DemolitionCard::~DemolitionCard() {}

void DemolitionCard::executeAction(Player &player)
{
    cout << "DemolitionCard diaktifkan! Pilih properti lawan yang akan dihancurkan." << endl;
    player.setUsedSkillThisTurn(true);
}
