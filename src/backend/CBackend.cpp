#include "Backend.hpp"
#include "CPrinter.hpp"
#include "FileUtil.hpp"

namespace OpenABL {

static std::string generateBuildScript(bool useFloat) {
  if (useFloat) {
    return "gcc -O2 -std=c99 -DLIBABL_USE_FLOAT=1 main.c libabl.c -lm -fopenmp -o main";
  } else {
    return "gcc -O2 -std=c99 main.c libabl.c -lm -fopenmp -o main";
  }
}

void CBackend::generate(AST::Script &script, const BackendContext &ctx) {
  if (script.usesRuntimeRemoval || script.usesRuntimeAddition) {
    throw BackendError("The C backend does not support dynamic add/remove yet");
  }

  bool useFloat = ctx.config.getBool("use_float", false);

  CPrinter printer(script, useFloat);
  printer.print(script);
  writeToFile(ctx.outputDir + "/main.c", printer.extractStr());
  copyFile(ctx.assetDir + "/c/libabl.h", ctx.outputDir + "/libabl.h");
  copyFile(ctx.assetDir + "/c/libabl.c", ctx.outputDir + "/libabl.c");
  writeToFile(ctx.outputDir + "/build.sh", generateBuildScript(useFloat));
  copyFile(ctx.assetDir + "/c/run.sh", ctx.outputDir + "/run.sh");
  makeFileExecutable(ctx.outputDir + "/build.sh");
  makeFileExecutable(ctx.outputDir + "/run.sh");
}

}
