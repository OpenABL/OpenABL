#include "ParserContext.hpp"

namespace OpenABL {

bool ParserContext::parse() {
  Parser parser(*this);
  initLexer();
  return parser.parse() == 0;
}

}
