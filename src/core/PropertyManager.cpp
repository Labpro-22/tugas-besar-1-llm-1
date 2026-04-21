#include "PropertyManager.hpp"
#include "Board.hpp"
#include "ColorGroup.hpp"
#include "Exceptions.hpp"
#include "Logger.hpp"
#include "Player.hpp"
#include "PropertyTile.hpp"
#include "StreetTile.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

void PropertyManager::mortgage(Player& player, Board& board, int currentTurn, Logger& logger) {
    vector<PropertyTile*> mortgageable;
    for (PropertyTile* prop : player.getProperties()) {
        if (prop->getStatus() == PropertyStatus::OWNED)
            mortgageable.push_back(prop);
    }

    if (mortgageable.empty()) {
        throw GameStateException("Tidak ada properti yang dapat digadaikan saat ini.");
    }

    cout << "=== Properti yang Dapat Digadaikan ===\n";
    for (int i = 0; i < static_cast<int>(mortgageable.size()); i++) {
        cout << (i + 1) << ". " << mortgageable[i]->getName() << " (" << mortgageable[i]->getCode()
             << ")  Nilai Gadai: M" << mortgageable[i]->getMortgageValue() << "\n";
    }
    cout << "Pilih nomor properti (0 untuk batal): ";

    int choice = 0;
    cin >> choice;
    cin.ignore();
    if (choice < 1 || choice > static_cast<int>(mortgageable.size())) {
        if (choice == 0)
            return;
        throw InvalidCommandException("Nomor properti tidak valid untuk digadaikan.");
    }

    PropertyTile* selected = mortgageable[choice - 1];
    auto* selectedStreet = dynamic_cast<StreetTile*>(selected);

    if (selectedStreet) {
        ColorGroup cg = selectedStreet->getColorGroup();
        bool hasBuildings = false;
        vector<StreetTile*> groupStreets;
        for (int i = 0; i < board.getTotalTiles(); i++) {
            auto* s = dynamic_cast<StreetTile*>(board.getTileAt(i));
            if (s && s->getColorGroup() == cg && s->getOwner() == &player) {
                groupStreets.push_back(s);
                if (s->getPropertyLevel() > 0)
                    hasBuildings = true;
            }
        }
        if (hasBuildings) {
            cout << selected->getName()
                 << " tidak dapat digadaikan! Masih ada bangunan di "
                    "color group ["
                 << colorGroupToString(cg) << "].\n";
            cout << "Jual semua bangunan color group [" << colorGroupToString(cg) << "]? (y/n): ";
            char resp;
            cin >> resp;
            cin.ignore();
            if (resp != 'y' && resp != 'Y') {
                cout << "Dibatalkan.\n";
                return;
            }
            for (auto* s : groupStreets) {
                int lvl = s->getPropertyLevel();
                if (lvl == 0)
                    continue;
                int refund = 0;
                if (lvl == 5) {
                    refund = (s->getHotelPrice() + 4 * s->getHousePrice()) / 2;
                } else {
                    refund = lvl * s->getHousePrice() / 2;
                }
                s->setPropertyLevel(0);
                player += refund;
                cout << "Bangunan " << s->getName() << " terjual. Kamu menerima M" << refund
                     << ".\n";
                logger.logEvent(LogLevel::INFO, currentTurn, player.getUsername(), "JUAL_BANGUNAN",
                                "Jual bangunan " + s->getName() + " M" + to_string(refund));
            }
        }
    }

    selected->mortgage();
    player += selected->getMortgageValue();
    cout << selected->getName() << " berhasil digadaikan. Kamu menerima M"
         << selected->getMortgageValue() << " dari Bank.\n";
    cout << "Uang kamu sekarang: M" << player.getMoney() << "\n";
    cout << "Catatan: Sewa tidak dapat dipungut dari properti yang digadaikan.\n";
    logger.logEvent(LogLevel::INFO, currentTurn, player.getUsername(), "GADAI",
                    "Gadai " + selected->getName() + " M" +
                        to_string(selected->getMortgageValue()));
}

