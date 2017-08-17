#pragma once

#include "AST.hpp"

namespace OpenABL {

struct NotSupportedError : public std::logic_error {
  NotSupportedError(const std::string &msg) : std::logic_error(msg) {}
};

struct BackendContext {
  const std::string &outputDir;
  const std::string &assetDir;
  const std::map<std::string, std::string> &config;
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
