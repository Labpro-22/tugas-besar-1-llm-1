#include "Game.hpp"
#include "Board.hpp"
#include "BoardFactory.hpp"
#include "ChanceCard.hpp"
#include "ColorGroup.hpp"
#include "CommandHandler.hpp"
#include "CommunityChestCard.hpp"
#include "ConfigManager.hpp"
#include "Dice.hpp"
#include "Exceptions.hpp"
#include "FinanceManager.hpp"
#include "GameSaveLoader.hpp"
#include "GameView.hpp"
#include "Player.hpp"
#include "PropertyManager.hpp"
#include "PropertyTile.hpp"
#include "RailroadTile.hpp"
#include "SkillCard.hpp"
#include "StreetTile.hpp"
#include "Tile.hpp"
#include "UtilityTile.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

Game::Game() : state(GameState::MENU), lastDiceTotal(0), turnManager(0), view(nullptr) {
    dice = make_unique<Dice>();
    board = make_unique<Board>();
}

Game::~Game() = default;

void Game::setView(GameView* gameView) {
    view = gameView;
}

// ── Helpers ──────────────────────────────────────────────────────────────────

vector<Player*> Game::getActivePlayers() const {
    vector<Player*> active;
    for (auto& p : players) {
        if (p->getStatus() != PlayerStatus::BANKRUPT)
            active.push_back(p.get());
    }
    return active;
}

void Game::updateMonopolyStatus() {
    if (!board)
        return;
    for (int i = 0; i < board->getTotalTiles(); i++) {
        auto* prop = dynamic_cast<PropertyTile*>(board->getTileAt(i));
        if (!prop)
            continue;
        auto* street = dynamic_cast<StreetTile*>(prop);
        if (!street)
            continue;

        ColorGroup cg = street->getColorGroup();
        Player* owner = street->getOwner();
        if (!owner) {
            street->setMonopolyOwned(false);
            continue;
        }

        bool allOwn = true;
        for (int j = 0; j < board->getTotalTiles(); j++) {
            auto* other = dynamic_cast<StreetTile*>(board->getTileAt(j));
            if (other && other->getColorGroup() == cg && other->getOwner() != owner) {
                allOwn = false;
                break;
            }
        }
        street->setMonopolyOwned(allOwn);
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

    // Delegate board construction to BoardFactory
    auto [newBoard, newChanceCards, newCommunityCards, newAllSkillCards, newChanceDeck,
          newCommunityDeck, newSkillDeck] = BoardFactory::build(cfg);
    board = move(newBoard);
    chanceCards = move(newChanceCards);
    communityCards = move(newCommunityCards);
    allSkillCards = move(newAllSkillCards);
    chanceDeck = move(newChanceDeck);
    communityDeck = move(newCommunityDeck);
    skillDeck = move(newSkillDeck);

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
    if (state != GameState::PLAYING)
        return;
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
        if (newCard)
            handleCardDrop(player, newCard);
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
        if (!extraTurnFromJail)
            return;
        // Exited jail via double — grant one extra command loop
        dice->resetDoublesCount();
        player.setRolledDice(false);
        while (!player.getHasRolledDice()) {
            if (player.isInJail())
                return;
            if (!view)
                return;
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
            if (player.isInJail())
                return;
            if (!view)
                return;
            string input = view->getCommandInput(player);
            handleCommand(input, player);
        }

        if (player.isInJail())
            return;

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
        cout << player.getUsername()
             << " menggunakan kartu Bebas dari Penjara dan keluar dari penjara!\n";
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
                        "Lempar di penjara: " + to_string(dv.first) + "+" + to_string(dv.second) +
                            "=" + to_string(lastDiceTotal));

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
    if (!newCard)
        return;
    cout << "\nKamu mendapatkan 1 kartu acak baru!\n";
    cout << "Kartu yang didapat: " << newCard->getName() << " - " << newCard->getDescription()
         << "\n";

    try {
        player.addCard(newCard);
        cout << "Kartu ditambahkan ke tangan. Tangan: " << player.getHand().size() << " kartu.\n";
    } catch (const CardLimitException&) {
        cout << "PERINGATAN: Kamu sudah memiliki 3 kartu di tangan (Maksimal 3)!\n";
        cout << "Kamu diwajibkan membuang 1 kartu.\n";

        const auto& hand = player.getHand();
        cout << "Daftar Kartu Kemampuan Anda:\n";
        for (int i = 0; i < static_cast<int>(hand.size()); i++) {
            cout << (i + 1) << ". " << hand[i]->getName() << " - " << hand[i]->getDescription()
                 << "\n";
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

// ── Command dispatcher (delegates to CommandHandler) ──────────────────────────

void Game::handleCommand(const string& input, Player& player) {
    commandHandler.dispatch(
        input, player, *this, propertyManager, *board, *dice, skillDeck.get(), view, logger,
        lastDiceTotal, [this](const string& filename) { saveGame(filename); }, players);
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
                    "Posisi " + to_string(oldPos) + " -> " + to_string(newPos) + " (" + tileName +
                        ")");

    if (passGo)
        grantSalary(player);
    if (t)
        t->landedOn(*this);
}

void Game::movePlayerTo(Player& player, int tileIndex) {
    int oldPos = player.getPosition();
    bool passGo = (tileIndex < oldPos);
    player.setPosition(tileIndex);

    Tile* t = board->getTileAt(tileIndex);
    string tileName = t ? t->getName() : "?";
    cout << "Bidak dipindahkan ke: " << tileName << ".\n";
    logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "GERAK",
                    "Posisi " + to_string(oldPos) + " -> " + to_string(tileIndex) + " (" +
                        tileName + ")");

    if (passGo)
        grantSalary(player);
    if (t)
        t->landedOn(*this);
}

void Game::repositionPlayer(Player& player, int tileIndex) {
    int oldPos = player.getPosition();
    player.setPosition(tileIndex);

    Tile* t = board->getTileAt(tileIndex);
    string tileName = t ? t->getName() : "?";
    cout << player.getUsername() << " dipindahkan ke " << tileName << " (tanpa efek).\n";
    logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "REPOSITION",
                    "Posisi " + to_string(oldPos) + " -> " + to_string(tileIndex) + " (" +
                        tileName + ")");
}

