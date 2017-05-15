#pragma once

#include "AST.hpp"

namespace OpenABL {

struct Backend {
  virtual void generate(AST::Script &script, const std::string &targetDir,
                        const std::string &assetDir) = 0;
};

struct CBackend : public Backend {
  void generate(AST::Script &script, const std::string &targetDir,
                const std::string &assetDir);
};

struct FlameBackend : public Backend {
  void generate(AST::Script &script, const std::string &targetDir,
                const std::string &assetDir);
};

struct FlameGPUBackend : public Backend {
  void generate(AST::Script &script, const std::string &targetDir,
                const std::string &assetDir);
};

struct MasonBackend : public Backend {
  void generate(AST::Script &script, const std::string &targetDir,
                const std::string &assetDir);
};

}
