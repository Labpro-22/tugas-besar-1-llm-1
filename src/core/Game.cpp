#include "Game.hpp"
#include "Player.hpp"
#include "Board.hpp"
#include "Dice.hpp"
#include "Tile.hpp"
#include "PropertyTile.hpp"
#include "StreetTile.hpp"
#include "RailroadTile.hpp"
#include "UtilityTile.hpp"
#include "GoTile.hpp"
#include "JailTile.hpp"
#include "GoToJailTile.hpp"
#include "FreeTile.hpp"
#include "CardTile.hpp"
#include "TaxTile.hpp"
#include "FestivalTile.hpp"
#include "GameView.hpp"
#include "GameSaveLoader.hpp"
#include "ConfigManager.hpp"
#include "PropertyLoader.hpp"
#include "ChanceCard.hpp"
#include "CommunityChestCard.hpp"
#include "SkillCard.hpp"
#include "MoveCard.hpp"
#include "DiscountCard.hpp"
#include "ShieldCard.hpp"
#include "TeleportCard.hpp"
#include "LassoCard.hpp"
#include "DemolitionCard.hpp"
#include "ColorGroup.hpp"
#include "Exceptions.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <stdexcept>

using namespace std;

Game::Game()
    : state(GameState::MENU),
      lastDiceTotal(0),
      turnManager(0),
      view(nullptr) {
    dice = make_unique<Dice>();
    board = make_unique<Board>();
}

Game::~Game() {}

void Game::setView(GameView* gameView) {
    view = gameView;
}

// ── Helpers ──────────────────────────────────────────────────────────────────

vector<Player*> Game::getActivePlayers() const {
    vector<Player*> active;
    for (const auto& p : players) {
        if (p->getStatus() != PlayerStatus::BANKRUPT) {
            active.push_back(p.get());
        }
    }
    return active;
}

void Game::updateMonopolyStatus() {
    map<ColorGroup, vector<StreetTile*>> groupTiles;
    for (int i = 0; i < board->getTotalTiles(); i++) {
        if (auto* s = dynamic_cast<StreetTile*>(board->getTileAt(i))) {
            groupTiles[s->getColorGroup()].push_back(s);
        }
    }
    for (auto& kv : groupTiles) {
        auto& tiles = kv.second;
        Player* owner = tiles[0]->getOwner();
        bool mono = (owner != nullptr);
        for (auto* t : tiles) {
            if (t->getOwner() != owner) {
                mono = false;
                break;
            }
        }
        for (auto* t : tiles) t->setMonopolyOwned(mono && owner != nullptr);
    }
}

void Game::distributeSkillCards() {
    for (auto& p : players) {
        for (int i = 0; i < 2 && skillDeck && !skillDeck->isEmpty(); ++i) {
            SkillCard* card = skillDeck->draw();
            if (card) {
                try {
                    p->addCard(card);
                } catch (...) {
                    skillDeck->discard(card);
                }
            }
        }
    }
}

// ── Board construction ────────────────────────────────────────────────────────

void Game::buildBoard(ConfigManager& cfg) {
    // ── Card decks ──────────────────────────────────────────────────────────
    chanceCards.push_back(make_unique<ChanceCard>(
        "Ke Stasiun Terdekat", "Pergi ke stasiun terdekat.", ChanceEffect::GO_TO_NEAREST_STATION));
    chanceCards.push_back(make_unique<ChanceCard>(
        "Mundur 3 Petak", "Mundur 3 petak.", ChanceEffect::MOVE_BACK_3));
    chanceCards.push_back(make_unique<ChanceCard>(
        "Masuk Penjara", "Masuk Penjara.", ChanceEffect::GO_TO_JAIL));
    chanceCards.push_back(make_unique<ChanceCard>(
        "Bebas dari Penjara", "Gunakan kartu ini untuk keluar dari penjara.", ChanceEffect::GET_OUT_OF_JAIL));

    vector<ChanceCard*> chanceRaw;
    for (auto& c : chanceCards) chanceRaw.push_back(c.get());
    chanceDeck = make_unique<CardDeck<ChanceCard>>(chanceRaw);

    communityCards.push_back(make_unique<CommunityChestCard>(
        "Ulang Tahun",
        "Ini adalah hari ulang tahun Anda. Dapatkan M100 dari setiap pemain.",
        CommunityChestEffect::BIRTHDAY_COLLECT_100));
    communityCards.push_back(make_unique<CommunityChestCard>(
        "Biaya Dokter", "Biaya dokter. Bayar M700.", CommunityChestEffect::DOCTOR_FEE_700));
    communityCards.push_back(make_unique<CommunityChestCard>(
        "Nyaleg", "Anda mau nyaleg. Bayar M200 kepada setiap pemain.",
        CommunityChestEffect::CAMPAIGN_PAY_200));

    vector<CommunityChestCard*> communityRaw;
    for (auto& c : communityCards) communityRaw.push_back(c.get());
    communityDeck = make_unique<CardDeck<CommunityChestCard>>(communityRaw);

    // Skill cards: 4 Move, 3 Discount, 2 Shield, 2 Teleport, 2 Lasso, 2 Demolition
    mt19937 rng(random_device{}());
    uniform_int_distribution<int> stepsDist(1, 6);
    uniform_int_distribution<int> discDist(10, 50);

    for (int i = 0; i < 4; i++) {
        int steps = stepsDist(rng);
        allSkillCards.push_back(make_unique<MoveCard>(
            "MoveCard", "Maju " + to_string(steps) + " petak.", steps));
    }
    for (int i = 0; i < 3; i++) {
        int pct = discDist(rng);
        allSkillCards.push_back(make_unique<DiscountCard>(
            "DiscountCard", "Diskon " + to_string(pct) + "%.", pct, 1));
    }
    for (int i = 0; i < 2; i++) {
        allSkillCards.push_back(make_unique<ShieldCard>(
            "ShieldCard", "Kebal tagihan/sanksi selama 1 giliran.", 1));
    }
    for (int i = 0; i < 2; i++) {
        allSkillCards.push_back(make_unique<TeleportCard>(
            "TeleportCard", "Pindah ke petak manapun."));
    }
    for (int i = 0; i < 2; i++) {
        allSkillCards.push_back(make_unique<LassoCard>(
            "LassoCard", "Tarik lawan ke posisimu."));
    }
    for (int i = 0; i < 2; i++) {
        allSkillCards.push_back(make_unique<DemolitionCard>(
            "DemolitionCard", "Hancurkan satu bangunan lawan."));
    }

    vector<SkillCard*> skillRaw;
    for (auto& c : allSkillCards) skillRaw.push_back(c.get());
    skillDeck = make_unique<CardDeck<SkillCard>>(skillRaw);

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
                board->addTile(make_unique<TaxTile>(5, "PPH", "Pajak Penghasilan",
                                                     TaxType::PPH, config.pphFlat,
                                                     config.pphPercentage));
                break;
            case 7:
                board->addTile(make_unique<FestivalTile>(8, "FES", "Festival"));
                break;
            case 10:
                board->addTile(
                    make_unique<JailTile>(11, "PEN", "Penjara", config.jailFine));
                break;
            case 17:
                board->addTile(make_unique<CardTile>(18, "DNU", "Dana Umum", *communityDeck));
                break;
            case 20:
                board->addTile(make_unique<FreeTile>(21, "BBP", "Bebas Parkir"));
                break;
            case 22:
                board->addTile(
                    make_unique<CardTile>(23, "KSP", "Kesempatan", *chanceDeck));
                break;
            case 30:
                board->addTile(
                    make_unique<GoToJailTile>(31, "PPJ", "Pergi ke Penjara"));
                break;
            case 33:
                board->addTile(make_unique<FestivalTile>(34, "FES", "Festival"));
                break;
            case 36:
                board->addTile(
                    make_unique<CardTile>(37, "KSP", "Kesempatan", *chanceDeck));
                break;
            case 38:
                board->addTile(make_unique<TaxTile>(39, "PBM", "Pajak Barang Mewah",
                                                     TaxType::PBM, config.pbmFlat));
                break;
            default:
                break;
        }
    }
}

