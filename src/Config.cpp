#include "Analysis.hpp"
#include "Config.hpp"

namespace OpenABL {

bool Config::getBool(
    const std::string &name, bool defaultValue) const {
  auto it = config.find(name);
  if (it == config.end()) {
    return defaultValue;
  }

  Value v = Value::fromString(it->second);
  if (!v.isBool()) {
    throw ConfigError("Value of " + name + " must be boolean");
  }

  return v.getBool();
}

}
