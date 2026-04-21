#include "TeleportCard.hpp"
#include "Board.hpp"
#include "IGameContext.hpp"
#include "Player.hpp"
#include <iostream>

using namespace std;

TeleportCard::TeleportCard(const string& name, const string& description)
    : SkillCard(name, description) {}

TeleportCard::~TeleportCard() {}

void TeleportCard::executeAction(IGameContext& ctx) {
    Player& player = ctx.getActivePlayer();
    int destIdx = ctx.promptTileIndex(player);
    ctx.repositionPlayer(player, destIdx);
    cout << "TeleportCard: pindah ke "
         << ctx.getBoard().getTileAt(destIdx)->getName() << "." << endl;
    ctx.getBoard().getTileAt(destIdx)->landedOn(ctx);
}
