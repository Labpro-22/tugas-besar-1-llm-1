#include "DemolitionCard.hpp"
#include "IGameContext.hpp"
#include "Player.hpp"
#include "PropertyTile.hpp"
#include "StreetTile.hpp"
#include <iostream>

using namespace std;

DemolitionCard::DemolitionCard(const string& name, const string& description)
    : SkillCard(name, description) {}

DemolitionCard::~DemolitionCard() {}

void DemolitionCard::executeAction(IGameContext& ctx) {
    Player& player = ctx.getActivePlayer();
    PropertyTile* target = ctx.promptSelectOpponentProperty(player);
    if (target) {
        auto* street = dynamic_cast<StreetTile*>(target);
        if (street) street->setPropertyLevel(0);
        cout << "DemolitionCard: seluruh bangunan di " << target->getName() << " dihancurkan!" << endl;
    }
}
