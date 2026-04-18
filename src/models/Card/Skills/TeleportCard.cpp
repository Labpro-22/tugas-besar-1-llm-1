#include "TeleportCard.hpp"
#include "Player.hpp"
#include <iostream>

using namespace std;

TeleportCard::TeleportCard(const string& name, const string& description)
    : SkillCard(name, description) {}

TeleportCard::~TeleportCard() {}

void TeleportCard::executeAction(Player& player) {
    cout << "TeleportCard diaktifkan! Pilih petak tujuan." << endl;
    player.setUsedSkillThisTurn(true);
}