// ── Game lifecycle ────────────────────────────────────────────────────────────

void Game::resetGameData() {
    players.clear();
    chanceCards.clear();
    communityCards.clear();
    allSkillCards.clear();
    chanceDeck.reset();
    communityDeck.reset();
    skillDeck.reset();
    board = make_unique<Board>();
    dice = make_unique<Dice>();
    logger.clear();
    turnManager = TurnManager(0);
    lastDiceTotal = 0;
    state = GameState::MENU;
}

void Game::createGame() {
    resetGameData();
    ConfigManager cfg("config");
    cfg.loadAll();
    config = cfg.getConfig();
    turnManager = TurnManager(config.maxTurn);

    buildBoard(cfg);

    vector<string> names = view->promptPlayerSetup();
    for (int i = 0; i < static_cast<int>(names.size()); ++i) {
        players.push_back(make_unique<Player>(i, names[i]));
        *players.back() += config.startingBalance;
    }

    turnManager.initOrder(static_cast<int>(players.size()));
    state = GameState::PLAYING;
}

void Game::loadGame(const string& filename) {
    resetGameData();
    GameSaveLoader loader;
    loader.load(*this, filename);
    state = GameState::PLAYING;
}

void Game::saveGame(const string& filename) const {
    GameSaveLoader loader;
    loader.save(*this, filename);
}

void Game::runCycle() {
    if (state != GameState::PLAYING) return;
    Player& active = getActivePlayer();
    runTurn(active);
    if (active.getStatus() == PlayerStatus::BANKRUPT) {
        turnManager.removePlayer(active.getId());
    }
    auto activePlayers = getActivePlayers();
    if (activePlayers.size() <= 1) {
        state = GameState::GAMEOVER;
        return;
    }
    if (active.getStatus() != PlayerStatus::BANKRUPT) {
        turnManager.advance(activePlayers);
    }
    if (turnManager.hasReachedMaxTurn()) {
        state = GameState::GAMEOVER;
    }
}

bool Game::checkWin() const {
    return state == GameState::GAMEOVER;
}

// ── Per-turn logic ────────────────────────────────────────────────────────────

void Game::runTurn(Player& player) {
    player.resetTurnFlags();
    dice->resetDoublesCount();

    // Distribute 1 skill card at start of turn
    if (skillDeck && !skillDeck->isEmpty()) {
        SkillCard* newCard = skillDeck->draw();
        if (newCard) handleCardDrop(player, newCard);
    }

    // Tick turn-based card effects
    player.tickDiscount();
    player.tickShield();

    // Tick festival on player's own properties
    for (auto* prop : player.getProperties()) {
        prop->tickFestival();
    }

    // Jail handling replaces normal turn
    if (player.isInJail()) {
        bool extraTurnFromJail = handleJailTurn(player);
        if (!extraTurnFromJail) return;
        // Exited jail via double — grant one extra command loop
        dice->resetDoublesCount();
        player.setRolledDice(false);
        while (!player.getHasRolledDice()) {
            if (player.isInJail()) return;
            if (!view) return;
            string input = view->getCommandInput(player);
            handleCommand(input, player);
        }
        return;
    }

    // Command loop — may cycle multiple times when player gets double
    bool extraTurn = false;
    do {
        extraTurn = false;
        player.setRolledDice(false);

        while (!player.getHasRolledDice()) {
            if (player.isInJail()) return;
            if (!view) return;
            string input = view->getCommandInput(player);
            handleCommand(input, player);
        }

        if (player.isInJail()) return;

        if (dice->isDouble()) {
            extraTurn = true;
            cout << "Double! " << player.getUsername() << " mendapat giliran tambahan.\n";
        }
    } while (extraTurn);
}

