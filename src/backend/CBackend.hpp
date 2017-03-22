#pragma once

#include <string>
#include "AST.hpp"
#include "Backend.hpp"
#include "Printer.hpp"
#include "CPrinter.hpp"
#include "FileUtil.hpp"

namespace OpenABL {

struct CBackend : public Backend {
  void generate(AST::Script &script, const std::string outputDir) {
    CPrinter printer;
    printer.print(script);
    writeToFile(outputDir + "/main.c", printer.extractStr());
    copyFile("asset/c/libabl.h", outputDir + "/libabl.h");
    copyFile("asset/c/libabl.c", outputDir + "/libabl.c");
    copyFile("asset/c/build.sh", outputDir + "/build.sh");
    makeFileExecutable(outputDir + "/build.sh");
  }
};

}
