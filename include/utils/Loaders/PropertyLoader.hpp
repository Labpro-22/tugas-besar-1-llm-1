#include "Loader.hpp"
#include <PropertyTile.hpp>

class PropertyLoader : public Loader
{
private:
  std::vector<PropertyTile *> properties;

public:
  PropertyLoader(std::string filename);
  ~PropertyLoader();
};

PropertyLoader::PropertyLoader(std::string filename) : Loader(filename)
{
}

PropertyLoader::~PropertyLoader()
{
}