bool Game::handleJailTurn(Player& player) {
    cout << "\n=== " << player.getUsername() << " berada di Penjara ===\n";
    int jailTurns = player.getJailTurns();
    cout << "Giliran penjara ke-" << (jailTurns + 1) << "\n";

    // Turn 4 (jailTurns == 3): forced payment
    if (jailTurns >= 3) {
        cout << "Giliran ke-4 di penjara. Wajib membayar denda M" << config.jailFine << ".\n";
        chargeVoluntary(player, config.jailFine);
        player.releaseFromJail();
        cout << player.getUsername() << " keluar dari penjara.\n";
        // Take regular roll
        lastDiceTotal = dice->rollRandom();
        auto dv = dice->getDiceValues();
        cout << "Dadu: " << dv.first << " + " << dv.second << " = " << lastDiceTotal << "\n";
        logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "DADU",
                        "Lempar: " + to_string(dv.first) + "+" + to_string(dv.second) + "=" +
                            to_string(lastDiceTotal) + " (keluar penjara, bayar denda)");
        movePlayerBy(player, lastDiceTotal);
        return false;
    }

    cout << "Opsi keluar dari penjara:\n";
    cout << "1. BAYAR - bayar denda M" << config.jailFine << "\n";
    cout << "2. LEMPAR - coba lempar double\n";
    if (player.hasJailFreeCard()) {
        cout << "3. KARTU - gunakan kartu Bebas dari Penjara\n";
    }
    cout << "Pilih (BAYAR/LEMPAR" << (player.hasJailFreeCard() ? "/KARTU" : "") << "): ";

    string choice;
    getline(cin, choice);
    transform(choice.begin(), choice.end(), choice.begin(), ::toupper);

    if (choice == "KARTU" || choice == "3") {
        if (!player.hasJailFreeCard()) {
            cout << "Kamu tidak memiliki kartu Bebas dari Penjara.\n";
            player.incrementJailTurns();
            return false;
        }
        player.useJailFreeCard();
        player.releaseFromJail();
        cout << player.getUsername() << " menggunakan kartu Bebas dari Penjara dan keluar dari penjara!\n";
        logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "PENJARA",
                        "Keluar penjara via kartu Bebas dari Penjara");
        lastDiceTotal = dice->rollRandom();
        auto dv = dice->getDiceValues();
        cout << "Dadu: " << dv.first << " + " << dv.second << " = " << lastDiceTotal << "\n";
        movePlayerBy(player, lastDiceTotal);
        return false;
    } else if (choice == "BAYAR" || choice == "1") {
        chargeVoluntary(player, config.jailFine);
        if (player.getStatus() == PlayerStatus::ACTIVE) {
            player.releaseFromJail();
            cout << player.getUsername() << " membayar M" << config.jailFine
                 << " dan keluar dari penjara.\n";
            logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "PENJARA",
                            "Bayar denda M" + to_string(config.jailFine) + " dan keluar penjara");
            lastDiceTotal = dice->rollRandom();
            auto dv = dice->getDiceValues();
            cout << "Dadu: " << dv.first << " + " << dv.second << " = " << lastDiceTotal << "\n";
            movePlayerBy(player, lastDiceTotal);
        }
        return false;
    } else {
        // Try to roll double
        dice->resetDoublesCount();
        lastDiceTotal = dice->rollRandom();
        auto dv = dice->getDiceValues();
        cout << "Dadu: " << dv.first << " + " << dv.second << " = " << lastDiceTotal << "\n";
        logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "DADU",
                        "Lempar di penjara: " + to_string(dv.first) + "+" +
                            to_string(dv.second) + "=" + to_string(lastDiceTotal));

        if (dice->isDouble()) {
            cout << "Double! " << player.getUsername() << " keluar dari penjara.\n";
            player.releaseFromJail();
            logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "PENJARA",
                            "Keluar penjara via double");
            movePlayerBy(player, lastDiceTotal);
            return true;
        } else {
            cout << "Tidak double. " << player.getUsername() << " tetap di penjara.\n";
            player.incrementJailTurns();
            return false;
        }
    }
}

void Game::handleCardDrop(Player& player, SkillCard* newCard) {
    if (!newCard) return;
    cout << "\nKamu mendapatkan 1 kartu acak baru!\n";
    cout << "Kartu yang didapat: " << newCard->getName() << " - " << newCard->getDescription()
         << "\n";

    try {
        player.addCard(newCard);
        cout << "Kartu ditambahkan ke tangan. Tangan: " << player.getHand().size()
             << " kartu.\n";
    } catch (const CardLimitException&) {
        cout << "PERINGATAN: Kamu sudah memiliki 3 kartu di tangan (Maksimal 3)!\n";
        cout << "Kamu diwajibkan membuang 1 kartu.\n";

        const auto& hand = player.getHand();
        cout << "Daftar Kartu Kemampuan Anda:\n";
        for (int i = 0; i < static_cast<int>(hand.size()); i++) {
            cout << (i + 1) << ". " << hand[i]->getName() << " - "
                 << hand[i]->getDescription() << "\n";
        }
        cout << (hand.size() + 1) << ". " << newCard->getName() << " - "
             << newCard->getDescription() << " [BARU]\n";
        cout << "Pilih nomor kartu yang ingin dibuang (1-" << (hand.size() + 1) << "): ";

        int dropChoice = 0;
        cin >> dropChoice;
        cin.ignore();

        if (dropChoice >= 1 && dropChoice <= static_cast<int>(hand.size())) {
            SkillCard* toDiscard = hand[dropChoice - 1];
            player.removeCard(toDiscard);
            skillDeck->discard(toDiscard);
            cout << toDiscard->getName() << " telah dibuang.\n";
            player.addCard(newCard);
        } else {
            // Discard the new card (also covers out-of-range)
            skillDeck->discard(newCard);
            cout << newCard->getName() << " (baru) telah dibuang.\n";
        }
        cout << "Sekarang kamu memiliki " << player.getHand().size() << " kartu di tangan.\n";
    }
}

// ── Command dispatcher ────────────────────────────────────────────────────────