void PropertyManager::redeem(Player& player, int currentTurn, Logger& logger) {
    vector<PropertyTile*> mortgaged;
    for (PropertyTile* prop : player.getProperties()) {
        if (prop->isMortgaged())
            mortgaged.push_back(prop);
    }

    if (mortgaged.empty()) {
        throw GameStateException("Tidak ada properti yang sedang digadaikan.");
    }

    cout << "=== Properti yang Sedang Digadaikan ===\n";
    for (int i = 0; i < static_cast<int>(mortgaged.size()); i++) {
        cout << (i + 1) << ". " << mortgaged[i]->getName() << " (" << mortgaged[i]->getCode()
             << ")  [M]  Harga Tebus: M" << mortgaged[i]->getPrice() << "\n";
    }
    cout << "Uang kamu saat ini: M" << player.getMoney() << "\n";
    cout << "Pilih nomor properti (0 untuk batal): ";

    int choice = 0;
    cin >> choice;
    cin.ignore();
    if (choice < 1 || choice > static_cast<int>(mortgaged.size())) {
        if (choice == 0)
            return;
        throw InvalidCommandException("Nomor properti tidak valid untuk ditebus.");
    }

    PropertyTile* selected = mortgaged[choice - 1];
    int redeemPrice = selected->getPrice();

    if (player.getMoney() < redeemPrice) {
        throw InsufficientFundsException(redeemPrice, player.getMoney());
    }

    player -= redeemPrice;
    selected->unmortgage();
    cout << selected->getName() << " berhasil ditebus! Kamu membayar M" << redeemPrice
         << " ke Bank.\n";
    cout << "Uang kamu sekarang: M" << player.getMoney() << "\n";
    logger.logEvent(LogLevel::INFO, currentTurn, player.getUsername(), "TEBUS",
                    "Tebus " + selected->getName() + " M" + to_string(redeemPrice));
}

