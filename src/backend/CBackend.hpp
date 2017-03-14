#pragma once

#include <string>
#include "AST.hpp"
#include "Printer.hpp"
#include "CPrinter.hpp"

namespace OpenABL {

struct CBackend {
  std::string astToCode(AST::Script &script) {
    script.print(printer);
    return printer.extractStr();
  }

private:
  CPrinter printer;
};

}
