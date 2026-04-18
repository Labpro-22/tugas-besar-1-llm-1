#include "PropertyTile.hpp"

#include <algorithm>

using namespace std;

PropertyTile::PropertyTile(const int id, const string& code, const string& name, PropertyType type,
                           const int price, int mortgageValue, const vector<int>& rentTable)
    : Tile(id, code, name), owner(nullptr), status(PropertyStatus::BANK), type(type), price(price),
      mortgageValue(mortgageValue), rentTable(rentTable) {}

PropertyTile::~PropertyTile() = default;

void PropertyTile::landedOn(Player& player) {
    if (!canCollectRentFrom(player)) {
        return;
    }

    calculateRent(player);
}

Player* PropertyTile::getOwner() const {
    return owner;
}

void PropertyTile::setOwner(Player* newOwner) {
    owner = newOwner;
    status = (owner == nullptr) ? PropertyStatus::BANK : PropertyStatus::OWNED;
}

void PropertyTile::releaseToBank() {
    owner = nullptr;
    status = PropertyStatus::BANK;
}

PropertyStatus PropertyTile::getStatus() const {
    return status;
}

void PropertyTile::setStatus(PropertyStatus newStatus) {
    status = newStatus;
    if (status == PropertyStatus::BANK) {
        owner = nullptr;
    }
}

int PropertyTile::getPrice() const {
    return price;
}

PropertyType PropertyTile::getType() const {
    return type;
}

int PropertyTile::getMortgageValue() const {
    return mortgageValue;
}

const vector<int>& PropertyTile::getRentTable() const {
    return rentTable;
}

int PropertyTile::getRentByIndex(int index) const {
    if (index < 0 || index >= rentTable.size()) {
        return 0;
    }
    return rentTable[index];
}

bool PropertyTile::canCollectRentFrom(const Player& player) const {
    return owner != nullptr && status == PropertyStatus::OWNED && owner != &player;
}

bool PropertyTile::isBankOwned() const {
    return status == PropertyStatus::BANK;
}

bool PropertyTile::isMortgaged() const {
    return status == PropertyStatus::MORTGAGED;
}

void PropertyTile::mortgage() {
    if (owner != nullptr) {
        status = PropertyStatus::MORTGAGED;
    }
}

void PropertyTile::unmortgage() {
    if (owner != nullptr) {
        status = PropertyStatus::OWNED;
    }
}
