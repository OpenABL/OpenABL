#include "Backend.hpp"
#include "MasonPrinter.hpp"
#include "FileUtil.hpp"

namespace OpenABL {

void MasonBackend::generate(
    AST::Script &script, const std::string &outputDir, const std::string &assetDir) {
  (void) assetDir;

  MasonPrinter printer(script);
  printer.print(script);
  writeToFile(outputDir + "/Main.java", printer.extractStr());
}

}
