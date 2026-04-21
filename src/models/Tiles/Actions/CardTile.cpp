#include "CardTile.hpp"
#include "Deck.hpp"
#include "IGameContext.hpp"
#include "Card.hpp"

using namespace std;

CardTile::CardTile(int id, const string& code, const string& name, ICardDeck& deck)
    : ActionTile(id, code, name), deck(deck) {}

CardTile::~CardTile() {}

void CardTile::executeAction(IGameContext& ctx) {
    Card* card = deck.draw();
    if (!card) return;
    card->executeAction(ctx);
    deck.discard(card);
}
