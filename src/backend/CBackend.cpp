/* Copyright 2017 OpenABL Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

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
