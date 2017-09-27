#include <map>

namespace OpenABL {

struct ConfigError : public std::runtime_error {
  ConfigError(const std::string &msg) : std::runtime_error(msg) {}
};

struct Config {
  std::map<std::string, std::string> config;

  bool getBool(const std::string &name, bool defaultValue) const;
  long getInt(const std::string &name, long defaultValue) const;
};

}
