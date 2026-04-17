#include "CommunityChestCard.hpp"
#include "Player.hpp"
#include <iostream>

using namespace std;

CommunityChestCard::CommunityChestCard(const string &name, const string &description, CommunityChestEffect effect)
    : Card(name, description), effect(effect)
{
}

CommunityChestCard::~CommunityChestCard() {}

CommunityChestEffect CommunityChestCard::getEffect() const
{
    return effect;
}

void CommunityChestCard::executeAction(Player &player)
{
    cout << "Kartu Dana Umum: " << getName() << endl;
    cout << "Efek: " << getDescription() << endl;

    switch (effect)
    {
    case CommunityChestEffect::BIRTHDAY_COLLECT_100:
        cout << player.getUsername() << " menerima M100 dari setiap pemain lainnya." << endl;
        break;

    case CommunityChestEffect::DOCTOR_FEE_700:
    {
        const int fee = 700;
        if (player.getMoney() < fee)
        {
            cout << player.getUsername() << " tidak mampu membayar biaya dokter (M" << fee << ")." << endl;
        }
        else
        {
            player -= fee;
            cout << player.getUsername() << " membayar M" << fee << " ke Bank. Sisa uang: M" << player.getMoney() << "." << endl;
        }
        break;
    }

    case CommunityChestEffect::CAMPAIGN_PAY_200:
        cout << player.getUsername() << " harus membayar M200 ke setiap pemain lainnya." << endl;
        break;
    }
}
