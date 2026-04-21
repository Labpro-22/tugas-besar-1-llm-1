#include "DiscountCard.hpp"
#include "IGameContext.hpp"
#include "Player.hpp"
#include <iostream>

using namespace std;

DiscountCard::DiscountCard(const string& name, const string& description, int discountPercentage,
                           int duration)
    : SkillCard(name, description), discountPercentage(discountPercentage), duration(duration) {}

DiscountCard::~DiscountCard() {}

int DiscountCard::getDiscountPercentage() const {
    return discountPercentage;
}

int DiscountCard::getDuration() const {
    return duration;
}

void DiscountCard::executeAction(IGameContext& ctx) {
    Player& player = ctx.getActivePlayer();
    player.applyDiscount(discountPercentage, duration);
    cout << "DiscountCard diaktifkan! Kamu mendapat diskon " << discountPercentage
         << "% untuk pembelian selama " << duration << " giliran." << endl;
}
