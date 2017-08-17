#include "Backend.hpp"
#include "CPrinter.hpp"
#include "FileUtil.hpp"

namespace OpenABL {

void CBackend::generate(AST::Script &script, const BackendContext &ctx) {
  CPrinter printer(script);
  printer.print(script);
  writeToFile(ctx.outputDir + "/main.c", printer.extractStr());
  copyFile(ctx.assetDir + "/c/libabl.h", ctx.outputDir + "/libabl.h");
  copyFile(ctx.assetDir + "/c/libabl.c", ctx.outputDir + "/libabl.c");
  copyFile(ctx.assetDir + "/c/build.sh", ctx.outputDir + "/build.sh");
  copyFile(ctx.assetDir + "/c/run.sh", ctx.outputDir + "/run.sh");
  makeFileExecutable(ctx.outputDir + "/build.sh");
  makeFileExecutable(ctx.outputDir + "/run.sh");
}

}
