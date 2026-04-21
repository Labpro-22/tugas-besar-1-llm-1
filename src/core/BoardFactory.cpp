#include "BoardFactory.hpp"
#include "Board.hpp"
#include "ConfigManager.hpp"
#include "GameConfig.hpp"
#include "PropertyLoader.hpp"

// Tile types
#include "CardTile.hpp"
#include "FestivalTile.hpp"
#include "FreeTile.hpp"
#include "GoTile.hpp"
#include "GoToJailTile.hpp"
#include "JailTile.hpp"
#include "TaxTile.hpp"

// Card types
#include "ChanceCard.hpp"
#include "CommunityChestCard.hpp"
#include "DemolitionCard.hpp"
#include "DiscountCard.hpp"
#include "LassoCard.hpp"
#include "MoveCard.hpp"
#include "ShieldCard.hpp"
#include "SkillCard.hpp"
#include "TeleportCard.hpp"

#include <map>
#include <memory>
#include <random>
#include <string>
#include <tuple>
#include <vector>

using namespace std;

BoardFactory::BuildResult BoardFactory::build(ConfigManager& cfg) {
    const GameConfig& config = cfg.getConfig();

    auto board = make_unique<Board>();
    vector<unique_ptr<ChanceCard>> chanceCards;
    vector<unique_ptr<CommunityChestCard>> communityCards;
    vector<unique_ptr<SkillCard>> allSkillCards;

    // ── Card decks ──────────────────────────────────────────────────────────
    chanceCards.push_back(make_unique<ChanceCard>(
        "Ke Stasiun Terdekat", "Pergi ke stasiun terdekat.", ChanceEffect::GO_TO_NEAREST_STATION));
    chanceCards.push_back(
        make_unique<ChanceCard>("Mundur 3 Petak", "Mundur 3 petak.", ChanceEffect::MOVE_BACK_3));
    chanceCards.push_back(
        make_unique<ChanceCard>("Masuk Penjara", "Masuk Penjara.", ChanceEffect::GO_TO_JAIL));
    chanceCards.push_back(make_unique<ChanceCard>("Bebas dari Penjara",
                                                  "Gunakan kartu ini untuk keluar dari penjara.",
                                                  ChanceEffect::GET_OUT_OF_JAIL));

    vector<ChanceCard*> chanceRaw;
    for (auto& c : chanceCards)
        chanceRaw.push_back(c.get());
    auto chanceDeck = make_unique<CardDeck<ChanceCard>>(chanceRaw);

    communityCards.push_back(make_unique<CommunityChestCard>(
        "Ulang Tahun", "Ini adalah hari ulang tahun Anda. Dapatkan M100 dari setiap pemain.",
        CommunityChestEffect::BIRTHDAY_COLLECT_100));
    communityCards.push_back(make_unique<CommunityChestCard>(
        "Biaya Dokter", "Biaya dokter. Bayar M700.", CommunityChestEffect::DOCTOR_FEE_700));
    communityCards.push_back(make_unique<CommunityChestCard>(
        "Nyaleg", "Anda mau nyaleg. Bayar M200 kepada setiap pemain.",
        CommunityChestEffect::CAMPAIGN_PAY_200));

    vector<CommunityChestCard*> communityRaw;
    for (auto& c : communityCards)
        communityRaw.push_back(c.get());
    auto communityDeck = make_unique<CardDeck<CommunityChestCard>>(communityRaw);

    // Skill cards: 4 Move, 3 Discount, 2 Shield, 2 Teleport, 2 Lasso, 2 Demolition
    mt19937 rng(random_device{}());
    uniform_int_distribution<int> stepsDist(1, 6);
    uniform_int_distribution<int> discDist(10, 50);

    for (int i = 0; i < 4; i++) {
        int steps = stepsDist(rng);
        allSkillCards.push_back(
            make_unique<MoveCard>("MoveCard", "Maju " + to_string(steps) + " petak.", steps));
    }
    for (int i = 0; i < 3; i++) {
        int pct = discDist(rng);
        allSkillCards.push_back(
            make_unique<DiscountCard>("DiscountCard", "Diskon " + to_string(pct) + "%.", pct, 1));
    }
    for (int i = 0; i < 2; i++) {
        allSkillCards.push_back(
            make_unique<ShieldCard>("ShieldCard", "Kebal tagihan/sanksi selama 1 giliran.", 1));
    }
    for (int i = 0; i < 2; i++) {
        allSkillCards.push_back(
            make_unique<TeleportCard>("TeleportCard", "Pindah ke petak manapun."));
    }
    for (int i = 0; i < 2; i++) {
        allSkillCards.push_back(make_unique<LassoCard>("LassoCard", "Tarik lawan ke posisimu."));
    }
    for (int i = 0; i < 2; i++) {
        allSkillCards.push_back(
            make_unique<DemolitionCard>("DemolitionCard", "Hancurkan satu bangunan lawan."));
    }

    vector<SkillCard*> skillRaw;
    for (auto& c : allSkillCards)
        skillRaw.push_back(c.get());
    auto skillDeck = make_unique<CardDeck<SkillCard>>(skillRaw);

    // ── Property tiles (IDs in property.txt are 1-based board positions) ──────
    auto& propLoader = cfg.getPropertyLoader();
    auto& props = propLoader.getProperties();

    map<int, unique_ptr<PropertyTile>> propMap;
    for (auto& p : props) {
        int pos = p->getId() - 1; // convert to 0-indexed
        propMap[pos] = move(p);
    }

    // ── Place all 40 tiles ────────────────────────────────────────────────────
    for (int i = 0; i < 40; i++) {
        if (propMap.count(i)) {
            board->addTile(move(propMap[i]));
            continue;
        }
        switch (i) {
        case 0:
            board->addTile(make_unique<GoTile>(1, "GO", "Petak Mulai", config.goSalary));
            break;
        case 2:
            board->addTile(make_unique<CardTile>(3, "DNU", "Dana Umum", *communityDeck));
            break;
        case 4:
            board->addTile(make_unique<TaxTile>(5, "PPH", "Pajak Penghasilan", TaxType::PPH,
                                                config.pphFlat, config.pphPercentage));
            break;
        case 7:
            board->addTile(make_unique<FestivalTile>(8, "FES", "Festival"));
            break;
        case 10:
            board->addTile(make_unique<JailTile>(11, "PEN", "Penjara", config.jailFine));
            break;
        case 17:
            board->addTile(make_unique<CardTile>(18, "DNU", "Dana Umum", *communityDeck));
            break;
        case 20:
            board->addTile(make_unique<FreeTile>(21, "BBP", "Bebas Parkir"));
            break;
        case 22:
            board->addTile(make_unique<CardTile>(23, "KSP", "Kesempatan", *chanceDeck));
            break;
        case 30:
            board->addTile(make_unique<GoToJailTile>(31, "PPJ", "Pergi ke Penjara"));
            break;
        case 33:
            board->addTile(make_unique<FestivalTile>(34, "FES", "Festival"));
            break;
        case 36:
            board->addTile(make_unique<CardTile>(37, "KSP", "Kesempatan", *chanceDeck));
            break;
        case 38:
            board->addTile(make_unique<TaxTile>(39, "PBM", "Pajak Barang Mewah", TaxType::PBM,
                                                config.pbmFlat));
            break;
        default:
            break;
        }
    }

    return std::make_tuple(move(board), move(chanceCards), move(communityCards),
                           move(allSkillCards), move(chanceDeck), move(communityDeck),
                           move(skillDeck));
}
