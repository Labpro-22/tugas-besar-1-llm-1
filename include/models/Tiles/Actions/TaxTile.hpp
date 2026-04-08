#include "ActionTile.hpp"


/// @brief A tile that represents a tax tile where players have to pay tax when they land on it.
class TaxTile : public ActionTile
{
private:
  /// @brief The tax type of this tile, determines how the tax is calculated.
  TaxType type;

  /// @brief  The fixed base amount used in tax calculation for certain tax types.
  int baseAmount;

  /// @brief The percentage used in tax calculation for certain tax types.
  int percentage;

public:
  /// @brief Creates a tax tile with fixed tax configuration.
  /// @param id The unique identifier of the tile.
  /// @param code The unique 3-character code of the tile.
  /// @param name The display name of the tile.
  /// @param type The tax type of this tile.
  /// @param baseAmount The fixed base amount used in tax calculation.
  /// @param percentage The percentage used in tax calculation.
  TaxTile(const int id, const std::string &code, const std::string &name, TaxType type, int baseAmount, int percentage);
  ~TaxTile();
};

TaxTile::TaxTile(const int id, const std::string &code, const std::string &name, TaxType type, int baseAmount, int percentage)
    : ActionTile(id, code, name), type(type), baseAmount(baseAmount), percentage(percentage)
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