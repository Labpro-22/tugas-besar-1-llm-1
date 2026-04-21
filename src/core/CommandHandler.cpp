#include "CommandHandler.hpp"
#include "Board.hpp"
#include "Deck.hpp"
#include "Dice.hpp"
#include "Exceptions.hpp"
#include "GameConfig.hpp"
#include "GameView.hpp"
#include "IGameContext.hpp"
#include "Logger.hpp"
#include "Player.hpp"
#include "PropertyManager.hpp"
#include "PropertyTile.hpp"
#include "SkillCard.hpp"
#include "Tile.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

void CommandHandler::dispatch(const string& input, Player& player, IGameContext& ctx,
                              PropertyManager& propMgr, Board& board, Dice& dice,
                              CardDeck<SkillCard>* skillDeck, GameView* view, Logger& logger,
                              int& lastDiceTotal, SaveCallback saveCallback,
                              const vector<unique_ptr<Player>>& players) {
    if (input.empty())
        return;
    istringstream iss(input);
    string cmd;
    iss >> cmd;
    transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

    try {
        if (cmd == "CETAK_PAPAN") {
            if (view)
                view->printBoard(board, players, ctx.getCurrentTurn(), ctx.getConfig().maxTurn);

        } else if (cmd == "LEMPAR_DADU") {
            if (player.getHasRolledDice()) {
                throw GameStateException("Kamu sudah melempar dadu pada giliran ini.");
            } else {
                handleLemparDadu(player, dice, ctx, logger, lastDiceTotal);
            }

        } else if (cmd == "ATUR_DADU") {
            if (player.getHasRolledDice()) {
                throw GameStateException("Kamu sudah melempar dadu pada giliran ini.");
            } else {
                int d1 = -1, d2 = -1;
                if (iss >> d1 >> d2) {
                    handleLemparDadu(player, dice, ctx, logger, lastDiceTotal, d1, d2);
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
            Tile* t = board.getTileByCode(code);
            auto* prop = dynamic_cast<PropertyTile*>(t);
            if (prop) {
                if (view)
                    view->printPropertyDeed(*prop);
            } else {
                throw InvalidPropertyException("Petak \"" + code +
                                               "\" tidak ditemukan atau bukan properti.");
            }

        } else if (cmd == "CETAK_PROPERTI") {
            if (view)
                view->printPlayerProperties(player);

        } else if (cmd == "GADAI") {
            propMgr.mortgage(player, board, ctx.getCurrentTurn(), logger);

        } else if (cmd == "TEBUS") {
            propMgr.redeem(player, ctx.getCurrentTurn(), logger);

        } else if (cmd == "BANGUN") {
            propMgr.build(player, board, ctx.getCurrentTurn(), logger);

        } else if (cmd == "GUNAKAN_KEMAMPUAN") {
            handleGunakanKemampuan(player, ctx, skillDeck, logger);

        } else if (cmd == "CETAK_LOG") {
            int n = -1;
            iss >> n;
            auto logs = logger.getRecentLogs(n);
            if (view)
                view->printTransactionLogs(logs);

        } else if (cmd == "SIMPAN") {
            if (player.getHasRolledDice()) {
                throw GameStateException(
                    "SIMPAN hanya bisa dilakukan di awal giliran (sebelum melempar dadu).");
            } else {
                string filename;
                if (!(iss >> filename))
                    filename = "nimonspoli_save.txt";

                if (filename.find("data/") != 0) {
                    filename = "data/" + filename;
                }
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
                saveCallback(filename);
                cout << "Permainan berhasil disimpan ke: " << filename << "\n";
                logger.logEvent(LogLevel::INFO, ctx.getCurrentTurn(), player.getUsername(),
                                "SIMPAN", "Simpan ke " + filename);
            }

        } else {
            throw InvalidCommandException(
                "Perintah tidak dikenali: " + cmd +
                ". Ketik CETAK_PAPAN, LEMPAR_DADU, ATUR_DADU X Y, CETAK_AKTA, "
                "CETAK_PROPERTI, GADAI, TEBUS, BANGUN, GUNAKAN_KEMAMPUAN, "
                "CETAK_LOG [N], SIMPAN [FILE].");
        }
    } catch (const GameException& e) {
        cout << e.what() << "\n";
    }
}

void CommandHandler::handleLemparDadu(Player& player, Dice& dice, IGameContext& ctx, Logger& logger,
                                      int& lastDiceTotal, int d1, int d2) {
    try {
        if (d1 < 0) {
            lastDiceTotal = dice.rollRandom();
        } else {
            lastDiceTotal = dice.rollManual(d1, d2);
        }
    } catch (const exception& e) {
        cout << "Error dadu: " << e.what() << "\n";
        return;
    }

    player.setRolledDice(true);
    auto dv = dice.getDiceValues();
    cout << "\nMengocok dadu...\n";
    cout << "Hasil: " << dv.first << " + " << dv.second << " = " << lastDiceTotal << "\n";

    logger.logEvent(LogLevel::INFO, ctx.getCurrentTurn(), player.getUsername(), "DADU",
                    "Lempar: " + to_string(dv.first) + "+" + to_string(dv.second) + "=" +
                        to_string(lastDiceTotal));

    if (dice.isSpeedingViolation()) {
        cout << player.getUsername() << " melanggar batas kecepatan (3 double berturut)!"
             << " Masuk Penjara!\n";
        ctx.sendPlayerToJail(player);
        logger.logEvent(LogLevel::INFO, ctx.getCurrentTurn(), player.getUsername(), "PENJARA",
                        "Speeding violation - masuk penjara");
        return;
    }

    cout << "Memajukan bidak " << player.getUsername() << " sebanyak " << lastDiceTotal
         << " petak...\n";
    ctx.movePlayerBy(player, lastDiceTotal);
}

void CommandHandler::handleGunakanKemampuan(Player& player, IGameContext& ctx,
                                            CardDeck<SkillCard>* skillDeck, Logger& logger) {
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
        cout << (i + 1) << ". " << hand[i]->getName() << " - " << hand[i]->getDescription() << "\n";
    }
    cout << "0. Batal\n";
    cout << "Pilih kartu (0-" << hand.size() << "): ";

    int choice = 0;
    cin >> choice;
    cin.ignore();
    if (choice < 1 || choice > static_cast<int>(hand.size())) {
        if (choice == 0)
            return;
        throw InvalidCommandException("Nomor kartu tidak valid.");
    }

    SkillCard* card = hand[choice - 1];
    string cardName = card->getName();
    player.removeCard(card);
    card->executeAction(ctx);
    skillDeck->discard(card);
    player.setUsedSkillThisTurn(true);
    logger.logEvent(LogLevel::INFO, ctx.getCurrentTurn(), player.getUsername(), "KARTU",
                    "Gunakan " + cardName);
}
