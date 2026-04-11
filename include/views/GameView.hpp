#pragma once

#include <string>
#include <vector>
#include <memory>
#include "Board.hpp"
#include "Player.hpp"
#include "PropertyTile.hpp"
#include "TransactionLogger.hpp"

/// @brief Determines which UI element is being displayed at a given time in the lifecycle.
enum class ViewState
{
    MAIN_MENU,
    IN_GAME,
    AUCTION_PANEL,
    LIQUIDATION_PANEL,
    GAME_OVER_SCREEN
};

/// @brief A terminal-based presentation layer that renders the game state and handles user inputs.
class GameView
{
private:
    /// @brief Clears the screen dynamically based on the terminal operating system.
    void clearScreen() const;

public:
    /// @brief Creates the View interface component.
    GameView();
    ~GameView();

    /// @brief Prints the main menu of the game where users can start a new game or load a saved one.
    void displayMainMenu() const;

    /// @brief Prompts for username settings and number of players at the beginning of the setup.
    /// @return A vector of strings containing player names.
    std::vector<std::string> promptPlayerSetup() const;

    /// @brief Renders the visual representation of the game board.
    /// @param board Const reference to the board logic.
    /// @param players Const reference to the list of players on the board.
    void printBoard(const Board& board, const std::vector<std::unique_ptr<Player>>& players) const;

    /// @brief Renders an individual property tile detail and its ownership statuses.
    /// @param property Referencing a property tile component.
    void printPropertyDeed(const PropertyTile& property) const;

    /// @brief Renders the available properties controlled by a precise player.
    /// @param player The protagonist user owning the properties.
    void printPlayerProperties(const Player& player) const;

    /// @brief Displays an ongoing auction panel layout.
    /// @param currentBid The highest ongoing offered price.
    /// @param highBidder The username of the user leading the auction.
    /// @param tile The property tile currently in the auction round.
    void renderAuctionPanel(int currentBid, const std::string& highBidder, const PropertyTile& tile) const;

    /// @brief Renders the interactive command prompt containing context actions for a specific player's turn.
    /// @param activePlayer Reference to the player whose turn is ongoing.
    /// @return String representation of the submitted and trimmed input.
    std::string getCommandInput(const Player& activePlayer) const;

    /// @brief Prints the history log entries to the terminal console.
    /// @param logs Collection of string histories formatted in lines.
    void printTransactionLogs(const std::vector<std::string>& logs) const;

    /// @brief Declares visually the ending sequences and rankings.
    /// @param winners A collection of the winner player instances or names.
    /// @param finalRankings Collection storing leaderboard metadata string summaries.
    void showEndGameScreen(const std::vector<std::string>& winners, const std::vector<std::string>& finalRankings) const;
};
