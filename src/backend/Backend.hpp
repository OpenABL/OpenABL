#pragma once

#include "AST.hpp"
#include "Config.hpp"

namespace OpenABL {

struct BackendError : public std::logic_error {
  BackendError(const std::string &msg) : std::logic_error(msg) {}
};

struct BackendContext {
  const std::string &outputDir;
  const std::string &assetDir;
  const Config &config;
};

struct Backend {
  virtual void generate(AST::Script &script, const BackendContext &ctx) = 0;
};

struct CBackend : public Backend {
  void generate(AST::Script &script, const BackendContext &ctx);
};

struct FlameBackend : public Backend {
  void generate(AST::Script &script, const BackendContext &ctx);
};

struct FlameGPUBackend : public Backend {
  void generate(AST::Script &script, const BackendContext &ctx);
};

struct MasonBackend : public Backend {
  void generate(AST::Script &script, const BackendContext &ctx);
};

struct DMasonBackend : public Backend {
  void generate(AST::Script &script, const BackendContext &ctx);
};

}