void Game::movePlayerToNearest(Player& player, const string& tileType) {
    int current = player.getPosition();
    int total = board->getTotalTiles();
    for (int i = 1; i < total; ++i) {
        int pos = (current + i) % total;
        Tile* t = board->getTileAt(pos);
        if (!t)
            continue;
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

// ── IGameContext: financial operations (delegated to FinanceManager) ─────────

void Game::grantSalary(Player& player) {
    financeManager.grantSalary(player, config, getCurrentTurn(), logger);
}

void Game::transferMoney(Player& payer, Player& collector, int amount) {
    financeManager.transferMoney(payer, collector, amount, getCurrentTurn(), logger, *this);
}

void Game::chargeToBank(Player& player, int amount) {
    financeManager.chargeToBank(player, amount, getCurrentTurn(), logger, *this);
}

void Game::chargeVoluntary(Player& player, int amount) {
    financeManager.chargeVoluntary(player, amount, getCurrentTurn(), logger, *this);
}

void Game::collectFromAll(Player& collector, int amountPerPlayer) {
    financeManager.collectFromAll(collector, amountPerPlayer, getActivePlayers(), getCurrentTurn(),
                                  logger, *this);
}

void Game::payToAll(Player& payer, int amountPerPlayer) {
    financeManager.payToAll(payer, amountPerPlayer, getActivePlayers(), getCurrentTurn(), logger,
                            *this);
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
    if (turnOrder.empty())
        return;

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
    if (!view)
        return false;
    cout << "\nKamu mendarat di " << tile.getName() << " (" << tile.getCode() << ")!\n";
    if (view)
        view->printPropertyDeed(tile);
    cout << "Uang kamu saat ini: M" << player.getMoney() << "\n";
    cout << "Apakah kamu ingin membeli properti ini seharga M" << tile.getPrice() << "? (y/n): ";
    char c;
    cin >> c;
    cin.ignore();
    return (c == 'y' || c == 'Y');
}

PropertyTile* Game::promptSelectOpponentProperty(Player& player) {
    if (!view)
        return nullptr;
    vector<pair<Player*, PropertyTile*>> opts;
    for (auto& p : players) {
        if (p.get() == &player || p->getStatus() == PlayerStatus::BANKRUPT)
            continue;
        for (PropertyTile* prop : p->getProperties()) {
            auto* s = dynamic_cast<StreetTile*>(prop);
            if (s && s->getPropertyLevel() > 0)
                opts.push_back({p.get(), prop});
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
    if (c < 1 || c > static_cast<int>(opts.size()))
        return nullptr;
    return opts[c - 1].second;
}

Player* Game::promptSelectTarget(Player& player) {
    if (!view)
        return nullptr;
    const int boardSize = board->getTotalTiles();
    const int myPos = player.getPosition();
    vector<Player*> targets;
    for (auto& p : players) {
        if (p.get() == &player || p->getStatus() == PlayerStatus::BANKRUPT)
            continue;
        // Only allow targets that are strictly ahead (circularly)
        int dist = (p->getPosition() - myPos + boardSize) % boardSize;
        if (dist == 0)
            continue;
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
    if (c < 1 || c > static_cast<int>(targets.size()))
        return nullptr;
    return targets[c - 1];
}

int Game::promptTaxChoice(Player& player, int flat, int pct) {
    if (!view)
        return 1;
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
    if (!view)
        return 0;
    cout << "=== Pilih Petak Tujuan (TeleportCard) ===\n";
    for (int i = 0; i < board->getTotalTiles(); i++) {
        Tile* t = board->getTileAt(i);
        if (t)
            cout << i << ". " << t->getCode() << " - " << t->getName() << "\n";
    }
    cout << "Masukkan nomor petak (0-" << (board->getTotalTiles() - 1) << "): ";
    int idx = 0;
    cin >> idx;
    cin.ignore();
    if (idx < 0 || idx >= board->getTotalTiles())
        idx = 0;
    return idx;
}

void Game::promptFestivalSelection(Player& player) {
    if (!view)
        return;
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
            festInfo = " [Festival aktif x" + to_string(p->getFestivalMultiplier()) + ", sisa " +
                       to_string(p->getFestivalDur()) + " giliran]";
        }
        cout << (i + 1) << ". " << p->getName() << " (" << p->getCode() << ")" << festInfo << "\n";
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
        cout << "Efek festival aktif! Sewa " << selected->getName() << ": x" << oldMult << " -> x"
             << newMult << ". Durasi: 3 giliran.\n";
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

pair<bool, int> Game::promptAuctionBid(Player& player, int currentBid, const PropertyTile& tile) {
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
        cout << "Uang kamu saat ini: M" << debtor.getMoney() << "  |  Kewajiban: M" << amountNeeded
             << "\n";

        vector<PropertyTile*> sellable, mortgageable;
        for (auto* prop : debtor.getProperties()) {
            if (prop->isMortgaged())
                continue;
            sellable.push_back(prop);
        }
        for (auto* prop : debtor.getProperties()) {
            if (prop->getStatus() == PropertyStatus::OWNED)
                mortgageable.push_back(prop);
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
