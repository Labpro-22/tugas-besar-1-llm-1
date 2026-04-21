#include "AuctionManager.hpp"
#include "IGameContext.hpp"
#include "Logger.hpp"
#include "Player.hpp"
#include "PropertyTile.hpp"
#include <iostream>
#include <vector>

using namespace std;

AuctionManager::AuctionManager() {}

AuctionManager::~AuctionManager() {}

void AuctionManager::runAuction(PropertyTile& tile, const vector<Player*>& bidderOrder,
                                IGameContext& ctx) {
    if (bidderOrder.empty()) return;

    const int bidderCount = static_cast<int>(bidderOrder.size());
    int highBid = -1;
    Player* highBidder = nullptr;
    int consecutivePasses = 0;
    int currentIndex = 0;

    cout << "\n=== LELANG: " << tile.getName() << " [" << tile.getCode() << "] ===\n";
    cout << "Harga beli asli: M" << tile.getPrice() << "\n";
    cout << "Masukkan BID <jumlah> untuk menawar, atau PASS untuk melewati.\n\n";

    while (true) {
        Player* bidder = bidderOrder[currentIndex];
        const bool mustBid = (highBidder == nullptr && consecutivePasses == bidderCount - 1);
        string highName = highBidder ? highBidder->getUsername() : "Tidak ada";
        int displayBid = highBid < 0 ? 0 : highBid;
        cout << "Penawaran tertinggi saat ini: M" << displayBid << " oleh " << highName << "\n";
        if (mustBid) {
            cout << bidder->getUsername()
                 << " wajib melakukan BID agar lelang memiliki minimal satu penawaran.\n";
        }

        pair<bool, int> bidResult = ctx.promptAuctionBid(*bidder, highBid, tile);
        bool didBid = bidResult.first;
        int amount = bidResult.second;

        if (didBid && amount > highBid) {
            highBid = amount;
            highBidder = bidder;
            consecutivePasses = 0;
            cout << bidder->getUsername() << " menawar M" << amount << "!\n";
            ctx.getLogger().logEvent(LogLevel::INFO, ctx.getCurrentTurn(), bidder->getUsername(),
                                     "LELANG_BID",
                                     tile.getCode() + " M" + to_string(amount));
        } else if (mustBid) {
            cout << "Belum ada penawaran. " << bidder->getUsername()
                 << " harus memasukkan BID yang valid.\n";
            continue;
        } else {
            ++consecutivePasses;
            cout << bidder->getUsername() << " melewati lelang.\n";
            ctx.getLogger().logEvent(LogLevel::INFO, ctx.getCurrentTurn(), bidder->getUsername(),
                                     "LELANG_PASS", tile.getCode());
            if (highBidder != nullptr && consecutivePasses >= bidderCount - 1) {
                break;
            }
        }

        currentIndex = (currentIndex + 1) % bidderCount;
    }

    cout << "\n=== HASIL LELANG ===\n";
    if (highBidder && highBid >= 0) {
        cout << highBidder->getUsername() << " memenangkan lelang dengan penawaran M" << highBid
             << "!\n";
        ctx.getLogger().logEvent(LogLevel::INFO, ctx.getCurrentTurn(), highBidder->getUsername(),
                                 "LELANG_MENANG",
                                 tile.getCode() + " M" + to_string(highBid));
        ctx.chargeVoluntary(*highBidder, highBid);
        if (highBidder->getStatus() != PlayerStatus::BANKRUPT) {
            ctx.grantProperty(*highBidder, tile);
        }
    } else {
        cout << "Tidak ada pemenang lelang. " << tile.getName() << " tetap milik Bank.\n";
    }
}
