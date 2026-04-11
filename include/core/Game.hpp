#pragma once

#include <vector>
#include <string>
#include <memory>
#include "Player.hpp"
#include "Board.hpp"
#include "Dice.hpp"
#include "Deck.hpp"
#include "ChanceCard.hpp"
#include "CommunityChestCard.hpp"
#include "SkillCard.hpp"
#include "TransactionLogger.hpp"

/// @brief Determines the different statuses of the game cycle.
enum class GameState
{
    MENU,
    PLAYING,
    AUCTION,
    LIQUIDATION,
    GAMEOVER
};

/// @brief Oversees the main mechanics, game loop, rules, and turns of the Nimonspoli Game engine.
class Game
{
private:
    std::vector<std::unique_ptr<Player>> players;
    std::unique_ptr<Board> board;
    std::unique_ptr<Dice> dice;

    std::unique_ptr<CardDeck<ChanceCard>> chanceDeck;
    std::unique_ptr<CardDeck<CommunityChestCard>> communityDeck;
    std::unique_ptr<CardDeck<SkillCard>> skillDeck;

    GameState state;
    
    int activePlayerIndex;
    int currentTurn;
    int maxTurn;

public:
    /// @brief Creates a game engine, waiting for initiation.
    Game();
    ~Game();

    /// @brief Initializes a fresh game instance based on the input user configuration and settings.
    /// @param numPlayers Total players joining the new game.
    /// @param maxTurns Maximum turns specified in the configuration before it concludes.
    void createGame(int numPlayers, int maxTurns);

    /// @brief Initializes a game state from a save file logic.
    /// @param filename The path name to Load.
    void loadGame(const std::string& filename);

    /// @brief Saves the current logical game state to a save file logic.
    /// @param filename The intended file save location.
    void saveGame(const std::string& filename) const;

    /// @brief Progresses the game loop through player turns natively until finished.
    void runCycle();

    /// @brief Moves the turn progression to the next active player.
    void advanceTurn();

    /// @brief Verifies whether the game should declare a state of GAMEOVER.
    /// @return True if conditions were met, false otherwise.
    bool checkWin() const;

    /// @brief Handles bankruptcy liquidations.
    /// @param defaultingPlayer The player failing to pay their obligations.
    /// @param creditor The player retrieving assets from bankrupt player. Bank if nullptr.
    /// @param amountRequired The debt threshold that has been missed.
    void triggerBankruptcy(Player* defaultingPlayer, Player* creditor, int amountRequired);

    /// @brief Starts an auction event specifically for when properties are dismissed or un-bought.
    /// @param tile The property tile pending action.
    /// @param triggerPlayerIndex The index of the player provoking this auction round.
    void initiateAuction(Tile* tile, int triggerPlayerIndex);
};
