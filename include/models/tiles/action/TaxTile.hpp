#include "ActionTile.hpp"

/// @brief A tile that represents a tax tile where players have to pay tax when they land on it.
class TaxTile : public ActionTile
{
private:
  TaxType type;
  int baseAmount;
  int percentage;
public:
  TaxTile(/* args */);
  ~TaxTile();
};

TaxTile::TaxTile(/* args */)
{
}

TaxTile::~TaxTile()
{
}

enum class TaxType
{
  PPH,
  PBM
};