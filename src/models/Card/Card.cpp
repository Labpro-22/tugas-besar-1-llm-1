#include "Card.hpp"

static int nextCardId = 1;

Card::Card(const std::string &name, const std::string &description)
    : id(nextCardId++), name(name), description(description)
{
}

Card::~Card() {}

int Card::getId() const
{
    return id;
}

const std::string &Card::getName() const
{
    return name;
}

const std::string &Card::getDescription() const
{
    return description;
}
