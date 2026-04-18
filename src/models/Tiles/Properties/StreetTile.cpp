#include "models/Tiles/Properties/StreetTile.hpp"

#include <algorithm>
#include <vector>

using namespace std;

StreetTile::StreetTile(const int id, const string& code, const string& name, const int price,
                       const string& color, int mortgageValue, int housePrice, int hotelPrice,
                       const vector<int>& rentByLevel)
    : PropertyTile(id, code, name, PropertyType::STREET, price, mortgageValue, rentByLevel),
      color(color), monopolyOwned(false), propertyLevel(0), housePrice(housePrice),
      hotelPrice(hotelPrice), festivalMultiplier(1), festivalDur(0) {}

StreetTile::~StreetTile() {}

void StreetTile::landedOn(Player& player) {
    if (isBankOwned()) {
        // TODO: Panggil CETAK_AKTA untuk menampilkan akta kepemilikan street ini.
        // CETAK_AKTA(*this);
        // Tawarkan pembelian ke player, proses transaksi jika uang cukup.
        // Jika tidak dibeli / uang tidak cukup, trigger mekanisme lelang.

        return;
    }
    PropertyTile::landedOn(player);
}

const string& StreetTile::getColor() const {
    return color;
}

int StreetTile::getPropertyLevel() const {
    return propertyLevel;
}

void StreetTile::setPropertyLevel(int level) {
    propertyLevel = clamp(level, 0, 5);
}

bool StreetTile::buildHouse() {
    if (propertyLevel < 0 || propertyLevel >= 4) {
        return false;
    }

    ++propertyLevel;
    return true;
}

bool StreetTile::buildHotel() {
    if (propertyLevel != 4) {
        return false;
    }

    propertyLevel = 5;
    return true;
}

int StreetTile::getHousePrice() const {
    return housePrice;
}

int StreetTile::getHotelPrice() const {
    return hotelPrice;
}

void StreetTile::activateFestival(int multiplier, int duration) {
    if (multiplier <= 0) {
        multiplier = 2;
    }
    if (duration <= 0) {
        duration = 3;
    }

    if (festivalMultiplier < 8) {
        festivalMultiplier = min(8, festivalMultiplier * multiplier);
    }

    festivalDur = duration;
}

void StreetTile::tickFestival() {
    if (festivalDur <= 0) {
        festivalMultiplier = 1;
        return;
    }

    --festivalDur;
    if (festivalDur == 0) {
        festivalMultiplier = 1;
    }
}

bool StreetTile::isMonopolyOwned() const {
    return monopolyOwned;
}

void StreetTile::setMonopolyOwned(bool value) {
    monopolyOwned = value;
}

int StreetTile::calculateRent(const Player& player) const {
    if (!canCollectRentFrom(player)) {
        return 0;
    }

    int rent = getRentByIndex(propertyLevel);
    if (propertyLevel == 0 && isMonopolyOwned()) {
        rent *= 2;
    }

    if (festivalDur > 0) {
        rent *= festivalMultiplier;
    }

    return rent;
}
