#include "Backend.hpp"
#include "CPrinter.hpp"
#include "FileUtil.hpp"

namespace OpenABL {

void CBackend::generate(
    AST::Script &script, const std::string &outputDir, const std::string &assetDir) {
  CPrinter printer(script);
  printer.print(script);
  writeToFile(outputDir + "/main.c", printer.extractStr());
  copyFile(assetDir + "/c/libabl.h", outputDir + "/libabl.h");
  copyFile(assetDir + "/c/libabl.c", outputDir + "/libabl.c");
  copyFile(assetDir + "/c/build.sh", outputDir + "/build.sh");
  makeFileExecutable(outputDir + "/build.sh");
}

}
