#include "ActionTile.hpp"

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