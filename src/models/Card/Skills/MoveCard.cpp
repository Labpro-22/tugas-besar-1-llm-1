#include "MoveCard.hpp"
#include "Player.hpp"
#include <iostream>

using namespace std;

static const int BOARD_SIZE = 40;

MoveCard::MoveCard(const std::string& name, const std::string& description, int steps)
    : SkillCard(name, description), steps(steps) {}

MoveCard::~MoveCard() {}

int MoveCard::getSteps() const {
    return steps;
}

void MoveCard::executeAction(Player& player) {
    int oldPos = player.getPosition();
    int newPos = (oldPos + steps) % BOARD_SIZE;
    player.setPosition(newPos);

    cout << "MoveCard digunakan! Maju " << steps << " petak." << endl;
    cout << "Bidak " << player.getUsername() << " berpindah dari petak " << (oldPos + 1)
         << " ke petak " << (newPos + 1) << "." << endl;

    player.setUsedSkillThisTurn(true);
}
