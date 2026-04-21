#include "MoveCard.hpp"
#include "IGameContext.hpp"
#include "Player.hpp"
#include <iostream>

using namespace std;

MoveCard::MoveCard(const string& name, const string& description, int steps)
    : SkillCard(name, description), steps(steps) {}

MoveCard::~MoveCard() {}

int MoveCard::getSteps() const {
    return steps;
}

void MoveCard::executeAction(IGameContext& ctx) {
    Player& player = ctx.getActivePlayer();
    cout << "MoveCard: maju " << steps << " petak." << endl;
    ctx.movePlayerBy(player, steps);
}
