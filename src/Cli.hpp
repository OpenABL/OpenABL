#include <map>
#include <string>

namespace OpenABL {
namespace Cli {

struct OptionError : public std::runtime_error {
  OptionError(const std::string &msg) : std::runtime_error(msg) {}
};

struct Options {
  bool help;
  bool lintOnly;
  bool build;
  bool run;
  std::string fileName;
  std::string backend;
  std::string outputDir;
  std::string assetDir;
  std::map<std::string, std::string> params;
  std::map<std::string, std::string> config;
};

Options parseOptions(int argc, char **argv);

}
}
