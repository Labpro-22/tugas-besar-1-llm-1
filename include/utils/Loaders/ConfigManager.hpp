#pragma once
#include "GameConfig.hpp"
#include <memory>
#include <string>

class PropertyLoader;

/**
 * @brief Facade over all config loaders.
 * Reads all config files from a directory and assembles a GameConfig value object.
 * Also exposes the loaded property tiles for Board construction.
 */
class ConfigManager {
private:
    std::string configDir;
    GameConfig config;

    std::unique_ptr<PropertyLoader> propertyLoader;

public:
    explicit ConfigManager(const std::string& configDir);
    ~ConfigManager();

    /// @brief Load all config files. Throws FileException on failure.
    void loadAll();

    const GameConfig& getConfig() const;
    PropertyLoader& getPropertyLoader() const;
};
