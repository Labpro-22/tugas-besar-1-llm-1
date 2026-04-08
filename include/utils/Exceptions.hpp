#include <exception>
#include <string>

class GameException : public std::exception
{
protected:
  std::string message;

public:
  GameException(const std::string &msg) : message(msg) {}
  const char *what() const noexcept override { return message.c_str(); }
};

class InsufficientFundsException : public GameException
{
public:
  InsufficientFundsException(int required, int available);
};

class InvalidPropertyException : public GameException
{
public:
  InvalidPropertyException(const std::string &reason);
};

class CardLimitException : public GameException
{
public:
  CardLimitException();
};

class InvalidCommandException : public GameException
{
public:
  InvalidCommandException(const std::string &command);
};

class FileException : public GameException
{
public:
  FileException(const std::string &filename, const std::string &operation);
};

class GameStateException : public GameException
{
public:
  GameStateException(const std::string &reason);
};
