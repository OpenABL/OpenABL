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

#pragma once

#include "AST.hpp"
#include "Config.hpp"

namespace OpenABL {

struct BackendError : public std::runtime_error {
  BackendError(const std::string &msg) : std::runtime_error(msg) {}
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

struct Mason2Backend : public Backend {
  void generate(AST::Script &script, const BackendContext &ctx);
};

struct DMasonBackend : public Backend {
  void generate(AST::Script &script, const BackendContext &ctx);
};

}