void PropertyManager::build(Player& player, Board& board, int currentTurn, Logger& logger) {
    vector<ColorGroup> monoGroups = player.getMonopolyGroups();
    if (monoGroups.empty()) {
        throw GameStateException(
            "Tidak ada color group yang memenuhi syarat untuk dibangun. (Belum Memonopoli)");
    }

    class GroupInfo {
    public:
        ColorGroup cg;
        vector<StreetTile*> streets;
    };

    vector<GroupInfo> buildableGroups;
    for (ColorGroup cg : monoGroups) {
        vector<StreetTile*> streets;
        bool anyCanBuild = false;
        for (int i = 0; i < board.getTotalTiles(); i++) {
            auto* s = dynamic_cast<StreetTile*>(board.getTileAt(i));
            if (s && s->getColorGroup() == cg && s->getOwner() == &player && !s->isMortgaged()) {
                streets.push_back(s);
                if (s->getPropertyLevel() < 5)
                    anyCanBuild = true;
            }
        }
        if (!streets.empty() && anyCanBuild)
            buildableGroups.push_back({cg, streets});
    }

    if (buildableGroups.empty()) {
        throw GameStateException("Tidak ada color group yang memenuhi syarat untuk dibangun.");
    }

    cout << "=== Color Group yang Memenuhi Syarat ===\n";
    for (int i = 0; i < static_cast<int>(buildableGroups.size()); i++) {
        auto& gi = buildableGroups[i];
        cout << (i + 1) << ". [" << colorGroupToString(gi.cg) << "]\n";
        for (auto* s : gi.streets) {
            int lvl = s->getPropertyLevel();
            string lvlStr = (lvl == 5) ? "Hotel" : to_string(lvl) + " rumah";
            int price = (lvl >= 4) ? s->getHotelPrice() : s->getHousePrice();
            cout << "  - " << s->getName() << " (" << s->getCode() << "): " << lvlStr
                 << " (Harga bangun: M" << price << ")\n";
        }
    }
    cout << "Uang kamu saat ini: M" << player.getMoney() << "\n";
    cout << "Pilih nomor color group (0 untuk batal): ";

    int groupChoice = 0;
    cin >> groupChoice;
    cin.ignore();
    if (groupChoice < 1 || groupChoice > static_cast<int>(buildableGroups.size())) {
        if (groupChoice == 0)
            return;
        throw InvalidCommandException("Color group tidak valid.");
    }

    auto& selectedGroup = buildableGroups[groupChoice - 1];

    // Pemerataan: find minimum level in the group
    int minLevel = 5;
    for (auto* s : selectedGroup.streets)
        minLevel = min(minLevel, s->getPropertyLevel());

    bool allFourHouses = true;
    for (auto* s : selectedGroup.streets) {
        if (s->getPropertyLevel() != 4) {
            allFourHouses = false;
            break;
        }
    }

    // Determine buildable tiles
    vector<StreetTile*> buildable;
    for (auto* s : selectedGroup.streets) {
        int lvl = s->getPropertyLevel();
        if (allFourHouses && lvl == 4) {
            buildable.push_back(s);
        } else if (!allFourHouses && lvl == minLevel && lvl < 4) {
            buildable.push_back(s);
        }
    }

    if (buildable.empty()) {
        throw GameStateException(
            "Tidak ada petak yang dapat dibangun saat ini (semua sudah hotel).");
    }

    cout << "Color group [" << colorGroupToString(selectedGroup.cg) << "]:\n";
    for (auto* s : selectedGroup.streets) {
        int lvl = s->getPropertyLevel();
        string lvlStr = (lvl == 5) ? "Hotel" : to_string(lvl) + " rumah";
        bool canBuild = find(buildable.begin(), buildable.end(), s) != buildable.end();
        cout << "- " << s->getName() << " (" << s->getCode() << "): " << lvlStr
             << (canBuild ? "  <- dapat dibangun" : "") << "\n";
    }

    cout << "\nPetak yang dapat dibangun:\n";
    for (int i = 0; i < static_cast<int>(buildable.size()); i++) {
        auto* s = buildable[i];
        int lvl = s->getPropertyLevel();
        string action = (lvl == 4) ? "Upgrade ke Hotel" : "Bangun 1 Rumah";
        int cost = (lvl >= 4) ? s->getHotelPrice() : s->getHousePrice();
        cout << (i + 1) << ". " << s->getName() << " - " << action << " (M" << cost << ")\n";
    }
    cout << "Pilih petak (0 untuk batal): ";

    int buildChoice = 0;
    cin >> buildChoice;
    cin.ignore();
    if (buildChoice < 1 || buildChoice > static_cast<int>(buildable.size())) {
        if (buildChoice == 0)
            return;
        throw InvalidCommandException("Nomor petak tidak valid.");
    }

    StreetTile* toBuild = buildable[buildChoice - 1];
    int lvl = toBuild->getPropertyLevel();
    int cost = (lvl >= 4) ? toBuild->getHotelPrice() : toBuild->getHousePrice();

    if (player.getMoney() < cost) {
        throw InsufficientFundsException(cost, player.getMoney());
    }

    player -= cost;
    string logDetail;
    if (lvl == 4) {
        toBuild->buildHotel();
        cout << toBuild->getName() << " di-upgrade ke Hotel! Biaya: M" << cost << "\n";
        logDetail = "Hotel di " + toBuild->getName();
    } else {
        toBuild->buildHouse();
        cout << "1 rumah dibangun di " << toBuild->getName() << ". Biaya: M" << cost << "\n";
        logDetail = "Rumah L" + to_string(lvl + 1) + " di " + toBuild->getName();
    }
    cout << "Uang tersisa: M" << player.getMoney() << "\n";
    logger.logEvent(LogLevel::INFO, currentTurn, player.getUsername(), "BANGUN", logDetail);
}
