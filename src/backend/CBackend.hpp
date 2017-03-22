#pragma once

#include <string>
#include "AST.hpp"
#include "Backend.hpp"
#include "Printer.hpp"
#include "CPrinter.hpp"
#include "FileUtil.hpp"

namespace OpenABL {

struct CBackend : public Backend {
  void generate(AST::Script &script, const std::string &outputDir, const std::string &assetDir) {
    CPrinter printer;
    printer.print(script);
    writeToFile(outputDir + "/main.c", printer.extractStr());
    copyFile(assetDir + "/c/libabl.h", outputDir + "/libabl.h");
    copyFile(assetDir + "/c/libabl.c", outputDir + "/libabl.c");
    copyFile(assetDir + "/c/build.sh", outputDir + "/build.sh");
    makeFileExecutable(outputDir + "/build.sh");
  }
};

}