void Game::handleCommand(const string& input, Player& player) {
    if (input.empty()) return;
    istringstream iss(input);
    string cmd;
    iss >> cmd;
    transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

    try {
        if (cmd == "CETAK_PAPAN") {
        if (view) view->printBoard(*board, players, getCurrentTurn(), config.maxTurn);

    } else if (cmd == "LEMPAR_DADU") {
        if (player.getHasRolledDice()) {
            throw GameStateException("Kamu sudah melempar dadu pada giliran ini.");
        } else {
            handleLemparDadu(player);
        }

    } else if (cmd == "ATUR_DADU") {
        if (player.getHasRolledDice()) {
            throw GameStateException("Kamu sudah melempar dadu pada giliran ini.");
        } else {
            int d1 = -1, d2 = -1;
            if (iss >> d1 >> d2) {
                handleLemparDadu(player, d1, d2);
            } else {
                cout << "Format: ATUR_DADU X Y (contoh: ATUR_DADU 3 4)\n";
            }
        }

    } else if (cmd == "CETAK_AKTA") {
        string code;
        if (!(iss >> code)) {
            cout << "Masukkan kode petak: ";
            cin >> code;
            cin.ignore();
        }
        Tile* t = board->getTileByCode(code);
        auto* prop = dynamic_cast<PropertyTile*>(t);
        if (prop) {
            if (view) view->printPropertyDeed(*prop);
        } else {
            throw InvalidPropertyException("Petak \"" + code + "\" tidak ditemukan atau bukan properti.");
        }

    } else if (cmd == "CETAK_PROPERTI") {
        if (view) view->printPlayerProperties(player);

    } else if (cmd == "GADAI") {
        handleGadai(player);

    } else if (cmd == "TEBUS") {
        handleTebus(player);

    } else if (cmd == "BANGUN") {
        handleBangun(player);

    } else if (cmd == "GUNAKAN_KEMAMPUAN") {
        handleGunakanKemampuan(player);

    } else if (cmd == "CETAK_LOG") {
        int n = -1;
        iss >> n;
        auto logs = logger.getRecentLogs(n);
        if (view) view->printTransactionLogs(logs);

    } else if (cmd == "SIMPAN") {
        if (player.getHasRolledDice()) {
            throw GameStateException("SIMPAN hanya bisa dilakukan di awal giliran (sebelum melempar dadu).");
        } else {
            string filename;
            if (!(iss >> filename)) filename = "nimonspoli_save.txt";
            // Check if file already exists — spec requires overwrite prompt
            {
                ifstream checkFile(filename);
                if (checkFile.good()) {
                    checkFile.close();
                    cout << "File '" << filename << "' sudah ada. Timpa file lama? (y/n): ";
                    char response;
                    cin >> response;
                    cin.ignore();
                    if (response != 'y' && response != 'Y') {
                        cout << "Penyimpanan dibatalkan.\n";
                        return;
                    }
                }
            }
            saveGame(filename);
            cout << "Permainan berhasil disimpan ke: " << filename << "\n";
            logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "SIMPAN",
                            "Simpan ke " + filename);
        }

    } else {
        throw InvalidCommandException("Perintah tidak dikenali: " + cmd +
             ". Ketik CETAK_PAPAN, LEMPAR_DADU, ATUR_DADU X Y, CETAK_AKTA, "
             "CETAK_PROPERTI, GADAI, TEBUS, BANGUN, GUNAKAN_KEMAMPUAN, "
             "CETAK_LOG [N], SIMPAN [FILE].");
    }
    } catch (const GameException& e) {
        cout << e.what() << "\n";
    }
}

// ── Command handlers (private) ────────────────────────────────────────────────

void Game::handleLemparDadu(Player& player, int d1, int d2) {
    try {
        if (d1 < 0) {
            lastDiceTotal = dice->rollRandom();
        } else {
            lastDiceTotal = dice->rollManual(d1, d2);
        }
    } catch (const exception& e) {
        cout << "Error dadu: " << e.what() << "\n";
        return;
    }

    player.setRolledDice(true);
    auto dv = dice->getDiceValues();
    cout << "\nMengocok dadu...\n";
    cout << "Hasil: " << dv.first << " + " << dv.second << " = " << lastDiceTotal << "\n";

    logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "DADU",
                    "Lempar: " + to_string(dv.first) + "+" + to_string(dv.second) + "=" +
                        to_string(lastDiceTotal));

    if (dice->isSpeedingViolation()) {
        cout << player.getUsername() << " melanggar batas kecepatan (3 double berturut)!"
             << " Masuk Penjara!\n";
        sendPlayerToJail(player);
        logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "PENJARA",
                        "Speeding violation - masuk penjara");
        return;
    }

    cout << "Memajukan bidak " << player.getUsername() << " sebanyak " << lastDiceTotal
         << " petak...\n";
    movePlayerBy(player, lastDiceTotal);
}

