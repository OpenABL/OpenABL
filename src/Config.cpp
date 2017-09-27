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

long Config::getInt(
    const std::string &name, long defaultValue) const {
  auto it = config.find(name);
  if (it == config.end()) {
    return defaultValue;
  }

  Value v = Value::fromString(it->second);
  if (!v.isInt()) {
    throw ConfigError("Value of " + name + " must be integer");
  }

  return v.getInt();
}

}
