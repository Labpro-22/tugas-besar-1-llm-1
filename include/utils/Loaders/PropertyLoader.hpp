#include "Loader.hpp"
#include <PropertyTile.hpp>

/// @brief Loader class responsible for loading property tile configurations from a file.
class PropertyLoader : public Loader
{
private:
  /// @brief A collection of property tile pointers loaded from the file.
  std::vector<std::unique_ptr<PropertyTile>> properties;

public:
  PropertyLoader(std::string filename);
  ~PropertyLoader();

  /// @brief Loads property tile configurations from the specified file.
  void loadConfig() override;

  /// @brief Retrieves the loaded property tiles.
  /// @return A vector of unique pointers to the loaded property tiles.
  std::vector<std::unique_ptr<PropertyTile>> &getProperties();
};