#include "GenericCPrinter.hpp"

namespace OpenABL {

void GenericCPrinter::print(AST::ExpressionStatement &stmt) {
  *this << *stmt.expr << ";";
}

}