void Game::handleGadai(Player& player) {
    vector<PropertyTile*> mortgageable;
    for (PropertyTile* prop : player.getProperties()) {
        if (prop->getStatus() == PropertyStatus::OWNED) mortgageable.push_back(prop);
    }

    if (mortgageable.empty()) {
        throw GameStateException("Tidak ada properti yang dapat digadaikan saat ini.");
    }

    cout << "=== Properti yang Dapat Digadaikan ===\n";
    for (int i = 0; i < static_cast<int>(mortgageable.size()); i++) {
        cout << (i + 1) << ". " << mortgageable[i]->getName() << " ("
             << mortgageable[i]->getCode() << ")  Nilai Gadai: M"
             << mortgageable[i]->getMortgageValue() << "\n";
    }
    cout << "Pilih nomor properti (0 untuk batal): ";

    int choice = 0;
    cin >> choice;
    cin.ignore();
    if (choice < 1 || choice > static_cast<int>(mortgageable.size())) {
        if (choice == 0) return;
        throw InvalidCommandException("Nomor properti tidak valid untuk digadaikan.");
    }

    PropertyTile* selected = mortgageable[choice - 1];
    auto* selectedStreet = dynamic_cast<StreetTile*>(selected);

    if (selectedStreet) {
        ColorGroup cg = selectedStreet->getColorGroup();
        bool hasBuildings = false;
        vector<StreetTile*> groupStreets;
        for (int i = 0; i < board->getTotalTiles(); i++) {
            auto* s = dynamic_cast<StreetTile*>(board->getTileAt(i));
            if (s && s->getColorGroup() == cg && s->getOwner() == &player) {
                groupStreets.push_back(s);
                if (s->getPropertyLevel() > 0) hasBuildings = true;
            }
        }
        if (hasBuildings) {
            cout << selected->getName() << " tidak dapat digadaikan! Masih ada bangunan di "
                    "color group ["
                 << colorGroupToString(cg) << "].\n";
            cout << "Jual semua bangunan color group [" << colorGroupToString(cg)
                 << "]? (y/n): ";
            char resp;
            cin >> resp;
            cin.ignore();
            if (resp != 'y' && resp != 'Y') {
                cout << "Dibatalkan.\n";
                return;
            }
            for (auto* s : groupStreets) {
                int lvl = s->getPropertyLevel();
                if (lvl == 0) continue;
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
                logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "JUAL_BANGUNAN",
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
    logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "GADAI",
                    "Gadai " + selected->getName() + " M" + to_string(selected->getMortgageValue()));
}

void Game::handleTebus(Player& player) {
    vector<PropertyTile*> mortgaged;
    for (PropertyTile* prop : player.getProperties()) {
        if (prop->isMortgaged()) mortgaged.push_back(prop);
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
        if (choice == 0) return;
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
    logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "TEBUS",
                    "Tebus " + selected->getName() + " M" + to_string(redeemPrice));
}

void Game::handleBangun(Player& player) {
    vector<ColorGroup> monoGroups = player.getMonopolyGroups();
    if (monoGroups.empty()) {
        throw GameStateException("Tidak ada color group yang memenuhi syarat untuk dibangun. (Belum Memonopoli)");
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
        for (int i = 0; i < board->getTotalTiles(); i++) {
            auto* s = dynamic_cast<StreetTile*>(board->getTileAt(i));
            if (s && s->getColorGroup() == cg && s->getOwner() == &player &&
                !s->isMortgaged()) {
                streets.push_back(s);
                if (s->getPropertyLevel() < 5) anyCanBuild = true;
            }
        }
        if (!streets.empty() && anyCanBuild) buildableGroups.push_back({cg, streets});
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
        if (groupChoice == 0) return;
        throw InvalidCommandException("Color group tidak valid.");
    }

    auto& selectedGroup = buildableGroups[groupChoice - 1];

    // Pemerataan: find minimum level in the group
    int minLevel = 5;
    for (auto* s : selectedGroup.streets) minLevel = min(minLevel, s->getPropertyLevel());

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
        throw GameStateException("Tidak ada petak yang dapat dibangun saat ini (semua sudah hotel).");
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
        if (buildChoice == 0) return;
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
    logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "BANGUN", logDetail);
}

void Game::handleGunakanKemampuan(Player& player) {
    if (player.getHasUsedSkillThisTurn()) {
        throw GameStateException("Penggunaan kartu dibatasi maksimal 1 kali dalam 1 giliran.");
    }
    if (player.getHasRolledDice()) {
        throw GameStateException("Kartu kemampuan hanya bisa digunakan SEBELUM melempar dadu.");
    }
    const auto& hand = player.getHand();
    if (hand.empty()) {
        throw GameStateException("Kamu tidak memiliki kartu kemampuan.");
    }

    cout << "Daftar Kartu Kemampuan Spesial Anda:\n";
    for (int i = 0; i < static_cast<int>(hand.size()); i++) {
        cout << (i + 1) << ". " << hand[i]->getName() << " - " << hand[i]->getDescription()
             << "\n";
    }
    cout << "0. Batal\n";
    cout << "Pilih kartu (0-" << hand.size() << "): ";

    int choice = 0;
    cin >> choice;
    cin.ignore();
    if (choice < 1 || choice > static_cast<int>(hand.size())) {
        if (choice == 0) return;
        throw InvalidCommandException("Nomor kartu tidak valid.");
    }

    SkillCard* card = hand[choice - 1];
    string cardName = card->getName();
    player.removeCard(card);
    card->executeAction(*this);
    skillDeck->discard(card);
    player.setUsedSkillThisTurn(true);
    logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "KARTU",
                    "Gunakan " + cardName);
}

// ── IGameContext: state queries ───────────────────────────────────────────────

Player& Game::getActivePlayer() {
    int idx = turnManager.getActivePlayerIndex();
    return *players[idx];
}

const vector<Player*> Game::getAllActivePlayers() const {
    return getActivePlayers();
}

Board& Game::getBoard() {
    return *board;
}

const GameConfig& Game::getConfig() const {
    return config;
}

int Game::getCurrentTurn() const {
    return turnManager.getCurrentTurn();
}

Logger& Game::getLogger() {
    return logger;
}

int Game::getLastDiceTotal() const {
    return lastDiceTotal;
}

// ── Public accessors ──────────────────────────────────────────────────────────

const vector<unique_ptr<Player>>& Game::getPlayers() const {
    return players;
}

const TurnManager& Game::getTurnManager() const {
    return turnManager;
}

const vector<unique_ptr<SkillCard>>& Game::getAllSkillCards() const {
    return allSkillCards;
}

GameState Game::getState() const {
    return state;
}

// ── IGameContext: player movement ─────────────────────────────────────────────

void Game::movePlayerBy(Player& player, int steps) {
    int oldPos = player.getPosition();
    int newPos = board->getNewPosition(oldPos, steps);
    bool passGo = steps > 0 && (oldPos + steps >= board->getTotalTiles());
    player.setPosition(newPos);

    Tile* t = board->getTileAt(newPos);
    string tileName = t ? t->getName() : "?";
    cout << "Bidak mendarat di: " << tileName << ".\n";
    logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "GERAK",
                    "Posisi " + to_string(oldPos) + " -> " + to_string(newPos) + " (" +
                        tileName + ")");

    if (passGo) grantSalary(player);
    if (t) t->landedOn(*this);
}

void Game::movePlayerTo(Player& player, int tileIndex) {
    int oldPos = player.getPosition();
    bool passGo = (tileIndex < oldPos);
    player.setPosition(tileIndex);

    Tile* t = board->getTileAt(tileIndex);
    string tileName = t ? t->getName() : "?";
    cout << "Bidak pindah ke: " << tileName << ".\n";
    logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "GERAK",
                    "Teleport " + to_string(oldPos) + " -> " + to_string(tileIndex) + " (" +
                        tileName + ")");

    if (passGo) grantSalary(player);
    if (t) t->landedOn(*this);
}

