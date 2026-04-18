#pragma once
#include <vector>
#pragma once

/// @brief A deck of cards for managing the draw and discard piles.
/// @tparam T The type of card in the deck.
template <typename T> class CardDeck {
private:
    /// @brief The draw pile of the deck, containing cards that can be drawn by players.
    std::vector<T*> drawPile;

    /// @brief The discard pile of the deck, containing cards that have been used or discarded by
    /// players.
    std::vector<T*> discardPile;

public:
    /// @brief Initializes the card deck with a given set of cards.
    /// @param cards A vector of pointers to cards to initialize the deck with.
    CardDeck(const std::vector<T*>& cards);
    ~CardDeck();

    /// @brief Draws a card from the draw pile. If the draw pile is empty, reshuffles discard pile
    /// into draw pile.
    /// @return A pointer to the drawn card.
    T* draw();

    /// @brief Discards a card to the discard pile.
    /// @param card A pointer to the card to be discarded.
    void discard(T* card);

    /// @brief Shuffles the draw pile.
    void shuffle();

    /// @brief Reshuffles the discard pile into the draw pile if the draw pile is empty, do nothing
    /// otherwise.
    void reshuffleDiscardPile();

    /// @brief Checks if the draw pile is empty.
    /// @return True if the draw pile is empty, false otherwise.
    bool isEmpty() const;

    /// @brief Gets the number of cards in the draw pile.
    /// @return Number of cards in the draw pile.
    int drawPileSize() const;

    /// @brief Gets the number of cards in the discard pile.
    /// @return Number of cards in the discard pile.
    int discardPileSize() const;
};

template <typename T> CardDeck<T>::CardDeck(const std::vector<T*>& cards) : drawPile(cards) {}

template <typename T> CardDeck<T>::~CardDeck() {}
