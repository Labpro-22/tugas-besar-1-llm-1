#include "Player.hpp"
#include <iostream>
using namespace std;

Player::Player(int id, const string &username) : 
    id(id), username(username), money(0), status(PlayerStatus::ACTIVE), position(0), jailTurns(0), hasUsedSkillThisTurn(false), hasRolledDice(false), consecutiveDoubles(0) {
}

Player::~Player() {}