void Game::repositionPlayer(Player& player, int tileIndex) {
    int oldPos = player.getPosition();
    player.setPosition(tileIndex);

    Tile* t = board->getTileAt(tileIndex);
    string tileName = t ? t->getName() : "?";
    cout << "Bidak dipindahkan ke: " << tileName << ".\n";
    logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "GERAK",
                    "Reposisi " + to_string(oldPos) + " -> " + to_string(tileIndex) + " (" +
                        tileName + ")");
}
void Game::movePlayerToNearest(Player& player, const string& tileType) {
    int current = player.getPosition();
    int total = board->getTotalTiles();
    for (int i = 1; i < total; ++i) {
        int pos = (current + i) % total;
        Tile* t = board->getTileAt(pos);
        if (!t) continue;
        auto* prop = dynamic_cast<PropertyTile*>(t);
        if (prop && prop->getType() == PropertyType::RAILROAD) {
            movePlayerTo(player, pos);
            return;
        }
    }
}

void Game::sendPlayerToJail(Player& player) {
    player.goToJail();
    player.setPosition(board->getJailPosition());
    dice->resetDoublesCount();
    cout << player.getUsername() << " masuk penjara!\n";
    logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "PENJARA",
                    "Masuk penjara di petak " + to_string(board->getJailPosition()));
}

// ── IGameContext: financial operations ───────────────────────────────────────

void Game::grantSalary(Player& player) {
    player += config.goSalary;
    cout << player.getUsername() << " melewati Petak Mulai! Terima gaji M" << config.goSalary
         << ". Uang: M" << player.getMoney() << "\n";
    logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "GAJI",
                    "Terima gaji GO M" + to_string(config.goSalary));
}

void Game::transferMoney(Player& payer, Player& collector, int amount) {
    if (payer.isShielded()) {
        cout << "[SHIELD] " << payer.getUsername() << " dilindungi dari tagihan M" << amount
             << "!\n";
        return;
    }
    if (payer.getMoney() < amount) {
        triggerBankruptcy(payer, &collector, amount);
        return;
    }
    payer -= amount;
    collector += amount;
    cout << payer.getUsername() << " membayar M" << amount << " ke " << collector.getUsername()
         << ". Uang " << payer.getUsername() << ": M" << payer.getMoney() << "\n";
    logger.logEvent(LogLevel::INFO, getCurrentTurn(), payer.getUsername(), "TRANSFER",
                    "Bayar M" + to_string(amount) + " ke " + collector.getUsername());
}

void Game::chargeToBank(Player& player, int amount) {
    if (player.isShielded()) {
        cout << "[SHIELD] " << player.getUsername() << " dilindungi dari biaya M" << amount
             << "!\n";
        return;
    }
    if (player.getMoney() < amount) {
        triggerBankruptcy(player, nullptr, amount);
        return;
    }
    player -= amount;
    cout << player.getUsername() << " membayar M" << amount << " ke Bank. Uang: M"
         << player.getMoney() << "\n";
    logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "BANK",
                    "Bayar ke Bank M" + to_string(amount));
}

void Game::chargeVoluntary(Player& player, int amount) {
    if (player.getMoney() < amount) {
        triggerBankruptcy(player, nullptr, amount);
        return;
    }
    player -= amount;
    cout << player.getUsername() << " membayar M" << amount << " ke Bank. Uang: M"
         << player.getMoney() << "\n";
    logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "BANK",
                    "Bayar sukarela M" + to_string(amount));
}

void Game::collectFromAll(Player& collector, int amountPerPlayer) {
    for (Player* p : getActivePlayers()) {
        if (p == &collector) continue;
        transferMoney(*p, collector, amountPerPlayer);
    }
}

void Game::payToAll(Player& payer, int amountPerPlayer) {
    for (Player* p : getActivePlayers()) {
        if (p == &payer) continue;
        transferMoney(payer, *p, amountPerPlayer);
    }
}

// ── IGameContext: property operations ────────────────────────────────────────

void Game::grantProperty(Player& player, PropertyTile& tile) {
    // Remove from previous owner if any
    if (tile.getOwner() && tile.getOwner() != &player) {
        tile.getOwner()->removeProperty(&tile);
    }
    tile.setOwner(&player);
    player.addProperty(&tile);
    updateMonopolyStatus();
    cout << tile.getName() << " kini menjadi milik " << player.getUsername() << ".\n";
    logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "PROPERTI",
                    "Dapat " + tile.getName() + " (" + tile.getCode() + ")");
}

void Game::initiateAuction(PropertyTile& tile) {
    const auto& turnOrder = turnManager.getTurnOrder();
    if (turnOrder.empty()) return;

    int triggerId = getActivePlayer().getId();
    int triggerPos = 0;
    for (int i = 0; i < static_cast<int>(turnOrder.size()); ++i) {
        if (turnOrder[i] == triggerId) {
            triggerPos = i;
            break;
        }
    }

    vector<Player*> bidderOrder;
    for (int offset = 1; offset <= static_cast<int>(turnOrder.size()); ++offset) {
        int playerId = turnOrder[(triggerPos + offset) % static_cast<int>(turnOrder.size())];
        Player* candidate = players[playerId].get();
        if (candidate->getStatus() != PlayerStatus::BANKRUPT) {
            bidderOrder.push_back(candidate);
        }
    }

    auctionManager.runAuction(tile, bidderOrder, *this);
}

void Game::triggerBankruptcy(Player& debtor, Player* creditor, int amount) {
    GameState prevState = state;
    state = GameState::LIQUIDATION;
    bankruptcyHandler.handle(debtor, creditor, amount, *this);
    state = prevState;
}

void Game::refreshMonopolyStatus() {
    updateMonopolyStatus();
}

// ── IGameContext: UI-mediated interactions ────────────────────────────────────

bool Game::promptBuyProperty(Player& player, PropertyTile& tile) {
    if (!view) return false;
    cout << "\nKamu mendarat di " << tile.getName() << " (" << tile.getCode() << ")!\n";
    if (view) view->printPropertyDeed(tile);
    cout << "Uang kamu saat ini: M" << player.getMoney() << "\n";
    cout << "Apakah kamu ingin membeli properti ini seharga M" << tile.getPrice()
         << "? (y/n): ";
    char c;
    cin >> c;
    cin.ignore();
    return (c == 'y' || c == 'Y');
}

