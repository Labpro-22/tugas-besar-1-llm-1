#include "ShieldCard.hpp"
#include "Player.hpp"
#include <iostream>

using namespace std;

ShieldCard::ShieldCard(const string& name, const string& description, int duration)
    : SkillCard(name, description), duration(duration) {}

ShieldCard::~ShieldCard() {}

int ShieldCard::getDuration() const {
    return duration;
}

void ShieldCard::executeAction(Player& player) {
    player.activateShield(duration);

    cout << "ShieldCard diaktifkan! Anda kebal terhadap tagihan atau sanksi selama " << duration
         << " giliran." << endl;

    player.setUsedSkillThisTurn(true);
}
