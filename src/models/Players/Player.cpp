#include "Player.hpp"
#include <algorithm>
#include <iostream>
using namespace std;

Player::Player(int id, const string& username)
    : id(id), username(username), money(0), status(PlayerStatus::ACTIVE), position(0), jailTurns(0),
      hasUsedSkillThisTurn(false), hasRolledDice(false), consecutiveDoubles(0) {}

Player::~Player() {}

Player& Player::operator+=(int amount) {
    money += amount;
    return *this;
}

Player& Player::operator-=(int amount) {
    money -= amount;
    return *this;
}

int Player::getLiquidWealth() const {
    return money;
}

vector<PropertyTile*> Player::getProperties() const {
    return properties;
}

void Player::addProperty(PropertyTile* property) {
    if (property == nullptr) {
        return;
    }

    auto it = find(properties.begin(), properties.end(), property);
    if (it == properties.end()) {
        properties.push_back(property);
    }
}

void Player::removeProperty(PropertyTile* property) {
    properties.erase(remove(properties.begin(), properties.end(), property), properties.end());
}

// TODO: hasMonopoly check isMonopolyOwned in StreetTile