PropertyTile* Game::promptSelectOpponentProperty(Player& player) {
    if (!view) return nullptr;
    vector<pair<Player*, PropertyTile*>> opts;
    for (auto& p : players) {
        if (p.get() == &player || p->getStatus() == PlayerStatus::BANKRUPT) continue;
        for (PropertyTile* prop : p->getProperties()) {
            auto* s = dynamic_cast<StreetTile*>(prop);
            if (s && s->getPropertyLevel() > 0) opts.push_back({p.get(), prop});
        }
    }
    if (opts.empty()) {
        cout << "Tidak ada properti lawan yang memiliki bangunan.\n";
        return nullptr;
    }
    cout << "=== Pilih Properti Lawan untuk Dihancurkan ===\n";
    for (int i = 0; i < static_cast<int>(opts.size()); i++) {
        auto* s = dynamic_cast<StreetTile*>(opts[i].second);
        int lvl = s ? s->getPropertyLevel() : 0;
        string lvlStr = (lvl == 5) ? "Hotel" : to_string(lvl) + " rumah";
        cout << (i + 1) << ". " << opts[i].second->getName() << " (" << opts[i].second->getCode()
             << ") milik " << opts[i].first->getUsername() << " - " << lvlStr << "\n";
    }
    cout << "0. Batal\nPilih: ";
    int c = 0;
    cin >> c;
    cin.ignore();
    if (c < 1 || c > static_cast<int>(opts.size())) return nullptr;
    return opts[c - 1].second;
}

Player* Game::promptSelectTarget(Player& player) {
    if (!view) return nullptr;
    const int boardSize = board->getTotalTiles();
    const int myPos = player.getPosition();
    vector<Player*> targets;
    for (auto& p : players) {
        if (p.get() == &player || p->getStatus() == PlayerStatus::BANKRUPT) continue;
        // Only allow targets that are strictly ahead (circularly)
        int dist = (p->getPosition() - myPos + boardSize) % boardSize;
        if (dist == 0) continue;
        targets.push_back(p.get());
    }
    if (targets.empty()) {
        cout << "Tidak ada pemain lawan yang berada di depan posisi kamu.\n";
        return nullptr;
    }
    cout << "=== Pilih Target Pemain (di depan posisi kamu) ===\n";
    for (int i = 0; i < static_cast<int>(targets.size()); i++) {
        cout << (i + 1) << ". " << targets[i]->getUsername() << " (posisi "
             << targets[i]->getPosition() << ")\n";
    }
    cout << "0. Batal\nPilih: ";
    int c = 0;
    cin >> c;
    cin.ignore();
    if (c < 1 || c > static_cast<int>(targets.size())) return nullptr;
    return targets[c - 1];
}

int Game::promptTaxChoice(Player& player, int flat, int pct) {
    if (!view) return 1;
    cout << "\nKamu mendarat di Pajak Penghasilan (PPH)!\n";
    cout << "Pilih opsi pembayaran pajak:\n";
    cout << "1. Bayar flat M" << flat << "\n";
    cout << "2. Bayar " << pct << "% dari total kekayaan\n";
    cout << "Pilihan (1/2): ";
    int choice = 1;
    cin >> choice;
    cin.ignore();
    if (choice == 2) {
        // Wealth is calculated AFTER the player has chosen — per spec requirement
        int totalWealth = player.getTotalWealth();
        int percentageAmount = totalWealth * pct / 100;
        cout << "Total kekayaan kamu:\n";
        cout << "  Uang tunai          : M" << player.getMoney() << "\n";
        cout << "  Harga beli properti : M" << player.getPropertyValue() << "\n";
        cout << "  Total               : M" << totalWealth << "\n";
        cout << "Pajak " << pct << "%             : M" << percentageAmount << "\n";
    }
    return (choice == 2) ? 2 : 1;
}

int Game::promptTileIndex(Player& player) {
    if (!view) return 0;
    cout << "=== Pilih Petak Tujuan (TeleportCard) ===\n";
    for (int i = 0; i < board->getTotalTiles(); i++) {
        Tile* t = board->getTileAt(i);
        if (t) cout << i << ". " << t->getCode() << " - " << t->getName() << "\n";
    }
    cout << "Masukkan nomor petak (0-" << (board->getTotalTiles() - 1) << "): ";
    int idx = 0;
    cin >> idx;
    cin.ignore();
    if (idx < 0 || idx >= board->getTotalTiles()) idx = 0;
    return idx;
}

void Game::promptFestivalSelection(Player& player) {
    if (!view) return;
    const auto& props = player.getProperties();
    if (props.empty()) {
        cout << "Kamu tidak memiliki properti yang dapat dipilih.\n";
        return;
    }
    cout << "\nKamu mendarat di petak Festival!\n";
    cout << "Daftar properti milikmu:\n";
    for (int i = 0; i < static_cast<int>(props.size()); i++) {
        auto* p = props[i];
        string festInfo = "";
        if (p->hasFestival()) {
            festInfo = " [Festival aktif x" + to_string(p->getFestivalMultiplier()) +
                       ", sisa " + to_string(p->getFestivalDur()) + " giliran]";
        }
        cout << (i + 1) << ". " << p->getName() << " (" << p->getCode() << ")"
             << festInfo << "\n";
    }
    cout << "Masukkan nomor properti (0 untuk skip): ";
    int choice = 0;
    cin >> choice;
    cin.ignore();
    if (choice < 1 || choice > static_cast<int>(props.size())) {
        cout << "Efek festival tidak diterapkan.\n";
        return;
    }
    PropertyTile* selected = props[choice - 1];
    int oldMult = selected->getFestivalMultiplier();
    selected->activateFestival();
    int newMult = selected->getFestivalMultiplier();

    if (newMult > oldMult) {
        cout << "Efek festival aktif! Sewa " << selected->getName() << ": x" << oldMult
             << " -> x" << newMult << ". Durasi: 3 giliran.\n";
        logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "FESTIVAL",
                        selected->getName() + ": sewa x" + to_string(newMult) +
                            ", durasi 3 giliran");
    } else {
        cout << "Efek sudah maksimum (harga sewa sudah digandakan tiga kali). "
                "Durasi di-reset menjadi 3 giliran.\n";
        logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "FESTIVAL",
                        selected->getName() + ": durasi reset 3 giliran (max)");
    }
}

