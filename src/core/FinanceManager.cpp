#include "FinanceManager.hpp"
#include "GameConfig.hpp"
#include "IGameContext.hpp"
#include "Logger.hpp"
#include "Player.hpp"

#include <iostream>
#include <string>

using namespace std;

void FinanceManager::grantSalary(Player& player, const GameConfig& config, int currentTurn,
                                 Logger& logger) {
    player += config.goSalary;
    cout << player.getUsername() << " melewati Petak Mulai! Terima gaji M" << config.goSalary
         << ". Uang: M" << player.getMoney() << "\n";
    logger.logEvent(LogLevel::INFO, currentTurn, player.getUsername(), "GAJI",
                    "Terima gaji GO M" + to_string(config.goSalary));
}

void FinanceManager::transferMoney(Player& payer, Player& collector, int amount, int currentTurn,
                                   Logger& logger, IGameContext& ctx) {
    if (payer.isShielded()) {
        cout << "[SHIELD] " << payer.getUsername() << " dilindungi dari tagihan M" << amount
             << "!\n";
        return;
    }
    if (payer.getMoney() < amount) {
        ctx.triggerBankruptcy(payer, &collector, amount);
        return;
    }
    payer -= amount;
    collector += amount;
    cout << payer.getUsername() << " membayar M" << amount << " ke " << collector.getUsername()
         << ". Uang " << payer.getUsername() << ": M" << payer.getMoney() << "\n";
    logger.logEvent(LogLevel::INFO, currentTurn, payer.getUsername(), "TRANSFER",
                    "Bayar M" + to_string(amount) + " ke " + collector.getUsername());
}

void FinanceManager::chargeToBank(Player& player, int amount, int currentTurn, Logger& logger,
                                  IGameContext& ctx) {
    if (player.isShielded()) {
        cout << "[SHIELD] " << player.getUsername() << " dilindungi dari biaya M" << amount
             << "!\n";
        return;
    }
    if (player.getMoney() < amount) {
        ctx.triggerBankruptcy(player, nullptr, amount);
        return;
    }
    player -= amount;
    cout << player.getUsername() << " membayar M" << amount << " ke Bank. Uang: M"
         << player.getMoney() << "\n";
    logger.logEvent(LogLevel::INFO, currentTurn, player.getUsername(), "BANK",
                    "Bayar ke Bank M" + to_string(amount));
}

void FinanceManager::chargeVoluntary(Player& player, int amount, int currentTurn, Logger& logger,
                                     IGameContext& ctx) {
    if (player.getMoney() < amount) {
        ctx.triggerBankruptcy(player, nullptr, amount);
        return;
    }
    player -= amount;
    cout << player.getUsername() << " membayar M" << amount << " ke Bank. Uang: M"
         << player.getMoney() << "\n";
    logger.logEvent(LogLevel::INFO, currentTurn, player.getUsername(), "BANK",
                    "Bayar sukarela M" + to_string(amount));
}

void FinanceManager::collectFromAll(Player& collector, int amountPerPlayer,
                                    const vector<Player*>& activePlayers, int currentTurn,
                                    Logger& logger, IGameContext& ctx) {
    for (Player* p : activePlayers) {
        if (p == &collector)
            continue;
        transferMoney(*p, collector, amountPerPlayer, currentTurn, logger, ctx);
    }
}

void FinanceManager::payToAll(Player& payer, int amountPerPlayer,
                              const vector<Player*>& activePlayers, int currentTurn, Logger& logger,
                              IGameContext& ctx) {
    for (Player* p : activePlayers) {
        if (p == &payer)
            continue;
        transferMoney(payer, *p, amountPerPlayer, currentTurn, logger, ctx);
    }
}
