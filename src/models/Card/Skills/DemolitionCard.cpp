#include "DemolitionCard.hpp"
#include "IGameContext.hpp"
#include "Player.hpp"
#include "PropertyTile.hpp"
#include <iostream>

using namespace std;

DemolitionCard::DemolitionCard(const string& name, const string& description)
    : SkillCard(name, description) {}

DemolitionCard::~DemolitionCard() {}

void DemolitionCard::executeAction(IGameContext& ctx) {
    Player& player = ctx.getActivePlayer();
    PropertyTile* target = ctx.promptSelectOpponentProperty(player);
    if (target) {
        cout << "DemolitionCard: " << target->getName() << " dihancurkan!" << endl;
        ctx.destroyPropertyToBank(*target);
    }
}