pair<bool, int> Game::promptAuctionBid(Player& player, int currentBid,
                                        const PropertyTile& tile) {
    while (true) {
        cout << "Giliran: " << player.getUsername() << "\n";
        cout << "Aksi (PASS / BID <jumlah>) > ";
        string line;
        getline(cin, line);
        istringstream iss(line);
        string cmd;
        iss >> cmd;
        transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
        if (cmd == "BID") {
            int amount = 0;
            if (iss >> amount && amount > currentBid && amount <= player.getMoney()) {
                return {true, amount};
            }
            if (amount <= currentBid)
                cout << "Penawaran harus lebih tinggi dari M" << currentBid << ".\n";
            else
                cout << "Uang kamu tidak cukup (M" << player.getMoney() << ").\n";
            // Re-prompt on invalid bid
            continue;
        }
        if (cmd == "PASS") {
            return {false, 0};
        }
        cout << "Perintah tidak valid. Gunakan PASS atau BID <jumlah>.\n";
    }
}

void Game::runLiquidationPanel(Player& debtor, int amountNeeded, Player* creditor) {
    cout << "\nKamu tidak dapat membayar M" << amountNeeded << "!\n";
    cout << "Uang kamu       : M" << debtor.getMoney() << "\n";
    cout << "Total kewajiban : M" << amountNeeded << "\n";
    cout << "Kekurangan      : M" << (amountNeeded - debtor.getMoney()) << "\n";
    cout << "\nDana likuidasi dapat menutup kewajiban. Kamu wajib melikuidasi aset.\n";

    while (debtor.getMoney() < amountNeeded) {
        cout << "\n=== Panel Likuidasi ===\n";
        cout << "Uang kamu saat ini: M" << debtor.getMoney()
             << "  |  Kewajiban: M" << amountNeeded << "\n";

        vector<PropertyTile*> sellable, mortgageable;
        for (auto* prop : debtor.getProperties()) {
            if (prop->isMortgaged()) continue;
            sellable.push_back(prop);
        }
        for (auto* prop : debtor.getProperties()) {
            if (prop->getStatus() == PropertyStatus::OWNED) mortgageable.push_back(prop);
        }

        if (sellable.empty() && mortgageable.empty()) {
            cout << "Tidak ada aset yang dapat dilikuidasi.\n";
            break;
        }

        cout << "[Jual ke Bank]\n";
        for (int i = 0; i < static_cast<int>(sellable.size()); i++) {
            auto* s = dynamic_cast<StreetTile*>(sellable[i]);
            int sellVal = sellable[i]->getPrice();
            if (s) {
                int lvl = s->getPropertyLevel();
                if (lvl == 5) {
                    sellVal += (s->getHotelPrice() + 4 * s->getHousePrice()) / 2;
                } else {
                    sellVal += lvl * s->getHousePrice() / 2;
                }
            }
            cout << (i + 1) << ". " << sellable[i]->getName() << " (" << sellable[i]->getCode()
                 << ")  Harga Jual: M" << sellVal << "\n";
        }
        cout << "[Gadaikan]\n";
        for (int i = 0; i < static_cast<int>(mortgageable.size()); i++) {
            cout << (static_cast<int>(sellable.size()) + i + 1) << ". "
                 << mortgageable[i]->getName() << " (" << mortgageable[i]->getCode()
                 << ")  Nilai Gadai: M" << mortgageable[i]->getMortgageValue() << "\n";
        }
        cout << "Pilih aksi (0 jika sudah cukup): ";

        int choice = 0;
        cin >> choice;
        cin.ignore();

        if (choice == 0) {
            cout << "Kamu belum boleh keluar dari panel likuidasi sebelum kewajiban terpenuhi.\n";
            continue;
        }

        if (choice >= 1 && choice <= static_cast<int>(sellable.size())) {
            auto* prop = sellable[choice - 1];
            auto* s = dynamic_cast<StreetTile*>(prop);
            int sellVal = prop->getPrice();
            if (s) {
                int lvl = s->getPropertyLevel();
                if (lvl == 5) {
                    sellVal += (s->getHotelPrice() + 4 * s->getHousePrice()) / 2;
                } else {
                    sellVal += lvl * s->getHousePrice() / 2;
                }
                s->setPropertyLevel(0);
            }
            debtor.removeProperty(prop);
            prop->releaseToBank();
            debtor += sellVal;
            updateMonopolyStatus();
            cout << prop->getName() << " terjual ke Bank. Kamu menerima M" << sellVal << ".\n";
            logger.logEvent(LogLevel::INFO, getCurrentTurn(), debtor.getUsername(), "JUAL",
                            "Jual " + prop->getName() + " M" + to_string(sellVal));
        } else {
            int mIdx = choice - static_cast<int>(sellable.size()) - 1;
            if (mIdx >= 0 && mIdx < static_cast<int>(mortgageable.size())) {
                auto* prop = mortgageable[mIdx];
                prop->mortgage();
                debtor += prop->getMortgageValue();
                cout << prop->getName() << " digadaikan. Kamu menerima M"
                     << prop->getMortgageValue() << ".\n";
                logger.logEvent(LogLevel::INFO, getCurrentTurn(), debtor.getUsername(), "GADAI",
                                "Gadai " + prop->getName() + " M" +
                                    to_string(prop->getMortgageValue()));
            }
        }
    }

    // Execute payment
    if (debtor.getMoney() >= amountNeeded) {
        debtor -= amountNeeded;
        if (creditor) {
            *creditor += amountNeeded;
            cout << "Kewajiban M" << amountNeeded << " terpenuhi. Membayar ke "
                 << creditor->getUsername() << "...\n";
            cout << "Uang " << debtor.getUsername() << ": M" << debtor.getMoney() << "\n";
            cout << "Uang " << creditor->getUsername() << ": M" << creditor->getMoney() << "\n";
        } else {
            cout << "Kewajiban M" << amountNeeded << " terpenuhi. Membayar ke Bank...\n";
        }
        return;
    }

    triggerBankruptcy(debtor, creditor, amountNeeded);
}
