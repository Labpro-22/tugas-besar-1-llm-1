#include <string>
#include <fstream>

class Loader
{
private:
  std::string filename;
  std::filebuf fileBuffer;

public:
  void virtual loadConfig() = 0;

  Loader(std::string filename);
  ~Loader();
};

Loader::Loader(std::string filename) : filename(filename)
{
}

Loader::~Loader()
{
}
