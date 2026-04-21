#include "Exceptions.hpp"
#include "Game.hpp"
#include "GameView.hpp"
#include "Player.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

static void runEndScreen(GameView& view, const Game& game) {
    const auto& players = game.getPlayers();
    vector<const Player*> activePlayers;
    for (const auto& player : players) {
        if (player->getStatus() != PlayerStatus::BANKRUPT) {
            activePlayers.push_back(player.get());
        }
    }

    if (activePlayers.size() == 1) {
        vector<string> winners = {activePlayers.front()->getUsername()};
        vector<string> rankings;
        for (const auto& player : players) {
            rankings.push_back(player->getUsername() + " - cash M" + to_string(player->getMoney()) +
                               ", properti " + to_string(player->getProperties().size()) +
                               ", kartu " + to_string(player->getHand().size()));
        }
        view.showEndGameScreen(winners, rankings);
        return;
    }

    class PlayerSummary {
    public:
        string username;
        int cash;
        int propertyCount;
        int cardCount;
    };

    vector<PlayerSummary> summaries;
    for (const auto& p : players) {
        summaries.push_back({p->getUsername(), p->getMoney(),
                             static_cast<int>(p->getProperties().size()),
                             static_cast<int>(p->getHand().size())});
    }
    sort(summaries.begin(), summaries.end(),
         [](const PlayerSummary& left, const PlayerSummary& right) {
             if (left.cash != right.cash)
                 return left.cash > right.cash;
             if (left.propertyCount != right.propertyCount)
                 return left.propertyCount > right.propertyCount;
             if (left.cardCount != right.cardCount)
                 return left.cardCount > right.cardCount;
             return left.username < right.username;
         });

    vector<string> winners;
    if (!summaries.empty()) {
        const PlayerSummary& top = summaries.front();
        for (const auto& summary : summaries) {
            if (summary.cash == top.cash && summary.propertyCount == top.propertyCount &&
                summary.cardCount == top.cardCount) {
                winners.push_back(summary.username);
            }
        }
    }

    vector<string> rankings;
    for (int i = 0; i < static_cast<int>(summaries.size()); ++i) {
        rankings.push_back(to_string(i + 1) + ". " + summaries[i].username + " - cash M" +
                           to_string(summaries[i].cash) + ", properti " +
                           to_string(summaries[i].propertyCount) + ", kartu " +
                           to_string(summaries[i].cardCount));
    }
    view.showEndGameScreen(winners, rankings);
}

int main() {
    GameView view;
    Game game;
    game.setView(&view);

    while (true) {
        view.displayMainMenu();

        int choice = 0;
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            // New game
            try {
                game.createGame();
            } catch (const GameException& e) {
                cout << "Gagal memulai permainan: " << e.what() << "\n";
                continue;
            } catch (const exception& e) {
                cout << "Gagal memulai permainan: " << e.what() << "\n";
                continue;
            }

            while (!game.checkWin()) {
                game.runCycle();
            }
            runEndScreen(view, game);

        } else if (choice == 2) {
            // Load game
            cout << "Nama file save: ";
            string filename;
            cin >> filename;
            cin.ignore();

            if (filename.find("data/") != 0) {
                filename = "data/" + filename;
            }

            try {
                game.loadGame(filename);
            } catch (const GameException& e) {
                cout << "Gagal memuat: " << e.what() << "\n";
                continue;
            } catch (const exception& e) {
                cout << "Gagal memuat: " << e.what() << "\n";
                continue;
            }

            while (!game.checkWin()) {
                game.runCycle();
            }
            runEndScreen(view, game);

        } else if (choice == 3) {
            cout << "Sampai jumpa!\n";
            break;
        } else {
            cout << "Pilihan tidak valid. Masukkan 1, 2, atau 3.\n";
        }
    }

    return 0;
}
