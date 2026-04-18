#include "Dice.hpp"
#include <cstdlib>
#include <ctime>
#include <stdexcept>

using namespace std;

Dice::Dice() : die1(0), die2(0), consecutiveDoubles(0) {
    srand(static_cast<unsigned int>(time(nullptr)));
}

Dice::~Dice() {}

int Dice::rollRandom() {
    die1 = (rand() % 6) + 1;
    die2 = (rand() % 6) + 1;

    if (isDouble()) {
        consecutiveDoubles++;
    } else {
        consecutiveDoubles = 0;
    }

    return getTotal();
}

int Dice::rollManual(int d1, int d2) {
    if (d1 < 1 || d1 > 6 || d2 < 1 || d2 > 6) {
        throw invalid_argument("Nilai dadu harus antara 1 dan 6.");
    }

    die1 = d1;
    die2 = d2;

    if (isDouble()) {
        consecutiveDoubles++;
    } else {
        consecutiveDoubles = 0;
    }

    return getTotal();
}

int Dice::getTotal() const {
    return die1 + die2;
}

bool Dice::isDouble() const {
    return die1 == die2;
}

bool Dice::isSpeedingViolation() const {
    return consecutiveDoubles >= 3;
}

void Dice::resetDoublesCount() {
    consecutiveDoubles = 0;
}

pair<int, int> Dice::getDiceValues() const {
    return {die1, die2};
}
