#pragma once
#include <string>
#include <fstream>

/// @brief Abstract base class for loading configurations from files, to be extended by specific loaders for different game components.
class Loader
{
private:
  /// @brief The filename to load from or save to.
  std::string filename;
  /// @brief File buffer for reading/writing operations.
  std::filebuf fileBuffer;

public:
  /// @brief Abstract method to be implemented by derived classes to load specific configurations.
  virtual void loadConfig() = 0;

  Loader(std::string filename);
  ~Loader();
};
