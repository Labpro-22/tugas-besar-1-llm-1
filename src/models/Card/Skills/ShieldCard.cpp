#include "ShieldCard.hpp"
#include "IGameContext.hpp"
#include "Player.hpp"
#include <iostream>

using namespace std;

ShieldCard::ShieldCard(const string& name, const string& description, int duration)
    : SkillCard(name, description), duration(duration) {}

ShieldCard::~ShieldCard() {}

int ShieldCard::getDuration() const {
    return duration;
}

void ShieldCard::executeAction(IGameContext& ctx) {
    Player& player = ctx.getActivePlayer();
    player.activateShield(duration);
    cout << "ShieldCard diaktifkan! Anda kebal terhadap tagihan atau sanksi selama " << duration
         << " giliran." << endl;
}
